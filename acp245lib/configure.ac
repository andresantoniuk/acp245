AC_PREREQ(2.61)
AC_INIT(acp245, 1.6.3)
AM_INIT_AUTOMAKE([1.10 -Wall -Wno-portability -Wno-override])

AC_COPYRIGHT(
[ Copyright (C) 2009 Edantech. All rights reserved.

])

VERSION_INFO="0:0:0"
AC_SUBST(VERSION_INFO)

AM_CONFIG_HEADER([acp245_config.h])

AC_LANG([C])

AC_STDC_HEADERS

AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_INSTALL
AC_PROG_LIBTOOL
AC_PROG_CXX


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
        AC_ERROR(Your system does not provide all required data types);
fi

if eval "test x$GCC = xyes"; then
    CFLAGS="-Wall -Wcast-align -Wmissing-declarations -Wmissing-prototypes -Wstrict-prototypes -Wpointer-arith -Wreturn-type -Wfloat-equal -Wdeclaration-after-statement -Wshadow -Wwrite-strings -Werror -std=c89 -pedantic"
    # extra warnings
    #CFLAGS="$CFLAGS -Wstrict-overflow=5 -Wclobbered -Wempty-body -Wignored-qualifiers -Wmissing-field-initializers -Wmissing-parameter-type -Wold-style-declaration -Woverride-init -Wsign-compare -Wtype-limits -Wuninitialized"
    # Debug, coverage and profiling info
    # CFLAGS="$CFLAGS -g3 --coverage"
    # Optimizations, Os=optimize for size
    CFLAGS="$CFLAGS -Os"
    #CFLAGS="$CFLAGS -O3"
    dnl -D__STRICT_ANSI__"
    dnl -D_GNU_SOURCE"
	dnl -D__STRICT_ANSI__"
    dnl -Wextra
fi

PKG_CHECK_MODULES(e_libs, e_libs-1.0)
AC_SUBST(e_libs_CFLAGS)
AC_SUBST(e_libs_LIBS)

PKG_CHECK_MODULES(e_libs_buff, e_libs_buff-1.0)
AC_SUBST(e_libs_buff_CFLAGS)
AC_SUBST(e_libs_buff_LIBS)

PKG_CHECK_MODULES(e_libs_check, e_libs_check-1.0)
AC_SUBST(e_libs_check_CFLAGS)
AC_SUBST(e_libs_check_LIBS)

PKG_CHECK_MODULES(e_libs_utils, e_libs_utils-1.0)
AC_SUBST(e_libs_utils_CFLAGS)
AC_SUBST(e_libs_utils_LIBS)

TARGET_LD_OPTS=""
case "$host_os" in
 *mingw*)
 TARGET_LD_OPTS="--enable-runtime-pseudo-reloc --enable-auto-import"
 ;;
esac
AC_SUBST(TARGET_LD_OPTS)

AH_VERBATIM([_GNU_SOURCE],
          [
/* Must be set to compile the library */
#define E_ACP245_HAVE_E_LIBS 1

/* Most portable way that we are aware to avoid compiler warnings */
#define UNUSED(x)   ((void)x)

#ifdef __GNUC__
#define UNUSED_FUNC __attribute__((unused))
#else
#define  __attribute__(x)  /* NOTHING */
#define UNUSED_FUNC /* NOTHING */
#endif

])

AM_PATH_PYTHON([2.5])

AC_CONFIG_MACRO_DIR([m4])

AC_CONFIG_FILES([
    Makefile
    Makefile.win
    Makefile.wm
	src/Makefile
	src/Makefile.win
    src/acp245.h
	python/Makefile
	python/Makefile.win
	python/src/Makefile.win
	tests/Makefile
	tests/Makefile.win
	doc/Makefile
	doxygen/doxyfile
    acp245-1.0.pc
])
AC_OUTPUT
echo "Configure finished, type 'make' to build."
