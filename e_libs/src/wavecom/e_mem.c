/*=============================================================================
        Copyright (c) 2009 by EDANTECH (ILWICK S.A.),Montevideo, URUGUAY

        This software is furnished under a license and may be used and copied
        only in accordance with the terms of such license and with the
        inclusion of the above copyright notice. This software or any other
        copies thereof may not be provided or otherwise made available to any
        other person. No title to and ownership of the software is hereby
        transferred.
==============================================================================*/
#include "e_libs_config.h"

#include "e_mem.h"

#include "wm_stdio.h"
#include "adl_memory.h"

#include "e_port.h"

void *e_mem_malloc(size_t size) {
    return adl_memGet((u32)size);
}

void e_mem_free(void *ptr) {
    if (ptr != NULL) {
        adl_memRelease(ptr);
    }
}

void *e_mem_realloc(void *ptr, size_t size) {
    void *tmp;
    if (NULL == ptr) {
        return e_mem_malloc(size);
    }
    if (size == 0) {
        e_mem_free(ptr);
        return NULL;
    }
    tmp = e_mem_malloc(size);
    if (!tmp) {
        return tmp;
    }

    /* FIXME, may read invalid memory :S*/
    (void) e_mem_cpy(tmp, ptr, size);
    e_mem_free(ptr);

    return tmp;
}

void *e_mem_set(void *s, int c, size_t n) {
    return wm_memset(s, c, n);
}

int e_mem_cmp(const void *s1, const void *s2, size_t n) {
    return wm_memcmp(s1, s2, n);
}

void *e_mem_cpy(void *dest, const void *src, size_t n) {
    return wm_memcpy(dest, src, n);
}

void *e_mem_move(void *dest, const void *src, size_t n) {
    u8* d = dest;
    const u8* s = src;
    if (s > d) {
        while(n--) {
            *d++ = *s++;
        }
    } else if(s < d) {        
        do {
            n--;
            d[n] = s[n];
        } while(n);
    } /* else: nothing to move */
    return dest;
}
