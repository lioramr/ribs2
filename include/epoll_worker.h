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
#ifndef _EPOLL_WORKER__H_
#define _EPOLL_WORKER__H_

#include "ribs_defs.h"
#include "context.h"
#include "list.h"

extern int ribs_epoll_fd;
extern struct epoll_event last_epollev;

struct epoll_worker_fd_data {
    struct ribs_context *ctx;
    struct list timeout_chain;
    struct timeval timestamp;
};

extern struct epoll_worker_fd_data *epoll_worker_fd_map;

int epoll_worker_init(void);
void epoll_worker_loop(void);
void epoll_worker_exit(void);
void yield(void);
void courtesy_yield(void);
int ribs_epoll_add(int fd, uint32_t events, struct ribs_context* ctx);
struct ribs_context* small_ctx_for_fd(int fd, void (*func)(void));
struct ribs_context* small_ctx_for_signal(int sfd, void (*func)(void));
struct ribs_context* small_ctx_for_timer(int tfd, void (*func)(void));

#include "../src/_epoll_worker.c"

#endif // _EPOLL_WORKER__H_
