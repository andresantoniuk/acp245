/*=============================================================================
        Copyright (c) 2009 by EDANTECH (ILWICK S.A.),Montevideo, URUGUAY

        This software is furnished under a license and may be used and copied
        only in accordance with the terms of such license and with the
        inclusion of the above copyright notice. This software or any other
        copies thereof may not be provided or otherwise made available to any
        other person. No title to and ownership of the software is hereby
        transferred.
==============================================================================*/
#ifndef __e_port_
#define __e_port_

#include "e_libs.h"
#include "e_types.h"

/* Boolean values for 'bool' type, _must_ evaluate to the corresponding boolean value */
#define FALSE    (0)
#define TRUE     (1)

/* standard returned values of function */
#define OK      (0)
#define ERROR   (-1)

#define e_abort(x,y)    { abort(); }

#ifdef E_LIBS_ASSERT_ENABLE
#include <assert.h>
#define e_assert(x)     assert(x)
#else
#define e_assert(x)
#endif /* E_LIBS_ASSERT_ENABLE */

#ifndef NULL
#define NULL                    ((void *) 0)
#endif

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#define e_strlen        strlen
#define e_strcpy        strcpy
#define e_strncpy       strncpy

#ifdef WIN_MSVC
#define e_strdup        _strdup
#else
#define e_strdup        e_strdup
#endif

#define e_atoi          atoi
#define e_sprintf       sprintf
#define e_ptr_ascii_to_u8(buff, len)    ((u8*)buff)

/* Some libc's don't have strlcat/strlcpy. Local copies are provided */
#if ! HAVE_DECL_STRLCAT
#include <stddef.h>
size_t strlcat(/*@out@*/char *dst, /*@in@*/const char *src, size_t size);
#endif

#if ! HAVE_DECL_STRLCPY
#include <stddef.h>
size_t strlcpy(/*@out@*/char *dst, /*@in@*/const char *src, size_t size);
#endif
#if ! HAVE_DECL_STRDUP
char *e_strdup (/*@in@*/ const char *s);
#endif

#endif
