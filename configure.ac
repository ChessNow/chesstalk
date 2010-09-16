# Process this file with autoconf to produce a configure script.

AC_INIT(chesstalk, [0.94], [cn-github@fractal-interval.com])

AC_PREREQ(2.59)

VERSION_MAJOR="0"
VERSION_MINOR="9"
VERSION_REVISION="3"
VERSION_EXTRA="darcs"

CONFIGURE_LINE="`echo "$0 $ac_configure_args" | sed -e 's/\\\/\\\\\\\/g'`"
CODENAME="Manifree"

AC_CANONICAL_BUILD
AC_CANONICAL_HOST

AM_INIT_AUTOMAKE

AC_CONFIG_HEADERS(config.h)

AC_CONFIG_MACRO_DIR([m4])
AC_PROG_LIBTOOL

AC_PROG_CC
AC_PROG_MAKE_SET

AM_PROG_CC_C_O

AC_CHECK_HEADER(stdio.h,[AC_DEFINE(HAVE_STDIO_H)],[AC_MSG_ERROR([Sorry, can't do anything for you])])

AC_CONFIG_SRCDIR([src/chesstalk.c])

#AC_CHECK_LIB([dl], [dlopen])
#AC_CHECK_LIB([pthread], [pthread_join])
#AC_SEARCH_LIBS([dlopen], [dl])

AC_PATH_X
AC_HEADER_STDC

AC_HEADER_STDBOOL
AC_C_CONST
AC_HEADER_TIME

AC_FUNC_MALLOC
AC_FUNC_MEMCMP
AC_FUNC_STAT
AC_FUNC_VPRINTF

AC_CHECK_HEADERS(stdio.h)
AC_CHECK_HEADERS(malloc.h)
AC_CHECK_HEADERS(stdlib.h)
AC_CHECK_HEADERS(string.h)

AC_CHECK_FUNCS(strchr.h)
AC_CHECK_FUNCS(strcpy)
AC_CHECK_FUNCS(strtol)

#AC_CHECK_HEADERS(pthread.h)

AC_CHECK_FUNCS(getenv)

AC_CONFIG_FILES([src/Makefile Makefile module/Makefile module/ajaxtime/Makefile])

AC_OUTPUT

