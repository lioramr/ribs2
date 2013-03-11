/*
    This file is part of RIBS2.0 (Robust Infrastructure for Backend Systems).
    RIBS is an infrastructure for building great SaaS applications (but not
    limited to).

    Copyright (C) 2012,2013 Adap.tv, Inc.

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
_RIBS_INLINE_ int TEMPLATE(VMBUF_T,init)(struct VMBUF_T *vmb, size_t initial_size) {
    if (NULL == vmb->buf) {
        initial_size = RIBS_VM_ALIGN(initial_size);
        vmb->buf = (char *)mmap(NULL, initial_size, PROT_WRITE | PROT_READ, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
        if (MAP_FAILED == vmb->buf) {
            perror("mmap, vmbuf_init");
            vmb->buf = NULL;
            return -1;
        }
        vmb->capacity = initial_size;
    } else if (vmb->capacity < initial_size)
        TEMPLATE(VMBUF_T,resize_to)(vmb, initial_size);
    TEMPLATE(VMBUF_T,reset)(vmb);
    return 0;
}

_RIBS_INLINE_ int TEMPLATE(VMBUF_T,resize_to)(struct VMBUF_T *vmb, size_t new_capacity) {
    new_capacity = RIBS_VM_ALIGN(new_capacity);
    char *newaddr = (char *)ribs_mremap(vmb->buf, vmb->capacity, new_capacity, PROT_WRITE | PROT_READ, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    if ((void *)-1 == newaddr)
        return perror("mremap, " STRINGIFY(VMBUF_T) "_resize_to"), -1;
    // success
    vmb->buf = newaddr;
    vmb->capacity = new_capacity;
    return 0;
}
