#ifdef __APPLE__

#include "apple.h"
#include "logger.h"
#include <errno.h>
#include "_vmbuf_preamble.h"

#undef sendfile
#undef sysconf
#undef socket
#undef mmap
#undef munmap

struct metadata_table {
    struct mmap_entry *data;
    uint32_t capacity;
    size_t size;
};

struct mmap_entry {
    /* 52 bits for addr | 8 bits for flags | 4 bits for prot */
    uint64_t meta;
    int fd;
    off_t offset;
};

/* hashtable of mmap_entry structs */
static struct metadata_table table = {NULL, 0, 0};

/* recognized mmap flags array */
static const uint32_t FLAGS[] = {MAP_SHARED, MAP_PRIVATE, MAP_ANON, MAP_FILE};
static const int NUM_FLAGS = sizeof(FLAGS)/sizeof(uint32_t);
/* prot array */
static const uint32_t PROT[] = {PROT_NONE, PROT_READ, PROT_WRITE, PROT_EXEC};
static const int NUM_PROT = sizeof(PROT)/sizeof(uint32_t);

static uint64_t encode_flags(int decoded_flags) {
    uint64_t flags_encoded = 0;
    int i;
    for (i = 0; i < NUM_FLAGS; ++i) {
        flags_encoded |= (decoded_flags & FLAGS[i]) ? (1 << i) : 0;
    }
    return flags_encoded;
}

static int decode_flags(uint64_t encoded_flags) {
    int flags_decoded = 0;
    int i;
    for (i = 0; i < NUM_FLAGS; ++i) {
        flags_decoded |= (int)(encoded_flags & (1 << i)) ? FLAGS[i] : 0;
    }
    return flags_decoded;
}

static uint64_t encode_prot(int decoded_prot) {
    int prot_encoded = 0;
    int i;
    for (i = 0; i < NUM_PROT; ++i)
        prot_encoded |= (decoded_prot & PROT[i]) ? (1 << i) : 0;
    return prot_encoded;
}

static int decode_prot(uint64_t encoded_prot) {
    int prot_decoded = 0;
    int i;
    for (i = 0; i < NUM_PROT; ++i)
        prot_decoded |= encoded_prot & (1 << i) ? PROT[i] : 0;
    return prot_decoded;
}

static inline uint64_t data_to_addr(uint64_t data) {
    return data & -4096;
}

static inline uint32_t capacity_to_size(uint32_t capacity) {
    return (capacity+1)*sizeof(struct mmap_entry);
}

/* one less than real capacity to avoid collisions while hashing page-aligned addresses */
static inline uint32_t size_to_capacity(uint32_t size) {
    return (size / sizeof(struct mmap_entry)) - 1;
}

static void metadata_table_grow() {
    uint32_t i;
    struct mmap_entry *old_data = table.data;
    uint32_t old_capacity = table.capacity;
    uint32_t new_size = capacity_to_size(table.capacity) << 1;
    table.data = mmap(NULL, new_size, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
    table.capacity = size_to_capacity(new_size);

    for (i = 0; i < old_capacity; ++i) {
        if (old_data[i].meta)
            table.data[data_to_addr(old_data[i].meta) % table.capacity] = old_data[i];
    }
    munmap(old_data, capacity_to_size(old_capacity));
}

static inline void create_metadata_table() {
    static const size_t META_MAP_INITIAL_SIZE = 4096;
    table.data = mmap(NULL, META_MAP_INITIAL_SIZE, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
    table.capacity = size_to_capacity(META_MAP_INITIAL_SIZE);
}

static void metadata_table_insert(struct mmap_entry entry) {
    if (table.data == NULL)
        create_metadata_table();
    else if (table.size > table.capacity >> 1)
        metadata_table_grow();

    struct mmap_entry *e = table.data + (data_to_addr(entry.meta) % table.capacity);
    while (e->meta) {
        ++e;
        if (e-table.data >= table.capacity)
            e = table.data;
    }
    memcpy(e, &entry, sizeof(struct mmap_entry));
    ++table.size;
}

static int metadata_table_find_addr(uint64_t addr, struct mmap_entry **entry) {
    struct mmap_entry *e = table.data + (addr % table.capacity);
    while (data_to_addr(e->meta) != addr) {
        if (!e->meta)
            return -1;
        ++e;
        if (e-table.data >= table.capacity)
            e = table.data;
    }
    *entry = e;
    return 0;
}

static int metadata_table_lookup(uint64_t addr, struct mmap_entry *entry) {
    if (table.data == NULL)
        create_metadata_table();
    struct mmap_entry *entry_ptr = NULL;
    if (0 > metadata_table_find_addr(addr, &entry_ptr))
        return -1;
    *entry = *entry_ptr;
    return 0;
}

static int metadata_table_del(uint64_t addr) {
    struct mmap_entry *entry = NULL;
    if (0 > metadata_table_find_addr(addr, &entry))
        return -1;
    memset(entry, '\0', sizeof(struct mmap_entry));
    --table.size;
    return 0;
}

void *mmap_apple(void *addr, size_t length, int prot, int flags, int fd, off_t offset) {
    struct mmap_entry entry;
    void *new_addr = mmap(addr, length, prot, flags, fd, offset);

    entry.meta = (uint64_t)new_addr & -4096;
    entry.meta |= encode_flags(flags) << 4;
    entry.meta |= encode_prot(prot);
    entry.fd = fd;
    entry.offset = offset;
    metadata_table_insert(entry);

    return new_addr;
}

int munmap_apple(void *addr, size_t len) {
    if (0 > metadata_table_del((uint64_t)addr))
        LOGGER_PERROR("apple.h was not included before the mmap syscall");
    return munmap(addr, len);
}

/* tries appending to current mapping.  If fails, makes new mapping somewhere else */
void *mremap(void *old_address, size_t old_size, size_t new_size, int flags) {
    void *new_address;
    struct mmap_entry entry;
    int mmap_flags;
    int mmap_prot;
    size_t first_attempt_addr = vmbuf_align((size_t)old_address+old_size);
    size_t first_attempt_size = (new_size - old_size) - (first_attempt_addr - ((size_t)old_address+old_size));
    if (flags != 0 && flags != MREMAP_MAYMOVE)
        return LOGGER_PERROR("only MREMAP_MAYMOVE is supported currently by OSX mremap"), (void *)-1;
    if (0 > metadata_table_lookup((uint64_t)old_address, &entry))
        return LOGGER_PERROR("apple.h was not included before the mmap syscall"), (void *)-1;

    mmap_flags = decode_flags((entry.meta & 4095) >> 4);
    mmap_prot = decode_prot((entry.meta & 15));

    new_address = mmap((void *)first_attempt_addr, first_attempt_size, mmap_prot, mmap_flags, entry.fd, entry.offset+old_size);
    if ((size_t)new_address != first_attempt_addr) {
        munmap(new_address, first_attempt_size);
        new_address = mmap(NULL, new_size, mmap_prot, mmap_flags, entry.fd, entry.offset);
        memcpy(new_address, old_address, old_size);
        munmap(old_address, old_size);
        metadata_table_del((uint64_t)old_address);
        entry.meta &= 4095;
        entry.meta |= (uint64_t)new_address;
        metadata_table_insert(entry);
    } else
        new_address = old_address;

    return new_address;
}

/* only available flag is EPOLL_CLOEXEC */
int epoll_create1(int flags) {
    kqfd = kqueue();
    if (flags & EPOLL_CLOEXEC)
        fcntl(kqfd, F_SETFD, fcntl(kqfd, F_GETFD) | FD_CLOEXEC);
    return kqfd;
}

/* event can be null if op is epoll_ctl_del */
int epoll_ctl(int kqfd, int op, int fd, struct epoll_event *event) {
    static const int e_filters[3] = {EPOLLIN, EPOLLOUT, SIGNALFD};
    static const int k_filters[3] = {EVFILT_READ, EVFILT_WRITE, EVFILT_SIGNAL};
    struct kevent k_event;
    int i;
    memset(&k_event, 0, sizeof(k_event));
    k_event.ident = fd;
    k_event.flags = (op&(EPOLL_CTL_ADD|EPOLL_CTL_MOD)?EV_ADD:0);
    if (!(op & EPOLL_CTL_DEL)) {
        k_event.flags |= (event->events&EPOLLONESHOT?EV_ONESHOT:0) | (event->events&EPOLLET?EV_CLEAR:0);
        k_event.udata = (void *)(uint64_t)event->data.fd;//double cast to avoid compiler warning
        for (i = 0; i < 3; ++i) {
            if (event->events & e_filters[i]) {
                k_event.filter = k_filters[i];
                if (0 > kevent(kqfd, &k_event, 1, NULL, 0, 0))
                    return -1;
            }
        }
        return 0;
    } else
        return LOGGER_PERROR("EPOLL_CTL_DEL not currently handled for OSX"), -1;
}

/* epoll_wait with BSD's kqueue */
int epoll_wait(int kqfd, struct epoll_event *events, int maxevents, int timeout) {
    struct kevent k_events[maxevents];
    int i;
    int ret;
    struct timespec ts;
    int16_t filter = 0;
    /* kevent uses timeout if non-null, otherwise ignores */
    const struct timespec *ts_ptr = NULL;
    memset(k_events, 0, sizeof(k_events));
    if (timeout >= 0) {
        ts.tv_sec = timeout / 1000;
        ts.tv_nsec = (timeout % 1000) * 1000000;
        ts_ptr = &ts;
    }
    ret = kevent(kqfd, NULL, 0, k_events, maxevents, ts_ptr);
    if (0 >= ret)
        return ret;
    for (i = 0; i < ret; ++i) {
        filter = k_events[i].filter;
        events[i].events = (filter&(EVFILT_READ|EVFILT_SIGNAL|EVFILT_TIMER)?EPOLLIN:0) | (filter&EVFILT_WRITE?EPOLLOUT:0);
        events[i].data.fd = (int)(uint64_t)k_events[i].udata;
    }
    return ret;
}

/* Creates file descriptor using kqueue() - fd not used, just needed for offset into epoll_fd_map
   Doesn't matter if CLOCK_MONOTONIC or CLOCK_REALTIME is used since only relative timeouts are supported by
   timerfd_settime */
int timerfd_create(int clockid, int flags) {
    (void)clockid;
    (void)flags;
    return kqueue();
}

/* add timer to kqueue here because we can't add a timer to kqueue without also setting its expiration. */
int timerfd_settime(int fd, int flags, const struct itimerspec *new_value, struct itimerspec *old_value) {
    struct kevent k_event;
    int ret;

    if (old_value != NULL)
        return LOGGER_PERROR("timerfd_settime's old_value must be NULL for OSX"), -1;
    if (flags != 0)
        return LOGGER_PERROR("timerfd_settime only handles relative timeouts for OSX"), -1;
    if ((new_value->it_interval.tv_sec != 0 && new_value->it_interval.tv_sec != new_value->it_value.tv_sec) || (new_value->it_interval.tv_nsec != 0 && new_value->it_interval.tv_nsec != new_value->it_value.tv_nsec))
        return LOGGER_PERROR("timerfd_settime only supports intervals on OSX if interval equals initial timeout"), -1;
    memset(&k_event, 0, sizeof(k_event));
    k_event.ident = fd;
    k_event.udata = (void *)(uint64_t)fd;
    k_event.filter = EVFILT_TIMER;
    k_event.data = new_value->it_value.tv_sec * 1000000 + new_value->it_value.tv_nsec/1000;
    k_event.fflags = NOTE_USECONDS;
    /* nonzero it_value arms timer, zero value disarms */
    if (new_value->it_value.tv_sec != 0 || new_value->it_value.tv_nsec != 0)
        k_event.flags |= EV_ADD;
    else
        k_event.flags |= EV_DELETE;
    /* if interval is zero, timer only expires once */
    if (new_value->it_interval.tv_sec == 0 && new_value->it_interval.tv_nsec == 0)
        k_event.flags |= EV_ONESHOT;
    ret = kevent(kqfd, &k_event, 1, NULL, 0, 0);
    return ret - ret;
}



int fstatat(int dirfd, const char *pathname, struct stat *buf, int flags) {
    int cwd = open(".", O_RDONLY);
    int ret_val;
    int fd;
    if (0 > fchdir(dirfd))
        return -1;
    if (flags & AT_SYMLINK_NOFOLLOW) {
        fd = open(pathname, O_NOFOLLOW);
    /* if open failed, likely a symlink */
        if (0 > fd)
            ret_val = lstat(pathname, buf);
        else
            ret_val = fstat(fd, buf);
    } else {
        ret_val = stat(pathname, buf);
    }
    if (0 > fchdir(cwd))
        return -1;
    return ret_val;
}

static long phys_pages() {
    int names[2];
    size_t memsize = 0;
    size_t pagesize = 0;
    size_t mem_ret_len = sizeof(memsize);
    size_t page_ret_len = sizeof(pagesize);

    names[0] = CTL_HW;
    names[1] = HW_USERMEM; /* physical memory not used by kernel */
    sysctl(names, 2, &memsize, &mem_ret_len, NULL, 0);

    names[1] = HW_PAGESIZE;
    sysctl(names, 2, &pagesize, &page_ret_len, NULL, 0);

    return (long)(memsize / pagesize);
}

/* changed name of sysconf to catch _SC_PHYS_PAGES */
long sysconf_apple(int name) {
    if (name == _SC_PHYS_PAGES)
        return phys_pages();
    else
        return sysconf(name);
}

/* send_file_apple implements linux's sendfile using bsd's sendfile */
size_t send_file_apple(int out_fd, int in_fd, off_t *offset, size_t count) {
    size_t start_file_ofs;
    off_t filesize;
    off_t bytes_read = count;
    off_t start_offs = (offset == NULL) ? lseek(in_fd, 0, SEEK_CUR) : *offset;
    int ret =  sendfile(in_fd, out_fd, start_offs, &bytes_read, (struct sf_hdtr *)NULL, 0);
    int err = errno;
  /* if 0, we finished file */
    if (bytes_read == 0) {
        start_file_ofs = lseek(in_fd, 0, SEEK_CUR);
        filesize = lseek(in_fd, 0, SEEK_END);
        bytes_read = filesize - start_offs;
        if (offset != NULL) {
            *offset = filesize;
            lseek(in_fd, start_file_ofs, SEEK_SET);
        }
    } else
        (offset != NULL) ? *offset += bytes_read : lseek(in_fd, bytes_read, SEEK_CUR);

    errno = err;
    if (0 > ret)
        return ret;
    else
        return bytes_read;
}

#define sendfile send_file_apple
#define sysconf sysconf_apple
#define socket socket_apple
#define mmap mmap_apple
#define munmap munmap_apple

#endif //__APPLE__
