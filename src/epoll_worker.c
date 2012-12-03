/*
    This file is part of RIBS2.0 (Robust Infrastructure for Backend Systems).
    RIBS is an infrastructure for building great SaaS applications (but not
    limited to).

    Copyright (C) 2012 Adap.tv, Inc.

    RIBS is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, version 2.1 of the License.

    RIBS is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with RIBS.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "epoll_worker.h"
#include <stdio.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <stdlib.h>
#include <signal.h>
#include "logger.h"
#include <fcntl.h>
#include <setjmp.h>

#ifdef __APPLE__
#include "apple.h"
#else
#include <sys/epoll.h>
#include <sys/signalfd.h>
#endif

int ribs_epoll_fd = -1;
struct epoll_event last_epollev;

struct epoll_worker_fd_data *epoll_worker_fd_map;
static jmp_buf jmp_epoll_worker_exit;

static int queue_ctx_fd = -1;

LIST_CREATE(epoll_worker_timeout_chain);

#ifdef UGLY_GETADDRINFO_WORKAROUND
static void sigrtmin_to_context(void) {
    struct signalfd_siginfo siginfo;
    while (1) {
       int res = read(last_epollev.data.fd, &siginfo, sizeof(struct signalfd_siginfo));
       if (sizeof(struct signalfd_siginfo) != res || NULL == (void *)siginfo.ssi_ptr) {
           LOGGER_PERROR("sigrtmin_to_ctx got NULL or < 128 bytes: %d", res);
           yield();
       } else
           ribs_swapcurcontext((void *)siginfo.ssi_ptr);
    }
}
#endif

static void pipe_to_context(void) {
    void *ctx;
    while (1) {
        if (sizeof(&ctx) != read(last_epollev.data.fd, &ctx, sizeof(&ctx))) {
            LOGGER_PERROR("read in pipe_to_context");
            yield();
        } else
            ribs_swapcurcontext(ctx);
    }
}

int ribs_epoll_add(int fd, uint32_t events, struct ribs_context* ctx) {
    struct epoll_event ev = { .events = events, .data.fd = fd };
    if (0 > epoll_ctl(ribs_epoll_fd, EPOLL_CTL_ADD, fd, &ev))
        return LOGGER_PERROR("epoll_ctl"), -1;
    epoll_worker_set_fd_ctx(fd, ctx);
    return 0;
}

struct ribs_context* small_ctx_for_fd(int fd, void (*func)(void)) {
    void *ctx=ribs_context_create(SMALL_STACK_SIZE, func);
    if (NULL == ctx)
        return LOGGER_PERROR("ribs_context_create"), NULL;
    if (0 > ribs_epoll_add(fd, EPOLLIN, ctx))
        return NULL;
    return ctx;
}

/* kqueue needs to know if fd is a signal */
struct ribs_context* small_ctx_for_signal(int sfd, void (*func)(void)) {
  void *ctx=ribs_context_create(SMALL_STACK_SIZE, func);
  if (NULL == ctx)
    return LOGGER_PERROR("ribs_context_create"), NULL;
  int ev;
  #ifdef __APPLE__
  ev = SIGNALFD;
  #else
  ev = EPOLLIN;
  #endif
  if (0 > ribs_epoll_add(sfd, ev, ctx))
    return NULL;
  return ctx;
}

struct ribs_context* small_ctx_for_timer(int tfd, void (*func)(void)) {
  void *ctx = ribs_context_create(SMALL_STACK_SIZE, func);
  if (NULL == ctx)
    return LOGGER_PERROR("ribs_context_create"), NULL;
  #ifdef __APPLE__
  /* timer added to event queue when armed */
  (void)tfd;
  epoll_worker_set_fd_ctx(tfd, ctx);
  #else
  if (0 > ribs_epoll_add(tfd, EPOLLIN, ctx))
    return NULL;
  #endif
  return ctx;
}

int epoll_worker_init(void) {

    struct rlimit rlim;
    if (0 > getrlimit(RLIMIT_NOFILE, &rlim))
        return LOGGER_PERROR("getrlimit(RLIMIT_NOFILE)"), -1;
    epoll_worker_fd_map = calloc(rlim.rlim_cur, sizeof(struct epoll_worker_fd_data));

    ribs_epoll_fd = epoll_create1(EPOLL_CLOEXEC);
    if (ribs_epoll_fd < 0)
        return LOGGER_PERROR("epoll_create1"), -1;

    /* block some signals */
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGPIPE);
    if (-1 == sigprocmask(SIG_BLOCK, &set, NULL))
        return LOGGER_PERROR("sigprocmask"), -1;

#ifdef UGLY_GETADDRINFO_WORKAROUND
    sigemptyset(&set);
    sigaddset(&set, SIGRTMIN);
    if (-1 == sigprocmask(SIG_BLOCK, &set, NULL))
        return LOGGER_PERROR("sigprocmask"), -1;

    /* sigrtmin to context */
    int sfd = signalfd(-1, &set, SFD_NONBLOCK);
    if (0 > sfd)
        return LOGGER_PERROR("signalfd"), -1;
    if (NULL == small_ctx_for_signal(sfd, sigrtmin_to_context))
        return -1;
#endif

    /* pipe to context */
    int pipefd[2];
    if (0 > pipe2(pipefd, O_NONBLOCK))
        return LOGGER_PERROR("pipe"), -1;
    if (NULL == small_ctx_for_signal(pipefd[0], pipe_to_context))
        return -1;
    queue_ctx_fd = pipefd[1];

    return 0;
}

void epoll_worker_loop(void) {
    if (0 == setjmp(jmp_epoll_worker_exit)) {
        for (;;yield());
    }
}

void epoll_worker_exit(void) {
    longjmp(jmp_epoll_worker_exit, 1);
}

void yield() {
  while(0 >= epoll_wait(ribs_epoll_fd, &last_epollev, 1, -1));
  ribs_swapcurcontext(epoll_worker_get_last_context());
}

void courtesy_yield() {
    if (0 == epoll_wait(ribs_epoll_fd, &last_epollev, 1, 0))
        return;
    if (0 > write(queue_ctx_fd, &current_ctx, sizeof(void *)))
        LOGGER_PERROR("unable to queue context: write");
    ribs_swapcurcontext(epoll_worker_get_last_context());
}
