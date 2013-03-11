#ifndef _RIBS_MMAN__H_
#define _RIBS_MMAN__H_
//ribs memory management

#ifdef __APPLE__
#include "apple.h"
#endif

_RIBS_INLINE_ void *ribs_mremap(void *addr, size_t old_size, size_t new_size, int prot, int flags, int fd, off_t offset) {
#ifdef __APPLE__
    return mremap_apple(addr, old_size, new_size, prot, flags, fd, offset);
#else
    return mremap(addr, old_size, new_size, MREMAP_MAYMOVE);
#endif
}

#endif _RIBS_MMAN__H_
