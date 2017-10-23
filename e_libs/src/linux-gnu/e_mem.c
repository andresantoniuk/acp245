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

#include <stdlib.h>
#include <string.h>

#include "e_port.h"

void *e_mem_malloc(size_t size) {
    return malloc(size);
}

void e_mem_free(void *ptr) {
    free(ptr);
}

void *e_mem_realloc(void *ptr, size_t size) {
    return realloc(ptr, size);
}

void *e_mem_set(void *s, int c, size_t n) {
    return memset(s, c, n);
}

extern int e_mem_cmp(const void *s1, const void *s2, size_t n) {
    return memcmp(s1, s2, n);
}

void *e_mem_cpy(void *dest, const void *src, size_t n) {
    return memcpy(dest, src, n);
}

void *e_mem_move(void *dest, const void *src, size_t n) {
    return memmove(dest, src, n);
}
