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

#define CANARY 0xDEADBEEF

struct mmap_meta {
  uint32_t canary;
  size_t length;
  int prot;
  int flags;
};

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

static void set_mmap_metadata(struct mmap_meta *data, size_t length, int prot, int flags) {
    data->canary = CANARY;
    data->length = length;
    data->prot = prot;
    data->flags = flags;
}

void *mmap_apple(void *addr, size_t length, int prot, int flags, int fd, off_t offset) {
    size_t map_size = vmbuf_align(length+sizeof(struct mmap_meta));
    void *new_addr = mmap(addr, map_size, prot, flags, fd, offset);
    struct mmap_meta *data = (struct mmap_meta *)(new_addr+length);
    set_mmap_metadata(data, map_size, prot, flags & ~MAP_FIXED);
    return new_addr;
}

int munmap_apple(void *addr, size_t len) {
    struct mmap_meta *metadata = (struct mmap_meta *)((char *)addr + len);
    if (metadata->canary != CANARY)
        return LOGGER_PERROR("either apple.h was not included before the mmap call or the mmap'd buffer was overflown"), -1;
    return munmap(addr, metadata->length);
}

void *mremap(void *old_address, size_t old_size, size_t new_size, int flags) {
    void *new_address;
    size_t space_needed;
    size_t map_size = 0;
    struct mmap_meta metadata = *(struct mmap_meta *)(old_address+old_size);
    if (flags != 0 && flags != MREMAP_MAYMOVE)
        return LOGGER_PERROR("only MREMAP_MAYMOVE is supported currently by OSX mremap"), (void *)-1;
    if (metadata.canary != CANARY)
        return LOGGER_PERROR("either apple.h was not included before the mmap call or the mmap'd buffer was overflown"), (void *)-1;

    space_needed = (new_size+sizeof(struct mmap_meta)) - metadata.length;
    if (0 >= space_needed) {
        memset(old_address+old_size, 0, sizeof(struct mmap_meta));
        memcpy(old_address+new_size, &metadata, sizeof(struct mmap_meta));
        return old_address;
    }
    map_size = vmbuf_align(metadata.length+space_needed);
    new_address = mmap((char *)old_address+metadata.length, map_size, metadata.prot, metadata.flags, -1, 0);
    /* sucessfully appended memory to current mapping */
    if (new_address == (char *)old_address+metadata.length) {
        memset(old_address+old_size, 0, sizeof(struct mmap_meta));
        set_mmap_metadata((struct mmap_meta *)(old_address+new_size), map_size+metadata.length, metadata.prot, metadata.flags);
        return old_address;
    /* need to transfer old memory to new mapping */
    } else {
        memcpy(new_address, old_address, old_size);
        munmap(old_address, metadata.length);
        set_mmap_metadata((struct mmap_meta *)(new_address+new_size), map_size, metadata.prot, metadata.flags);
        return new_address;
    }
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
