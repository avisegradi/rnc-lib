#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT([Random Network Coding Library], [1.0], [visegradi.adam@sztaki.mta.hu])
AM_INIT_AUTOMAKE([rnc], [1.0])
LT_INIT()
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([config.h.in])
AM_CONFIG_HEADER(config.h)

# Checks for programs.
AC_PROG_CXX
AC_PROG_AWK
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET

# Checks for libraries.

AM_PATH_GLIB_2_0([2.10.0])
if test "$no_glib" = yes; then
        AC_MSG_ERROR([glib libraries were not found])
fi

GTHREAD_LIBS=`pkg-config --libs gthread-2.0`
GLIB_CFLAGS=`pkg-config --cflags glib-2.0`
AC_SUBST([GTHREAD_LIBS])
AC_SUBST([GLIB_CFLAGS])

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h stdint.h stdlib.h string.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_OFF_T
AC_TYPE_UINT8_T
AC_TYPE_UINT16_T

# Checks for library functions.
AC_FUNC_MMAP
AC_CHECK_FUNCS([memset munmap strerror])

# Select finite field
AC_ARG_WITH([q256], AS_HELP_STRING([--with-q256], [Use 8 bit finite field. Default: 16 bit finite field]), [], [with_q256=no])
if test "$with_q256" != no; then
        AC_MSG_NOTICE([Using 8 bit finite field GF(256)])
	AC_DEFINE([Q256], [1], [Use 8 bit finite field.])
else
        AC_MSG_NOTICE([Using 16 bit finite field GF(65536)])
	AC_DEFINE([Q256], [0], [Use 8 bit finite field.])
fi

# Compile tests
AC_ARG_WITH([tests], AS_HELP_STRING([--without-tests], [Don't compile tests. By default, tests are compiled.]), [], [with_tests=yes])
AM_CONDITIONAL([WITH_TESTS], [test "x$with_tests" != xno])
if test "x$with_tests" != xno; then
        AC_MSG_NOTICE([Will compile tests])
else
        AC_MSG_NOTICE([Will NOT compile tests])
fi

######################################################################
# DOXYGEN SUPPORT
######################################################################

DX_HTML_FEATURE(ON)
DX_CHM_FEATURE(OFF)
DX_CHI_FEATURE(OFF)
DX_MAN_FEATURE(ON)
DX_RTF_FEATURE(OFF)
DX_XML_FEATURE(OFF)
DX_PDF_FEATURE(OFF)
DX_PS_FEATURE(OFF)

DX_INIT_DOXYGEN([$PACKAGE_NAME],[doxygen.cfg])

######################################################################

AC_CONFIG_FILES([
                Makefile
                src/Makefile
                test/Makefile
                test/original/Makefile
                test/common/Makefile
                test/test-fq/Makefile
                test/test-matr/Makefile
                rnc-1.0.pc])
AC_OUTPUT
