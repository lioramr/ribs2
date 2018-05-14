/*
    This file is part of RIBS2.0 (Robust Infrastructure for Backend Systems).
    RIBS is an infrastructure for building great SaaS applications (but not
    limited to).

    Copyright (C) 2014 Adap.tv, Inc.

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
#ifndef _RIBS_SSL__H_
#define _RIBS_SSL__H_

#include "ribs_defs.h"

#ifdef RIBS2_SSL

#include <openssl/ssl.h>
#include <openssl/err.h>

void ribs_ssl_init(void);
SSL *ribs_ssl_get(int fd);
SSL *ribs_ssl_alloc(int fd, SSL_CTX *ssl_ctx);
void ribs_ssl_free(int fd);
int ribs_ssl_want_io(SSL *ssl, int res);
int ribs_ssl_set_options(SSL_CTX *ssl_ctx, char *cipher_list, char *dhparam_file);

#endif

#endif // _RIBS_SSL__H_
