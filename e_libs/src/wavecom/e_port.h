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

#include "wm_types.h"
#include "wm_stdio.h"

typedef s32     e_ret;

#define e_abort(x,y)    adl_errHalt(x, y)

#ifdef E_LIBS_ASSERT_ENABLE
#include "adl_global.h"
#include "e_log.h"
#define e_assert(x)     if(!(x)){E_FATAL("ASSERTION FAILED:" #x);if(!E_LOG_IS(DEBUG)) {adl_errHalt(0, "assertion failed");}}
#else
#define e_assert(x)
#endif /* E_LIBS_ASSERT_ENABLE */

#include <string.h>
#define e_strlen    wm_strlen
#define e_strcpy    wm_strcpy
#define e_strncpy   wm_strncpy
#define e_strdup    strdup
#define e_atoi      wm_atoi
#define e_sprintf   wm_sprintf
#define e_ptr_ascii_to_u8(buff, len)    ((u8*)buff)


/* Some libc's don't have strlcat/strlcpy. Local copies are provided */
#ifndef HAVE_STRLCAT
#include <stddef.h>
size_t strlcat(/*@out@*/char *dst, /*@in@*/const char *src, size_t size);
#endif

#ifndef HAVE_STRLCPY
#include <stddef.h>
size_t strlcpy(/*@out@*/char *dst, /*@in@*/const char *src, size_t size);
#endif

#ifndef HAVE_STRDUP
#include <stddef.h>
char *strdup (/*@in@*/ const char *s);
#endif

#endif
