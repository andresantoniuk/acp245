AC_PREREQ(2.61)
AC_INIT(e_libs, 1.1.0)
AM_INIT_AUTOMAKE([1.10 -Wall -Wno-portability -Wno-override])

VERSION_INFO="0:1:0"
AC_SUBST(VERSION_INFO)

AC_COPYRIGHT(
[ Copyright (C) 2009 Edantech. All rights reserved.

])

AM_CONFIG_HEADER([e_libs_config.h])

AC_LANG([C])

AC_STDC_HEADERS

#
# Check for package features
#

require_libevent=no

dnl check for log support
AC_ARG_ENABLE(logging,
  AC_HELP_STRING([--disable-logging],
                 [disable logging support]),
  [ac_logging=$enableval], [ac_logging=yes])
AC_MSG_CHECKING([for logging support])
if test x"$ac_logging" = "xyes"; then
    AC_MSG_RESULT([yes])
    AC_DEFINE([E_LIBS_LOG_ENABLE], 1, [logging support])
else
  AC_MSG_RESULT([no])
fi
AM_CONDITIONAL([E_LIBS_LOG_ENABLE], [test x"$ac_logging" = xyes])

dnl check for net support
AC_ARG_ENABLE(net,
  AC_HELP_STRING([--disable-net],
                 [disable net library]),
  [ac_net=$enableval], [ac_net=yes])
AC_MSG_CHECKING([for net library support])
if test x"$ac_net" = "xyes"; then
    AC_MSG_RESULT([yes])
    AC_DEFINE([E_LIBS_NET_ENABLE], 1, [net library support])
    require_libevent=yes
else
  AC_MSG_RESULT([no])
fi
AM_CONDITIONAL([E_LIBS_NET_ENABLE], [test x"$ac_net" = xyes])

dnl check for timer support
AC_ARG_ENABLE(timer,
  AC_HELP_STRING([--disable-timer],
                 [disable timer library]),
  [ac_timer=$enableval], [ac_timer=yes])
AC_MSG_CHECKING([for timer library support])
if test x"$ac_timer" = "xyes"; then
    AC_MSG_RESULT([yes])
    AC_DEFINE([E_LIBS_TIMER_ENABLE], 1, [timer library support])
    require_libevent=yes
else
  AC_MSG_RESULT([no])
fi
AM_CONDITIONAL([E_LIBS_TIMER_ENABLE], [test x"$ac_timer" = xyes])

dnl check for startup support
AC_ARG_ENABLE(startup,
  AC_HELP_STRING([--disable-startup],
                 [disable startup library]),
  [ac_startup=$enableval], [ac_startup=yes])
AC_MSG_CHECKING([for startup library support])
if test x"$ac_startup" = "xyes"; then
    AC_MSG_RESULT([yes])
    AC_DEFINE([E_LIBS_STARTUP_ENABLE], 1, [startup library support])
    require_libevent=yes
else
  AC_MSG_RESULT([no])
fi
AM_CONDITIONAL([E_LIBS_STARTUP_ENABLE], [test x"$ac_startup" = xyes])

dnl check for conf support
AC_ARG_ENABLE(conf,
  AC_HELP_STRING([--disable-conf],
                 [disable conf library]),
  [ac_conf=$enableval], [ac_conf=yes])
AC_MSG_CHECKING([for conf library support])
if test x"$ac_conf" = "xyes"; then
    AC_MSG_RESULT([yes])
    AC_DEFINE([E_LIBS_CONF_ENABLE], 1, [conf library support])
else
  AC_MSG_RESULT([no])
fi
AM_CONDITIONAL([E_LIBS_CONF_ENABLE], [test x"$ac_conf" = xyes])

dnl check for assert support
AC_ARG_ENABLE(assert,
  AC_HELP_STRING([--disable-assert],
                 [disable assertions]),
  [ac_assert=$enableval], [ac_assert=yes])
AC_MSG_CHECKING([for assertion support])
if test x"$ac_assert" = "xyes"; then
    AC_MSG_RESULT([yes])
    AC_DEFINE([E_LIBS_ASSERT_ENABLE], 1, [assertion support])
else
  AC_MSG_RESULT([no])
fi
AM_CONDITIONAL([E_LIBS_ASSERT_ENABLE], [test x"$ac_assert" = xyes])

#
# check for programs
#
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_INSTALL
AC_PROG_CC
AC_PROG_CC_C89

AC_LIBTOOL_DLOPEN
AC_LIBTOOL_WIN32_DLL
AC_PROG_LIBTOOL

# check for wine
AC_CHECK_PROGS(WINE, wine, "")
AC_CHECK_PROGS(WINEPATH, winepath, "")

#
# check for compiler characteristics
#
if eval "test x$GCC = xyes"; then
    CFLAGS="-std=c89 -D__STRICT_ANSI__"
	CFLAGS="$CFLAGS -Wmissing-declarations -Wmissing-prototypes -Werror"
fi

AC_C_VARARRAYS
AC_C_INLINE

AC_C_BIGENDIAN
type_error="no"
AC_CHECK_SIZEOF([char])
AC_CHECK_SIZEOF([short])
AC_CHECK_SIZEOF([int])
AC_CHECK_SIZEOF([long])
AC_CHECK_SIZEOF([float])
AC_CHECK_SIZEOF([double])
if test x"$ac_cv_sizeof_char" != "x1" ; then
        AC_MSG_WARN(requires sizeof(char)==1);
        type_error="yes"
fi
if test x"$ac_cv_sizeof_short" != "x2" ; then
        AC_MSG_WARN(requires sizeof(short)==2);
        type_error="yes"
fi
if test x"$ac_cv_sizeof_int" != "x4" ; then
        AC_MSG_WARN(requires sizeof(int)==4);
        type_error="yes"
fi
if test x"$ac_cv_sizeof_float" != "x4" ; then
        AC_MSG_WARN(requires sizeof(float)==4);
        type_error="yes"
fi
if test x"$ac_cv_sizeof_double" != "x8" ; then
        AC_MSG_WARN(requires sizeof(double)==8);
        type_error="yes"
fi
if test x"$type_error" = "xyes" ; then
        AC_ERROR;
fi
AC_COMPILE_IFELSE([__thread int foo;],
				  [AC_DEFINE([HAVE_TLS], 1, [thread-local storage support])],
				  [AC_MSG_WARN(no thread-local suppport)])

#
# Check for library functions
#

AC_CHECK_FUNCS(gettimeofday)
AC_CHECK_FUNCS(inet_ntoa)
AC_CHECK_FUNCS(strlcpy)
AC_CHECK_DECLS(strlcpy)
AC_CHECK_FUNCS(strlcat)
AC_CHECK_DECLS(strlcat)
AC_CHECK_FUNCS(strdup)
AC_CHECK_DECLS(strdup)
AC_CHECK_FUNCS(snprintf)
AC_CHECK_FUNCS(vsnprintf)
AC_CHECK_DECLS(vsnprintf)
AC_CHECK_FUNCS(timegm)
AC_CHECK_FUNCS(clock)
AC_CHECK_FUNCS(time)
AC_CHECK_FUNCS(getopt)

if test x"$require_libevent" = "xyes"; then
	AC_CHECK_LIB(event, event_dispatch,
		     event_LIBS="-levent",event_LIBS="")
	AC_SUBST(event_LIBS)
fi

TARGET_LD_OPTS=""
case "$host_os" in
 *mingw*)
 AC_CHECK_LIB(wsock32, main,
	     wsock32_LIBS="-lwsock32",
	     wsock32_LIBS="")
 AC_SUBST(wsock32_LIBS)
 TARGET_LD_OPTS=""
 ;;
esac
AC_SUBST(TARGET_LD_OPTS)

#
# Check for headers
#

AC_CHECK_HEADERS([fcntl.h unistd.h])

# Check for 'check'
# PKG_CHECK_MODULES([CHECK], [check >= 0.9.4])

# Define socklen_t if not available (WIN32)
AC_MSG_CHECKING([for socklen_t])
AC_TRY_COMPILE([
 #include <sys/types.h>
 #include <sys/socket.h>
],
  [socklen_t x;],
  [AC_MSG_RESULT([yes])],
  [AC_MSG_RESULT([no])
  AC_DEFINE(socklen_t, [unsigned int],
			[Define to unsigned int if you dont have it])]
)

if eval "test x$GCC = xyes"; then
    CFLAGS="$CFLAGS -Wall -Wcast-align -Wstrict-prototypes -Wpointer-arith -Wreturn-type -Wfloat-equal -Wdeclaration-after-statement -Wshadow -Wwrite-strings -pedantic"
    # extra warnings
    CFLAGS="$CFLAGS -Wmissing-field-initializers -Wsign-compare"
    #CFLAGS="$CFLAGS -O3 -Wuninitialized"
    CFLAGS="$CFLAGS -Os -Wuninitialized"
fi

AH_VERBATIM([_GNU_SOURCE],
          [
/* Most portable way that we are aware to avoid compiler warnings */
#define UNUSED(x)	((void)x)

#ifdef __GNUC__
#define UNUSED_FUNC __attribute__((unused))
#else
#define  __attribute__(x)  /* NOTHING */
#define UNUSED_FUNC /* NOTHING */
#endif

#define E_LIBS_INC_CONFIG   1
])

AC_CONFIG_MACRO_DIR([m4])

AC_CONFIG_FILES([
    Makefile
    Makefile.win
	src/Makefile
	src/Makefile.win
	tests/Makefile
	tests/Makefile.win
	doc/Makefile
	doxygen/doxyfile
    e_libs-1.0.pc
    e_libs_buff-1.0.pc
    e_libs_utils-1.0.pc
    e_libs_crc-1.0.pc
    e_libs_net-1.0.pc
    e_libs_conf-1.0.pc
    e_libs_check-1.0.pc
])
AC_OUTPUT
echo "Configure finished, type 'make' to build."
