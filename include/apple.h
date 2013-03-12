#ifndef _APPLE__H_
#define _APPLE__H_
#include <fcntl.h>
#include <sys/event.h>
#include <string.h>
#include <sys/mman.h>
#include <stdio.h>
#include <sys/sysctl.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/socket.h>
#include "ribs_defs.h"

#define MAP_ANONYMOUS MAP_ANON

/* mremap flags */
#define MREMAP_MAYMOVE 1
#define MREMAP_FIXED (1<<1)

#define _SC_PHYS_PAGES (1<<30)

#define TCP_CORK TCP_NOPUSH

/* fstatat flags */
#define AT_SYMLINK_NOFOLLOW 1

/* socket flags */
#define SOCK_NONBLOCK (1<<30)
#define SOCK_CLOEXEC (1<<29)

enum epoll_ctl {
  /* events */
  EPOLLIN = 1,
  EPOLLOUT = 1 << 1,
  SIGNALFD = 1 << 2,
  EPOLLONESHOT = 1 << 3,
  EPOLLET = 1 << 4,
  /* ops */
  EPOLL_CTL_ADD = 1 << 5,
  EPOLL_CTL_MOD = 1 << 6,
  EPOLL_CTL_DEL = 1 << 7
};

/* epoll_create1 flags */
#define EPOLL_CLOEXEC (1<<30)

/* timerfd flags */
#define CLOCK_MONOTONIC 1
#define TFD_CLOEXEC (1<<1)
#define TFD_NONBLOCK (1<<2)

typedef union epoll_data {
    void *ptr;
    int fd;
    uint32_t u32;
    uint64_t u64;
} epoll_data_t;

struct epoll_event {
    int32_t events;
    epoll_data_t data;
};

struct itimerspec {
    struct timespec it_interval; /* Timer interval */
    struct timespec it_value;    /* initial expiration */
};

int kqfd;

int epoll_create1(int flags);
int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);
int epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout);

int timerfd_create(int clockid, int flags);
int timerfd_settime(int fd, int flags, const struct itimerspec *new_value, struct itimerspec *old_value);

void *mremap_apple(void *old_address, size_t old_size, size_t new_size, int prot, int flags, int fd, off_t offset);
int fstatat(int dirfd, const char *pathname, struct stat *buf, int flags);
long sysconf_apple(int name);
size_t send_file_apple(int out_fd, int in_fd, off_t *offset, size_t count);
int pipe2(int fildes[2], int flags);
char *strchrnul(const char *s, int c);
int accept4(int socket, struct sockaddr *addr_buf, socklen_t *addr_len, int flags);
int socket_apple(int socket_family, int socket_type, int protocol);

/* linux and BSD have different sendfile() routines with same name */
#define sendfile send_file_apple
/* redirect sysconf to sysconf_apple to handle _SC_PHYS_PAGES */
#define sysconf sysconf_apple
/* redirect socket to socket_apple to handle SOCK_NONBLOCK and SOCK_CLOEXEC flags */
#define socket socket_apple

#endif //_APPLE__H_
