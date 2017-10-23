/*=============================================================================
        Copyright (c) 2009 by EDANTECH (ILWICK S.A.),Montevideo, URUGUAY

        This software is furnished under a license and may be used and copied
        only in accordance with the terms of such license and with the
        inclusion of the above copyright notice. This software or any other
        copies thereof may not be provided or otherwise made available to any
        other person. No title to and ownership of the software is hereby
        transferred.
==============================================================================*/
#ifndef __e_mem_h_
#define __e_mem_h_

#include "e_port.h"

#ifdef __cplusplus
extern "C" {
#endif

/*@only@*/ /*@null@*/
extern void *e_mem_malloc(size_t size);

/*@only@*/ /*@null@*/
extern void *e_mem_realloc(/*@only@*/ /*@null@*/ /*@out@*/ void *ptr, 
        size_t size);

extern void e_mem_free(/*@only@*/ /*@null@*/ /*@out@*/ void *ptr);

extern void *e_mem_set(void *s, int c, size_t n);

extern void *e_mem_cpy(void *dest, const void *src, size_t n);

extern int e_mem_cmp(const void *s1, const void *s2, size_t n);

extern void *e_mem_move(void *dest, const void *src, size_t n);

#ifdef __cplusplus
}
#endif

#endif
