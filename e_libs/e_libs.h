/* e_libs.h -- fundamental types and structures for the e_libs library */

#ifndef __e_libs_h__
#define __e_libs_h__

#ifndef E_LIBS_INC_CONFIG

/* Feature configuration switches begin here */
#define E_LIBS_VERSION "1.1.0"
#define E_LIBS_ASSERT_ENABLE 1
#define E_LIBS_CONF_ENABLE 1
#define E_LIBS_LOG_ENABLE 1
#define E_LIBS_NET_ENABLE 1
#define E_LIBS_STARTUP_ENABLE 1
#define E_LIBS_TIMER_ENABLE 1

#endif /* E_LIBS_CONFIG_INC */

/* Most portable way that we are aware to avoid compiler warnings */
#define E_UNUSED(x)	((void)x)

#ifdef __GNUC__
#define E_UNUSED_FUNC __attribute__((unused))
#else
#define  __attribute__(x)  /* NOTHING */
#define E_UNUSED_FUNC /* NOTHING */
#endif

#define E_INLINE /* NOTHING */

#ifdef _MSC_VER
#define __func__ __FUNCTION__
#endif

#endif /* __e_libs_h__ */
