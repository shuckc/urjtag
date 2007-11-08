#
# $Id$
#
# Copyright (C) 2002, 2003 ETC s.r.o.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
# 02111-1307, USA.
#
# Written by Marcel Telka <marcel@telka.sk>, 2002, 2003.
#

AC_INIT(JTAG Tools,0.6,,jtag)

AC_PREREQ(2.54)
AC_COPYRIGHT([Copyright (C) 2002, 2003 ETC s.r.o.])
AC_REVISION($Revision$)

AC_CONFIG_AUX_DIR(tools)

AM_INIT_AUTOMAKE([check-news dist-bzip2])

AC_CONFIG_FILES(
	Makefile
	doc/Makefile
	data/Makefile
	include/Makefile
	src/Makefile
	src/lib/Makefile
	src/tap/Makefile
	src/part/Makefile
	src/bus/Makefile
	src/cmd/Makefile
	src/svf/Makefile
	po/Makefile.in
)

AM_MAINTAINER_MODE

AM_CONFIG_HEADER(config.h)

AC_GNU_SOURCE

AM_GNU_GETTEXT(external)
AM_GNU_GETTEXT_VERSION(0.14.1)

AC_PROG_CC
AC_PROG_RANLIB
AC_PROG_LEX
AC_PROG_YACC

AC_CONFIG_SUBDIRS(libbrux)

AC_ARG_WITH(include,
	AC_HELP_STRING([--with-include=PATH], [Absolute path to the openwince includes]),
	openwince_inc="$withval")

AC_CACHE_CHECK([for the openwince includes], openwince_includes_path, [
	openwince_includes_path="no"
	for openwince_include in $openwince_inc /usr/include/openwince /usr/local/include/openwince; do
		if test -f "$openwince_include/common.h"; then
			openwince_includes_path="$openwince_include"
			break
		fi
	done
])
if test "$openwince_includes_path" = "no"; then
	AC_MSG_ERROR([The openwince include package can not be found.])
fi
CPPFLAGS="$CPPFLAGS -I$openwince_includes_path -I$openwince_includes_path/device -I$openwince_includes_path/arm"

AC_SEARCH_LIBS([ioperm], [ioperm])
if test "$ac_cv_search_ioperm" != "no"; then
	AC_DEFINE(HAVE_IOPERM, 1, [Define to 1 if you have the ioperm() function])
else
	AC_SEARCH_LIBS([i386_set_ioperm], [i386])
	if test "$ac_cv_search_i386_set_ioperm" != "no"; then
		AC_DEFINE(HAVE_I386_SET_IOPERM, 1,
			[Define to 1 if you have the i386_set_ioperm() function])
	fi
fi

AC_CHECK_FUNCS(getline getdelim)
AC_CHECK_FUNCS(swprintf)

AC_CHECK_HEADERS(linux/ppdev.h)

VL_LIB_READLINE

dnl check for libftdi-config
AC_PATH_PROG(HAVELIBFTDI, libftdi-config, $PATH)
if test ! -z "$HAVELIBFTDI"; then
dnl   LIBFTDI_CFLAGS=`$HAVELIBFTDI --cflags`
   LIBFTDI_LIBS=`$HAVELIBFTDI --libs`
   CFLAGS="$CFLAGS $LIBFTDI_CFLAGS"
   LIBS="$LIBS $LIBFTDI_LIBS"
   AC_DEFINE(HAVE_LIBFTDI, 1, [Define if you have libftdi])
else
   AC_MSG_ERROR([*** libftdi-config not found. You need a working libftdi installation.])
fi

dnl check for libusb-config
AC_PATH_PROG(HAVELIBUSB, libusb-config, $PATH)
if test ! -z "$HAVELIBUSB"; then
dnl   LIBUSB_CFLAGS=`$HAVELIBUSB --cflags`
   LIBUSB_LIBS=`$HAVELIBUSB --libs`
   CFLAGS="$CFLAGS $LIBUSB_CFLAGS"
   LIBS="$LIBS $LIBUSB_LIBS"
   AC_DEFINE(HAVE_LIBUSB, 1, [Define if you have libusb])
else
   AC_MSG_ERROR([*** libusb-config not found. You need a working libusb installation.])
fi

CFLAGS="$CFLAGS -Wall"
CPPFLAGS="$CPPFLAGS -I\$(top_srcdir) -I\$(top_srcdir)/include"

AC_OUTPUT
