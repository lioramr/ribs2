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
#ifndef _CONTEXT__H_
#define _CONTEXT__H_

#include "ribs_defs.h"

#ifdef __APPLE__
#include "apple.h"
#else
#include <sys/epoll.h>
#endif

#define SMALL_STACK_SIZE 4096

#ifdef __x86_64__
#define NUM_ADDITIONAL_REGS 5
#endif

#ifdef __i386__
#define NUM_ADDITIONAL_REGS 3
#endif

struct ribs_context {
#ifndef __arm__
    uintptr_t stack_pointer_reg;
    uintptr_t parent_context_reg;
    uintptr_t additional_reg[NUM_ADDITIONAL_REGS];
#else
    uintptr_t parent_context_reg;
    uintptr_t first_func_reg;
    uintptr_t additional_reg[7];
    uintptr_t stack_pointer_reg;
    uintptr_t linked_func_reg;
#endif
    epoll_data_t data;
    struct ribs_context *next_free;
    int fd;
    char reserved[];
};

extern struct ribs_context main_ctx;
extern struct ribs_context *current_ctx;

extern void ribs_swapcurcontext(struct ribs_context *rctx);
extern void ribs_makecontext(struct ribs_context *ctx, struct ribs_context *pctx, void (*func)(void));

extern struct ribs_context *ribs_context_create(size_t stack_size, void (*func)(void));

#define RIBS_RESERVED_TO_CONTEXT(ptr) ((struct ribs_context *)((char *)ptr - offsetof(struct ribs_context, reserved)))

#endif // _CONTEXT__H_
