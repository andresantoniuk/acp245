/*=============================================================================
        Copyright (c) 2009 by EDANTECH (ILWICK S.A.),Montevideo, URUGUAY

        This software is furnished under a license and may be used and copied
        only in accordance with the terms of such license and with the
        inclusion of the above copyright notice. This software or any other
        copies thereof may not be provided or otherwise made available to any
        other person. No title to and ownership of the software is hereby
        transferred.
==============================================================================*/
#ifndef __e_log_h_
#define __e_log_h_

#include "e_port.h"

#define FACILITY_NONE    0
#define FACILITY_CONSOLE 1
#define FACILITY_CUSTOM  2
#define FACILITY_BUFFER	 4

#define E_BUFSIZ		512

#define E_LOG_NONE  	0
#define E_LOG_FATAL		1
#define E_LOG_ERR		2
#define E_LOG_WARN		3
#define E_LOG_INFO		4
#define E_LOG_PROG		5
#define E_LOG_DEBUG		6
#define E_LOG_TRACE		7
#define E_LOG_STACK		8
#define E_LOG_MEM		10
#define E_LOG_ALL		255

#define E_LOG_NOMEM		E_FATAL("Out of memory.\n")

#ifndef E_LIBS_LOG_ENABLE

#define E_LOG_IS(x)     0
#define E_LOG_SET_LEVEL(x)
#define E_LOG_SET_FACILITY(x)
#define E_LOG_ADD_FACILITY(x)
#define E_LOG_SET_CUSTOM_HANDLER(x)

/* define variadic and non-variadic macros. C99 requires all macros args to 
 * be used. */
#if (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L) || defined(__WM__)
#define E_LOG(x, ...)
#define E_FATAL(...)
#define E_ERR(...)
#define E_WARN(...)
#define E_INFO(...)
#define E_DBG(...)
#define E_TRACE(...)
#define E_STACK(...)
#else
/* use empty functions if not using log and variadic macros are not supported */
#ifdef __cplusplus
extern "C" {
#endif
#define E_LOG if (0) E_LOG_FAKE
#define E_FATAL if (0)E_LOG_FAKE2
#define E_ERR if (0) E_LOG_FAKE2
#define E_WARN if (0) E_LOG_FAKE2
#define E_INFO if (0) E_LOG_FAKE2
#define E_DBG if (0) E_LOG_FAKE2
#define E_TRACE if (0) E_LOG_FAKE2
#define E_STACK if (0) E_LOG_FAKE2
E_UNUSED_FUNC E_INLINE static void E_LOG_FAKE(int errlevel, const char *fmt, ...){}
E_UNUSED_FUNC E_INLINE static void E_LOG_FAKE2(const char *fmt, ...){}
#if 0
E_UNUSED_FUNC E_INLINE static void E_LOG(int errlevel, const char *fmt, ...){}
E_UNUSED_FUNC E_INLINE static void E_FATAL(const char *fmt, ...){}
E_UNUSED_FUNC E_INLINE static void E_ERR(const char *fmt, ...){}
E_UNUSED_FUNC E_INLINE static void E_WARN(const char *fmt, ...){}
E_UNUSED_FUNC E_INLINE static void E_INFO(const char *fmt, ...){}
E_UNUSED_FUNC E_INLINE static void E_DBG(const char *fmt, ...){}
E_UNUSED_FUNC E_INLINE static void E_TRACE(const char *fmt, ...){}
E_UNUSED_FUNC E_INLINE static void E_STACK(const char *fmt, ...){}
#endif
#ifdef __cplusplus
}
#endif
#endif /* (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L) || defined(__WM__) */

#else

#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif
extern int e_log_facility;

extern int e_log_errlevel;

#define E_LOG_IS(x)             e_log_errlevel >= E_LOG_##x
#define E_LOG_SET_LEVEL(x)      e_log_set_level(E_LOG_##x)
#define E_LOG_SET_FACILITY(x)   e_log_set_facility(FACILITY_##x)
#define E_LOG_ADD_FACILITY(x)   e_log_add_facility(FACILITY_##x)
#define E_LOG_SET_CUSTOM_HANDLER(x)   e_log_set_custom_handler(x)

/* define variadic and non-variadic macros. C99 requires all macros args
 * to be used. */
#if (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L ) || (defined(__WM__))
#define __e_log_h_MACROS
#define E_LOG(x, ...)	if(e_log_errlevel >= x) { \
    e_log_file(__FILE__, __LINE__, x, __VA_ARGS__ ); }
#define E_FATAL(...)	E_LOG(E_LOG_FATAL, __VA_ARGS__ )
#define E_ERR(...)		E_LOG(E_LOG_ERR, __VA_ARGS__ )
#define E_WARN(...)	    E_LOG(E_LOG_WARN, __VA_ARGS__ )
#define E_INFO(...)		E_LOG(E_LOG_INFO, __VA_ARGS__ )
#define E_DBG(...)		E_LOG(E_LOG_DEBUG, __VA_ARGS__ )
#define E_TRACE(...)	E_LOG(E_LOG_TRACE, __VA_ARGS__ )
#define E_STACK(...)	E_LOG(E_LOG_STACK, __VA_ARGS__ )

#else
extern void E_LOG(int errlevel, const char *fmt, ...)
    __attribute__((format(printf, 2, 3)));
extern void E_FATAL(const char *fmt, ...)
    __attribute__((format(printf, 1, 2)));
extern void E_ERR(const char *fmt, ...)
    __attribute__((format(printf, 1, 2)));
extern void E_WARN(const char *fmt, ...)
    __attribute__((format(printf, 1, 2)));
extern void E_INFO(const char *fmt, ...)
    __attribute__((format(printf, 1, 2)));
extern void E_DBG(const char *fmt, ...)
    __attribute__((format(printf, 1, 2)));
extern void E_TRACE(const char *fmt, ...)
    __attribute__((format(printf, 1, 2)));
extern void E_STACK(const char *fmt, ...)
    __attribute__((format(printf, 1, 2)));
#endif /* (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L ) || (defined(__WM__)) */

extern void e_log_file(const char* file, const long line, int errlevel,
        const char *fmt, ...);

extern char* e_log_get_err(void);

extern int e_log_get_facility(void);

extern int e_log_set_facility(int f);

extern int e_log_add_facility(int f);

extern int e_log_get_level(void);

extern void e_log_set_level(int level);

extern void e_log_set_console_level(int level);

typedef void (*e_log_custom_handler) (int level, ascii *str);
extern void e_log_set_custom_handler(e_log_custom_handler handler);

#ifdef __cplusplus
}
#endif

#endif /* E_LIBS_LOG_ENABLE */

#endif /* __e_log_h */
