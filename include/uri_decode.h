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
#ifndef _URI_DECODE__H_
#define _URI_DECODE__H_

#include "ribs_defs.h"
#include "hashtable.h"

#ifdef __APPLE__
#include "apple.h"
#endif

_RIBS_INLINE_ size_t http_uri_decode(char *uri, char *target);
_RIBS_INLINE_ void http_uri_decode_query_params(char *query_params, struct hashtable *params);
#include "../src/_uri_decode.c"

#endif // _URI_DECODE__H_
