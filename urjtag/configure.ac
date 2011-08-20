#
# $Id$
#
# Copyright (C) 2007, 2008 Kolja Waschk and other
#    UrJTAG.org developers, (C) 2002, 2003 ETC s.r.o.
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
# Written by Marcel Telka <marcel@telka.sk>, 2002, 2003,
#  and other UrJTAG authors, 2007, 2008, ...
#

AC_INIT(UrJTAG,0.10,http://urjtag.org,urjtag)

dnl autoconf-2.61 was released in 2006
AC_PREREQ(2.61)
AC_COPYRIGHT([Copyright (C) 2007/08 Kolja Waschk and other UrJTAG authors; 2002/03 ETC s.r.o.])
AC_REVISION($Revision$)

# Get SVN Revision - idea from xfce-power-manager (C) 2007 by B.Tarricone
dnl get svn revision
AC_MSG_CHECKING([code revision])
SVN=`which svn`
SVN_REVISION=0
if test -n "$SVN" -a -x "$SVN"; then
    SVN_REVISION=`LC_ALL=C svn info -R "$srcdir" 2>/dev/null | awk 'BEGIN { x=0 } /^Revision: / { if($2>x) x=$2; } END { printf "%d\n",x }'`
    SVN_FROM="svn info"
fi
if test "$SVN_REVISION" -eq 0 -a -e "$srcdir/ChangeLog"; then
    SVN_REVISION=`awk 'BEGIN { x=0 } /Revision: / { if($2>x) x=$2; } END { printf "%d\n",x }' "$srcdir/ChangeLog"`
    SVN_FROM="ChangeLog"
fi
if test "$SVN_REVISION" -eq 0; then
    SVN_REVISION=`git log -n1 --pretty=format:%b 2>/dev/null | sed -n '/^git-svn-id:/{s:.*@::;s: .*::;p}'`
    SVN_FROM="git-svn log"
fi
if test "$SVN_REVISION" -eq "0" ; then
    AC_MSG_RESULT([unknown, using 0])
else
    AC_MSG_RESULT([from $SVN_FROM: $SVN_REVISION])
fi
AC_DEFINE_UNQUOTED([SVN_REVISION], "$SVN_REVISION", [subversion revision number])
AC_SUBST([SVN_REVISION])

AC_CONFIG_AUX_DIR(tools)

dnl automake-1.10 was released in 2006
AM_INIT_AUTOMAKE([1.10 check-news dist-bzip2])
dnl silent rules requires automake-1.11+
m4_ifdef([AM_SILENT_RULES],
	[AM_SILENT_RULES([yes])],
	[AC_SUBST([AM_DEFAULT_VERBOSITY], [1])])

AC_CONFIG_FILES(
	Makefile
	doc/Makefile
	data/Makefile
	extra/fjmem/Makefile
	include/Makefile
	include/urjtag/Makefile
	src/Makefile
	src/urjtag.pc
	src/lib/Makefile
	src/tap/Makefile
	src/part/Makefile
	src/bus/Makefile
	src/flash/Makefile
	src/cmd/Makefile
	src/svf/Makefile
	src/bsdl/Makefile
	src/stapl/Makefile
	src/jim/Makefile
	src/pld/Makefile
	src/global/Makefile
	src/apps/jtag/Makefile
	src/apps/bsdl2jtag/Makefile
	src/bfin/Makefile
	po/Makefile.in
)

AM_MAINTAINER_MODE

AM_CONFIG_HEADER(config.h)
AM_CONFIG_HEADER(include/urjtag/urjtag.h)

AC_GNU_SOURCE

dnl gettext-0.16.1 was released in 2006
AM_GNU_GETTEXT(external)
AM_GNU_GETTEXT_VERSION(0.16.1)

LT_INIT
AC_PROG_CC
AC_PROG_YACC
AC_PROG_CC_C99
PKG_PROG_PKG_CONFIG


AC_SEARCH_LIBS([ioperm], [ioperm])
if test "$ac_cv_search_ioperm" != "no"; then
	AC_DEFINE(HAVE_IOPERM, 1, [Define to 1 if you have the ioperm() function])
	HAVE_IOPERM="yes"
else
	AC_SEARCH_LIBS([i386_set_ioperm], [i386])
	if test "$ac_cv_search_i386_set_ioperm" != "no"; then
		AC_DEFINE(HAVE_I386_SET_IOPERM, 1,
			[Define to 1 if you have the i386_set_ioperm() function])
		HAVE_I386_SET_IOPERM="yes"
	fi
fi

AC_CHECK_FUNCS(m4_flatten([
	_sleep
	getdelim
	geteuid
	getline
	getuid
	nanosleep
	pread
	swprintf
	usleep
]))

AC_CHECK_FUNC(clock_gettime, [], [ AC_CHECK_LIB(rt, clock_gettime) ])


dnl check for sigaction with SA_ONESHOT or SA_RESETHAND
AC_TRY_COMPILE([#include <signal.h>], [
#ifndef SA_ONESHOT
#  define SA_ONESHOT SA_RESETHAND
#endif
int sa_flags = SA_ONESHOT;
],
[AC_DEFINE([HAVE_SIGACTION_SA_ONESHOT], [1], [Have a defined SA_ONESHOT])],
[])

AC_CHECK_HEADERS([linux/ppdev.h], [HAVE_LINUX_PPDEV_H="yes"])
AC_CHECK_HEADERS([dev/ppbus/ppi.h], [HAVE_DEV_PPBUS_PPI_H="yes"])
AC_CHECK_HEADERS(m4_flatten([
	wchar.h
	windows.h
	sys/wait.h
]))

AC_CHECK_TYPE([wchar_t], [],
	[AC_DEFINE([wchar_t], [char], [Make sure we have wchar_t])],
	[#include <stddef.h>])

AC_CHECK_TYPE([struct timespec],
	[AC_DEFINE(HAVE_STRUCT_TIMESPEC, 1, [Define to 1 if you have the struct timespec type])],
	[], [[#include <time.h>]])

dnl See if the windows target is polluting the namespace on us which
dnl will break some of the lex/yacc (e.g. bsdl) builds.  We need both
dnl defines as some versions of mingw have changed names over time.
AS_IF([test "x$ac_cv_header_windows_h" = "xyes"],[
	AC_MSG_CHECKING([for IN/OUT defines in windows.h])
	AC_TRY_COMPILE([#include <windows.h>], [
#if defined(IN) || defined(OUT)
# error "IN/OUT defined"
#endif
],
	[AC_MSG_RESULT([no])],[
	AS_VAR_APPEND([CPPFLAGS], " -DNO_W32_PSEUDO_MODIFIERS -D_NO_W32_PSEUDO_MODIFIERS")
	AC_MSG_RESULT([yes])
	])
])

VL_LIB_READLINE


dnl check for libusb

AC_ARG_WITH([libusb],
  [AS_HELP_STRING([--with-libusb], [lib for USB JTAG cables @<:@default=detect 1.0 0.1@:>@])],,
    [with_libusb=detect])

HAVELIBUSB=no
LIBUSB_LIBS=
AS_IF([test "x$with_libusb" != xno], [
  AS_IF([test "x$with_libusb" != x0.1 -a \
              "x$with_libusb" != x1.0 -a \
              "x$with_libusb" != xdetect -a \
              "x$with_libusb" != xyes], [
    HAVELIBUSB=manual
    LIBUSB_LIBS="-L$with_libusb -L$with_libusb/lib -lusb"
    LIBUSB_CFLAGS="-I$with_libusb -I$with_libusb/include"
  ], [
    AS_IF([test "x$with_libusb" != x0.1 -a "x$HAVELIBUSB" = xno], [
      PKG_CHECK_MODULES(LIBUSB, libusb-1.0, [
          HAVELIBUSB=1.0
          AC_DEFINE(HAVE_LIBUSB1, 1, [Define if you have libusb-1.0])
        ], [:])
    ])
    AS_IF([test "x$with_libusb" != x1.0 -a "x$HAVELIBUSB" = xno], [
      PKG_CHECK_MODULES(LIBUSB, libusb, [HAVELIBUSB=0.1], [:])
    ])
    AS_IF([test "x$HAVELIBUSB" = xno], [
      AS_IF([test "x$with_libusb" != xdetect], [
        AC_MSG_ERROR([*** libusb not detected])
      ], [
        AC_MSG_WARN([*** libusb not detected. No support for USB JTAG cables via libusb.])
      ])
    ],[
      AC_DEFINE(HAVE_LIBUSB, 1, [Define if you have libusb])
    ])
  ])
  AS_VAR_APPEND([CPPFLAGS], " $LIBUSB_CFLAGS")
])
AC_SUBST(LIBUSB_LIBS)
AM_CONDITIONAL(HAVE_LIBUSB, [test "x$HAVELIBUSB" != "xno"])


dnl Use FTDI library?

AC_ARG_WITH([libftdi],
  [AS_HELP_STRING([--with-libftdi],
    [use libftdi for FTDI-based cables])],
    [], [with_libftdi=check])

HAVELIBFTDI=no
LIBFTDI_LIBS=
AS_IF([test "x$with_libftdi" != xno], [
  save_LIBS=$LIBS
  save_CPPFLAGS=$CPPFLAGS
  PKG_CHECK_MODULES(LIBFTDI, libftdi, [:;], [dnl
    AS_IF([test "x$with_libftdi" != xyes -a "x$with_libftdi" != xcheck], [
      LIBFTDI_LIBS="-L$with_libftdi -lftdi"
      LIBFTDI_CFLAGS="-I$with_libftdi"
    ],[
      AC_PATH_TOOL(LIBFTDI_CONFIG, libftdi-config, no)
      AS_IF([test "$LIBFTDI_CONFIG" != "no"],[
        LIBFTDI_LIBS=`$LIBFTDI_CONFIG --libs`
        LIBFTDI_CFLAGS=`$LIBFTDI_CONFIG --cflags`
      ])
    ])
  ])
  AS_VAR_APPEND([LIBS], " $LIBFTDI_LIBS")
  AS_VAR_APPEND([CPPFLAGS], " $LIBFTDI_CFLAGS")
  AC_CHECK_FUNC([ftdi_usb_open], [
    AC_DEFINE(HAVE_LIBFTDI, 1, [Define if you have libftdi])
    HAVELIBFTDI=yes
  ],[
    AC_MSG_WARN([*** libftdi not detected. No support for FTDI-based USB JTAG cables via libftdi.])
    CPPFLAGS=$save_CPPFLAGS
  ])
  AC_CHECK_FUNC([ftdi_read_data_submit], [
    AS_IF([test "x$HAVELIBUSB" = "x0.1"], [
      AC_MSG_ERROR([this libftdi cannot be used with libusb-0.1, libusb-1.0 is needed])
    ])
    AC_DEFINE(HAVE_LIBFTDI_ASYNC_MODE, 1, [Define if libftdi support async mode])
    HAVELIBFTDI_ASYNCMODE=yes
  ])
  LIBS=$save_LIBS
])
AC_SUBST(LIBFTDI_LIBS)
AM_CONDITIONAL(HAVE_LIBFTDI, [test "x$HAVELIBFTDI" = "xyes"])
AM_CONDITIONAL(HAVE_LIBFTDI_ASYNC_MODE, [test "x$HAVELIBFTDI_ASYNCMODE" = "xyes"])


dnl Use FTDI ftd2xx library?

AC_ARG_WITH([ftd2xx],
  [AS_HELP_STRING([--with-ftd2xx],
    [use ftd2xx library for FTDI-based cables])],
    [], [with_ftd2xx=check])

FTD2XXLIB=
AS_IF([test "x$with_ftd2xx" = xyes -o "x$with_ftd2xx" = xcheck], [
  AC_CHECK_LIB([ftd2xx], [FT_OpenEx], [
    HAVELIBFTD2XX=yes
    LIBS="-lftd2xx $LIBS"
  ],[
    AC_MSG_WARN([*** libftd2xx not found. No support for FTDI-based USB JTAG cables via libftd2xx.])
  ])
],[
  AS_IF([test "x$with_ftd2xx" != xno], [
    HAVELIBFTD2XX=yes
    case $host in
    *cygwin*|*mingw*)
      CFLAGS="$CFLAGS -I$with_ftd2xx"
      AS_IF([test -d "$with_ftd2xx/i386"], [
        FTD2XXLIB="$with_ftd2xx/i386/ftd2xx.lib"
      ],[
        FTD2XXLIB="$with_ftd2xx/ftd2xx.lib"
      ])
      ;;
    *)
      CFLAGS="$CFLAGS -I$with_ftd2xx -L$with_ftd2xx"
      LIBS="-lftd2xx $LIBS"
      ;;
    esac
  ],)
])
AS_IF([test "x$HAVELIBFTD2XX" = xyes], [
  AM_CONDITIONAL(HAVE_LIBFTD2XX, true)
  AC_DEFINE(HAVE_LIBFTD2XX, 1, [define if you have libftd2xx])
],[
  AM_CONDITIONAL(HAVE_LIBFTD2XX, false)
])
AC_SUBST(FTD2XXLIB)


dnl Use InpOut I/O library?
dnl http://www.highrez.co.uk/Downloads/InpOut32/default.htm

AC_ARG_WITH([inpout32],
  [AS_HELP_STRING([--with-inpout32],
    [use InpOut32.dll for parallel port access on Windows])],
    [], [with_inpout32=auto])

case $host in
  *cygwin*|*mingw*) ;;
  *) with_inpout32=no ;;
esac

HAVE_INPOUTXX=no
AS_IF([test "x$with_inpout32" != xno], [
  AC_CHECK_HEADER([inpout32.h], [
    HAVE_INPOUTXX=yes
    AC_DEFINE(HAVE_INPOUTXX, 1, [define if you have inpout32.dll])
  ],[:],[
	#include <windows.h>
	#include <stdbool.h>
  ])
  AS_IF([test "x$with_inpout32$HAVE_INPOUTXX" = "xyesno"], [
    AC_MSG_ERROR([inpout32 support requested, but not found])
  ])
])
AM_CONDITIONAL(HAVE_INPOUTXX, [test "x$HAVE_INPOUTXX" = xyes])


WARNINGCFLAGS="-Wall -Wmissing-prototypes -Wstrict-prototypes -Wpointer-arith"
AC_ARG_ENABLE([werror],
  [AS_HELP_STRING([--enable-werror], [build with -Werror])],
  [], [enable_werror=maybe])
AS_IF([test "x$enable_werror" = xmaybe -a -d .svn], [
  enable_werror=yes
])
AS_IF([test "x$enable_werror" = xyes], [
  WARNINGCFLAGS="$WARNINGCFLAGS -Werror"
])
AC_SUBST(WARNINGCFLAGS)

AS_VAR_PREPEND([CPPFLAGS], '-I$(top_srcdir) -I$(top_srcdir)/include ')

AC_ARG_ENABLE([apps],
  [AS_HELP_STRING([--disable-apps],
    [disable building the jtag and bsdl2jtag main programs])],,
    [enable_apps=yes])

AS_IF([test "x$enable_apps" = xyes], [
  AM_CONDITIONAL(ENABLE_APPS, true)
  AC_DEFINE(ENABLE_APPS, 1, [define if you want to build the jtag and bsdl2jtag main programs])
],[
  AM_CONDITIONAL(ENABLE_APPS, false)
])


# check for lex/flex
AC_PROG_LEX
# check for modern version of flex
AC_PROG_AWK
AC_PROG_SED

AS_IF([test "$LEX" = flex], [
   ver_ge_ver () {
      v1=$1
      v2=$2
      v3=$3
      ref_v1=$4
      ref_v2=$5
      ref_v3=$6

      AS_IF([test -n "$v1" -a "$v1" -gt "$ref_v1"], [
         return 0
      ])
      AS_IF([test -n "$v1" -a "$v1" -eq "$ref_v1"], [
         AS_IF([test "$v2" -gt "$ref_v2"], [
	    return 0
         ])
         AS_IF([test "$v2" -eq "$ref_v2"], [
            AS_IF([test "$v3" -ge "$ref_v3"], [
	       return 0
            ])
         ])
      ])

      return 1
   }

   flex_version=`$LEX --version | $AWK '{print $2}'`
   AS_IF([test "$flex_version" = "version"], [
       flex_version=`$LEX --version | $AWK '{print $3}'`
   ])

   flex_v1=`echo $flex_version | $AWK -F . '{print $1}' | $SED -e 's/[a-zA-Z]//g'`
   flex_v2=`echo $flex_version | $AWK -F . '{print $2}' | $SED -e 's/[a-zA-Z]//g'`
   flex_v3=`echo $flex_version | $AWK -F . '{print $3}' | $SED -e 's/[a-zA-Z]//g'`

   # svf and bsdl lexer require flex >= 2.5.33
   flex_ref_v1=2
   flex_ref_v2=5
   flex_ref_v3=33
   flex_ref_version="$flex_ref_v1.$flex_ref_v2.$flex_ref_v3"
   AC_MSG_CHECKING([for flex >= $flex_ref_version, required for SVF and BSDL lexer])
   #
   AS_IF([ver_ge_ver $flex_v1 $flex_v2 $flex_v3 $flex_ref_v1 $flex_ref_v2 $flex_ref_v3], [
      AC_MSG_RESULT([yes - flex $flex_version])
      svf_lexer=true
      bsdl_lexer=true
   ], [
      AC_MSG_RESULT([no - flex $flex_version])
      svf_lexer=false
      bsdl_lexer=false
   ])
], [
   svf_lexer=false
   bsdl_lexer=false
])


dnl If the transformed svf_flex.c is already existing, it doesn't matter
dnl that flex is too old for building the lexer.
AS_IF([test "x$svf_lexer" = xfalse], [
   AS_IF([test -r src/svf/svf_bison.h], [
      svf_lexer=true
   ])
])

dnl Enable SVF player?
AC_ARG_ENABLE(svf,
[AS_HELP_STRING([--disable-svf], [Disable SVF player])],
[case "${enableval}" in
   yes) svf=true ;;
   no)  svf=false ;;
   *)   AC_MSG_ERROR(bad value ${enableval} for --enable-svf) ;;
 esac],
[svf=$svf_lexer])
AS_IF([test "x$svf" = xtrue], [
  AM_CONDITIONAL(ENABLE_SVF, true)
  AC_DEFINE(ENABLE_SVF, 1, [define if SVF player is enabled])
],[
  AM_CONDITIONAL(ENABLE_SVF, false)

  # SVF is disabled, but the generated files are still required for
  # POTFILES. They simply have to exist as empty files.
  AS_IF([test ! -r src/svf/svf_flex.c], [
    touch src/svf/svf_flex.c
  ])
  AS_IF([test ! -r src/svf/svf_bison.c], [
    touch src/svf/svf_bison.c
  ])
])


dnl If the transformed bsdl_flex.c is already existing, it doesn't matter
dnl that flex is too old for building the lexer.
AS_IF([test "x$bsdl_lexer" = xfalse], [
   AS_IF([test -r src/bsdl/bsdl_bison.h], [
      bsdl_lexer=true
   ])
])

dnl Enable STAPL player?
AC_ARG_ENABLE(stapl,
[AS_HELP_STRING([--disable-stapl], [Disable STAPL player])],
[case "${enableval}" in
   yes) stapl=true ;;
   no)  stapl=false ;;
   *)   AC_MSG_ERROR(bad value ${enableval} for --enable-stapl) ;;
 esac])
AS_IF([test "x$stapl" = xtrue], [
  AM_CONDITIONAL(ENABLE_STAPL, true)
  AC_DEFINE(ENABLE_STAPL, 1, [define if STAPL player is enabled])
],[
  AM_CONDITIONAL(ENABLE_STAPL, false)
])

dnl Enable BSDL subsystem?
AC_ARG_ENABLE(bsdl,
[AS_HELP_STRING([--disable-bsdl], [Disable BSDL subsystem])],
[case "${enableval}" in
   yes) bsdl=true ;;
   no)  bsdl=false ;;
   *)   AC_MSG_ERROR(bad value ${enableval} for --enable-bsdl) ;;
 esac],
[bsdl=$bsdl_lexer])
AS_IF([test "x$bsdl" = xtrue], [
  AM_CONDITIONAL(ENABLE_BSDL, true)
  AC_DEFINE(ENABLE_BSDL, 1, [define if BSDL subsystem is enabled])
],[
  AM_CONDITIONAL(ENABLE_BSDL, false)

  # BSDL is disabled, but the generated files are still required for
  # POTFILES. They simply have to exist as empty files.
  AS_IF([test ! -r src/bsdl/vhdl_flex.c], [
    touch src/bsdl/vhdl_flex.c
  ])
  AS_IF([test ! -r src/bsdl/vhdl_bison.c], [
    touch src/bsdl/vhdl_bison.c
  ])
  AS_IF([test ! -r src/bsdl/bsdl_flex.c], [
    touch src/bsdl/bsdl_flex.c
  ])
  AS_IF([test ! -r src/bsdl/bsdl_bison.c], [
    touch src/bsdl/bsdl_bison.c
  ])
])

dnl Enable experimental brute-force JEDEC flash autodetection?
AC_ARG_ENABLE(jedec-exp,
[AS_HELP_STRING([--enable-jedec-exp], [Enable experimental JEDEC flash detection])],
[case "${enableval}" in
   yes) jedecexp=true ;;
   no)  jedecexp=false ;;
   *)   AC_MSG_ERROR(bad value ${enableval} for --enable-jedec-exp) ;;
 esac],
[jedecexp=false])
AS_IF([test "x$jedecexp" = xtrue], [
  AM_CONDITIONAL(JEDEC_EXP, true)
  AC_DEFINE(JEDEC_EXP, 1, [define if experimental JEDEC flash detection is enabled])
],[
  AM_CONDITIONAL(JEDEC_EXP, false)
])


dnl
dnl URJ_DRIVER_SET([driver set name],
dnl                [drivers to enable by default],
dnl                [drivers to disable by default],
dnl                [extra code to run before processing user list])
dnl
dnl Only the first two arguments are required.  The extra code is useful to
dnl automatically strip out drivers if required external libraries were not
dnl found.
dnl
AC_DEFUN([URJ_DRIVER_SET], [
	AC_DEFUN([DEF_ENABLE_DRIVERS], m4_flatten([$2]))
	AC_DEFUN([DEF_DISABLE_DRIVERS], m4_flatten([$3]))
	AC_DEFUN([ALL_DRIVERS], DEF_ENABLE_DRIVERS DEF_DISABLE_DRIVERS)

AC_ARG_ENABLE($1,
[AS_HELP_STRING([--enable-]$1, [Enable default set or specific ]$1[ drivers:])]
[AS_HELP_STRING([],            ['default' enables:])]
[AS_HELP_STRING([],DEF_ENABLE_DRIVERS)]
m4_ifnblank([$3], [
[AS_HELP_STRING([],            [Disabled by default are:])]
[AS_HELP_STRING([],DEF_DISABLE_DRIVERS)]
])dnl
,
[AS_CASE([${enableval}],
    [yes],  [drivers=default],
    [no],   [drivers=none],
    [none], [drivers=none],
            [drivers=`echo ${enableval} | $SED -e 's/,/ /g'`])]
,
[drivers=default])
	# expand 'default' to default enabled $1 drivers
	drivers=`echo ${drivers} | $SED -e "s/default/DEF_ENABLE_DRIVERS/"`
	m4_ifnblank([$4], $4)

	enabled_drivers=''
	m4_foreach_w([x], ALL_DRIVERS, [
		AC_DEFUN([DRIVER_DEFINE], m4_toupper([ENABLE_]$1[_]x))
		AS_IF([echo "$drivers" | $GREP -q "]x["], [
			AC_DEFINE(DRIVER_DEFINE, 1, [define if ]x[ is enabled])
			AM_CONDITIONAL(DRIVER_DEFINE, true)
			AS_VAR_APPEND([enabled_drivers], "x ")
		], [
			AM_CONDITIONAL(DRIVER_DEFINE, false)
		])
	])
	[enabled_]$1[_drivers]=$enabled_drivers
	$1[drivers]=$drivers
])



# Enable bus drivers
URJ_DRIVER_SET([bus], [
	arm9tdmi
	au1500
	avr32
	bcm1250
	blackfin
	bscoach
	ejtag
	ejtag_dma
	fjmem
	ixp425
	ixp435
	ixp465
	jopcyc
	h7202
	lh7a400
	mpc5200
	mpc824x
	mpc837x
	ppc405ep
	ppc440gx_ebc8
	prototype
	pxa2x0
	pxa27x
	s3c4510
	sa1110
	sh7727
	sh7750r
	sh7751r
	sharc_21065L
	sharc_21369_ezkit
	slsup3
	tx4925
	zefant_xs3
])


# Enable cable drivers
URJ_DRIVER_SET([cable], [
	arcom
	byteblaster
	dlc5
	ea253
	ei012
	ft2232
	gpio
	ice100
	igloo
	jlink
	keithkoep
	lattice
	mpcbdm
	triton
	usbblaster
	vsllink
	wiggler
	xpc
],[
	ep9307
	jim
	ts7800
],[
	# automatically disable cable drivers when a required feature is not available
	AS_IF([test "x$HAVELIBFTDI" != "xyes" -a "x$HAVELIBFTD2XX" != "xyes"], [
		drivers=`echo ${drivers} | $SED -e "s/ft2232//" -e "s/usbblaster//"`
	])
	AS_IF([test "x$HAVELIBUSB" = "xno"], [
		drivers=`echo ${drivers} | $SED \
			-e s/ice100// \
			-e s/jlink// \
			-e s/vsllink// \
			-e s/xpc//`
	])
	AS_IF([test "x$ac_cv_func_pread" != "xyes"], [
		drivers=`echo ${drivers} | $SED -e s/gpio//`
	])
])
dnl the "fake" jim cable driver is special
AS_IF([echo "$enabled_cable_drivers" | $GREP -q jim], [
	AC_DEFINE([ENABLE_JIM], 1, [define if jim is enabled])
	AM_CONDITIONAL([ENABLE_JIM], true)
],[
	AM_CONDITIONAL([ENABLE_JIM], false)
])


# Enable lowlevel drivers
URJ_DRIVER_SET([lowlevel], [
	direct
	ftdi
	ftd2xx
	ppdev
	ppi
],[
],[
	# automatically disable lowlevel drivers when a required feature is not available
	AS_IF([test "x$HAVELIBFTDI" != "xyes"], [
		drivers=`echo ${drivers} | $SED -e "s/ftdi//"`
	])
	AS_IF([test "x$HAVELIBFTD2XX" != "xyes"], [
		drivers=`echo ${drivers} | $SED -e "s/ftd2xx//"`
	])
	AS_IF([test "x$HAVE_LINUX_PPDEV_H" != "xyes"], [
		drivers=`echo ${drivers} | $SED -e "s/ppdev//"`
	])
	AS_IF([test "x$HAVE_DEV_PPBUS_PPI_H" != "xyes"], [
		drivers=`echo ${drivers} | $SED -e "s/ppi//"`
	])
	AS_IF([test "x$HAVE_IOPERM" != "xyes" -a "x$HAVE_I386_SET_IOPERM" != "xyes" -a "x$HAVE_INPOUTXX" != "xyes" ], [
		drivers=`echo ${drivers} | $SED -e "s/direct//"`
	])
])


dnl Enable a relocatable jtag?
AC_ARG_ENABLE(relocatable,
[AS_HELP_STRING([--enable-relocatable], [Enable relocatable paths])],
[relocatable=$enableval], [relocatable=no])
AS_IF([test "x$relocatable" = xyes], [
  AM_CONDITIONAL(JTAG_RELOCATABLE, true)
  AC_DEFINE(JTAG_RELOCATABLE, 1, [define for relocatable paths])
],[
  AM_CONDITIONAL(JTAG_RELOCATABLE, false)
])


dnl Enable use of DMALLOC library?
AC_ARG_ENABLE(dmalloc,
[AS_HELP_STRING([--enable-dmalloc], [Enable heap debugging with dmalloc library])],
[dmalloc=$enableval], [dmalloc=no])
AS_IF([test "x$dmalloc" = xyes], [
  AM_CONDITIONAL(DMALLOC, true)
  AC_DEFINE(DMALLOC, 1, [define for dmalloc library])
],[
  AM_CONDITIONAL(DMALLOC, false)
])


# Enable flash multi-byte write mode?
AC_ARG_ENABLE(flash-multi-byte,
[AS_HELP_STRING([--disable-flash-multi-byte], [Disable flash multi-byte write mode])],
[flash_multi_byte=$enableval], [flash_multi_byte=yes])
AS_IF([test "x$flash_multi_byte" = xyes], [
  AC_DEFINE(FLASH_MULTI_BYTE, 1, [define for flash multi-byte write mode])
])


AC_OUTPUT

dnl
dnl Configuration summary
dnl
FLAG_HAVELIBUSB=$HAVELIBUSB
AS_IF([test "x$HAVELIBFTDI_ASYNCMODE" = "xyes"], [
  FLAG_HAVELIBFTDI_ASYNCMODE="(have async mode)"
],[
  FLAG_HAVELIBFTDI_ASYNCMODE="(no async mode)"
])
AC_DEFUN([MAKE_YESNO_VAR],[dnl
  AS_IF([test ${$1:-$2} != $2], [dnl
     FLAG_$1=yes
  ], [dnl
     FLAG_$1=no
  ])
])
MAKE_YESNO_VAR([HAVELIBFTDI], [no])
MAKE_YESNO_VAR([HAVELIBFTD2XX], [no])
MAKE_YESNO_VAR([HAVE_INPOUTXX], [no])
MAKE_YESNO_VAR([svf], [false])
MAKE_YESNO_VAR([bsdl], [false])
MAKE_YESNO_VAR([stapl], [false])
AC_MSG_NOTICE([

urjtag is now configured for

  Libraries:
    libusb     : $FLAG_HAVELIBUSB
    libftdi    : $FLAG_HAVELIBFTDI $FLAG_HAVELIBFTDI_ASYNCMODE
    libftd2xx  : $FLAG_HAVELIBFTD2XX
    inpout32   : $FLAG_HAVE_INPOUTXX

  Subsystems:
    SVF        : $FLAG_svf
    BSDL       : $FLAG_bsdl
    STAPL      : $FLAG_stapl

  Drivers:
    Bus        : $enabled_bus_drivers
    Cable      : $enabled_cable_drivers
    Lowlevel   : $enabled_lowlevel_drivers
])
