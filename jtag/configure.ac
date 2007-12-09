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

AC_INIT(UrJTAG,0.6,,urjtag)

AC_PREREQ(2.54)
AC_COPYRIGHT([Copyright (C) 2002, 2003 ETC s.r.o.])
AC_REVISION($Revision$)

# Get SVN Revision - idea from xfce-power-manager (C) 2007 by B.Tarricone
dnl get svn revision
SVN=`which svn`
if test -n "$SVN" -a -x "$SVN"; then
    SVN_REVISION=`LC_ALL=C svn info | awk '/^Revision: / {printf "%d\n", $2}'`
else if test -e "ChangeLog"; then
    SVN_REVISION=`cat ChangeLog | awk '/^Revision: / {printf "%d\n", $2}'`
else
    SVN_REVISION=`echo '$Revision$' | awk '/^Revision: / {printf "%d\n", $2}'`
fi fi

AC_DEFINE_UNQUOTED([SVN_REVISION], "$SVN_REVISION", [subversion revision number])
AC_SUBST([SVN_REVISION])

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
AC_PROG_YACC

# check for lex/flex
AC_PROG_LEX
# check for modern version of flex
AC_PROG_AWK
AC_PROG_SED
if test "$LEX" == flex ; then
   # compare >= 2.5.31
   flex_ref_v1=2
   flex_ref_v2=5
   flex_ref_v3=31
   flex_ref_version="$flex_ref_v1.$flex_ref_v2.$flex_ref_v3"
   AC_MSG_CHECKING([for flex >= $flex_ref_version])

   modern_flex=no
   flex_version=`$LEX --version | $AWK '{print $2}'`
   if test "$flex_version" == "version" ; then
       flex_version=`$LEX --version | $AWK '{print $3}'`
   fi

   flex_v1=`echo $flex_version | $AWK -F . '{print $1}' | $SED -e 's/[a-zA-Z]//g'`
   flex_v2=`echo $flex_version | $AWK -F . '{print $2}' | $SED -e 's/[a-zA-Z]//g'`
   flex_v3=`echo $flex_version | $AWK -F . '{print $3}' | $SED -e 's/[a-zA-Z]//g'`

   if test -n "$flex_v1" -a "$flex_v1" -gt "$flex_ref_v1" ; then
      modern_flex=yes
   fi
   if test -n "$flex_v1" -a "$flex_v1" -eq "$flex_ref_v1" ; then
      if test "$flex_v2" -gt "$flex_ref_v2" ; then
	 modern_flex=yes
      fi
      if test "$flex_v2" -eq "$flex_ref_v2" ; then
	 if test "$flex_v3" -ge "$flex_ref_v3" ; then
	    modern_flex=yes
	 fi
      fi
   fi

   if test "$modern_flex" == yes ; then
      AC_DEFINE([HAVE_MODERN_FLEX], 1, [Define if flex version is >= $flex_ref_version])
      AC_MSG_RESULT([yes - flex $flex_version])
      AM_CONDITIONAL([BISON_LOCATIONS], true)
   else
      AC_MSG_RESULT([no - flex $flex_version])
      AM_CONDITIONAL([BISON_LOCATIONS], false)
   fi
else
   AM_CONDITIONAL([BISON_LOCATIONS], false)
fi

AC_CONFIG_SUBDIRS(libbrux)

#AC_ARG_WITH(include,
#       AC_HELP_STRING([--with-include=PATH], [Absolute path to the openwince includes]),
#       openwince_inc="$withval")
#AC_CACHE_CHECK([for the openwince includes], openwince_includes_path, [
#       openwince_includes_path="no"
#       for openwince_include in $openwince_inc /usr/include/openwince /usr/local/include/openwince; do
#               if test -f "$openwince_include/common.h"; then
#                       openwince_includes_path="$openwince_include"
#                       break
#               fi
#       done
#])
#if test "$openwince_includes_path" = "no"; then
#       AC_MSG_ERROR([The openwince include package can not be found.])
#fi
#CPPFLAGS="$CPPFLAGS -I$openwince_includes_path -I$openwince_includes_path/device -I$openwince_includes_path/arm"

AC_CONFIG_SUBDIRS(inclow)
CPPFLAGS="$CPPFLAGS -I\$(top_srcdir)/inclow"
CPPFLAGS="$CPPFLAGS -I\$(top_srcdir)/inclow/device"
CPPFLAGS="$CPPFLAGS -I\$(top_srcdir)/inclow/arm"

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
AC_PATH_PROG(HAVELIBFTDI, libftdi-config, no)
if test "$HAVELIBFTDI" != "no" ; then
dnl   LIBFTDI_CFLAGS=`$HAVELIBFTDI --cflags`
   AM_CONDITIONAL(HAVE_LIBFTDI, true)
   LIBFTDI_LIBS=`$HAVELIBFTDI --libs`
   CFLAGS="$CFLAGS $LIBFTDI_CFLAGS"
   LIBS="$LIBS $LIBFTDI_LIBS"
   AC_DEFINE(HAVE_LIBFTDI, 1, [Define if you have libftdi])
else
   AM_CONDITIONAL(HAVE_LIBFTDI, false)
   AC_MSG_WARN([*** libftdi-config not found. No support for FTDI-based USB JTAG cables via libftdi.])
fi

dnl check for libusb-config
AC_PATH_PROG(HAVELIBUSB, libusb-config, no) 
if test "$HAVELIBUSB" != "no" ; then
dnl   LIBUSB_CFLAGS=`$HAVELIBUSB --cflags`
   AM_CONDITIONAL(HAVE_LIBUSB, true)
   LIBUSB_LIBS=`$HAVELIBUSB --libs`
   CFLAGS="$CFLAGS $LIBUSB_CFLAGS"
   LIBS="$LIBS $LIBUSB_LIBS"
   AC_DEFINE(HAVE_LIBUSB, 1, [Define if you have libusb])
else
   AM_CONDITIONAL(HAVE_LIBUSB, false)
   AC_MSG_WARN([*** libusb-config not found. Omitting support for some USB JTAG cables.])
fi

dnl check for availability of libftd2xx
AC_CHECK_LIB([ftd2xx], [FT_Open], [
   AM_CONDITIONAL(HAVE_LIBFTD2XX, true)
   AC_DEFINE(HAVE_LIBFTD2XX, 1, [define if you have libftd2xx])
   LIBS="-lftd2xx $LIBS"
],[
   AM_CONDITIONAL(HAVE_LIBFTD2XX, false)
   AC_MSG_WARN([*** libftd2xx not found. No support for FTDI-based USB JTAG cables via libftd2xx.])
])

CFLAGS="$CFLAGS -Wall"
CPPFLAGS="$CPPFLAGS -I\$(top_srcdir) -I\$(top_srcdir)/include"

AC_OUTPUT


dnl
dnl Configuration summary
dnl
if test ${HAVELIBFTDI:-no} != no ; then
   FLAG_HAVELIBFTDI=yes
else
   FLAG_HAVELIBFTDI=no
fi
if test ${HAVELIBUSB:-no} != no ; then
   FLAG_HAVELIBUSB=yes
else
   FLAG_HAVELIBUSB=no
fi
if echo $LIBS | grep -q 'ftd2xx' ; then
   FLAG_HAVELIBFTD2XX=yes
else
   FLAG_HAVELIBFTD2XX=no
fi
AC_MSG_RESULT([
jtag is now configured for

  USB cable support      : $FLAG_HAVELIBUSB
  FTDI cable support
    via libftdi          : $FLAG_HAVELIBFTDI
    via libftd2xx        : $FLAG_HAVELIBFTD2XX
  Parser error locations : $modern_flex ($LEX $flex_version)
])
