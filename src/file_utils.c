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

#include "file_utils.h"
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

int mkdir_recursive(const char *filename) {
    char file[strlen(filename) + 1];
    strcpy(file, filename);
    char *p = strrchr(file, '/');
    if (NULL == p)
        return 0;
    *p = 0;
    char *cur = file;
    while (*cur) {
        ++cur;
        char *p = strchrnul(cur, '/');
        char c = *p;
        *p = 0;
        if (0 > mkdir(file, 0755) && errno != EEXIST)
            return -1;
        cur = p;
        *p = c;
    }
    *p = '/'; // restore
    return 0;
}
