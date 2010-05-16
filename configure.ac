dnl $Id$

AC_INIT(src/ydpdict.c)
AM_INIT_AUTOMAKE(libydpdict, 1.0.3)
AC_PREREQ(2.50)
AC_CONFIG_HEADERS(config.h)

AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_LIBTOOL
AC_C_CONST
AC_C_BIGENDIAN

VERSION_COLON=`echo "$VERSION" | sed 's/"//g' | sed 's/\./,/g'`
AC_SUBST(VERSION_COLON)

AC_CONFIG_FILES([Makefile src/Makefile pkgconfig/Makefile pkgconfig/libydpdict.pc include/Makefile include/ydpdict.h misc/Makefile])
AC_OUTPUT

