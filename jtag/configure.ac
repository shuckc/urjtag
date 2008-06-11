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

AC_INIT(UrJTAG,0.8,http://urjtag.org,urjtag)

AC_PREREQ(2.54)
AC_COPYRIGHT([Copyright (C) 2007/08 Kolja Waschk and other UrJTAG authors; 2002/03 ETC s.r.o.])
AC_REVISION($Revision$)

# Get SVN Revision - idea from xfce-power-manager (C) 2007 by B.Tarricone
dnl get svn revision
AC_MSG_CHECKING([code revision])
SVN=`which svn`
SVN_REVISION=0
if test -n "$SVN" -a -x "$SVN"; then
    SVN_REVISION=`LC_ALL=C svn info -R "$srcdir" | awk 'BEGIN { x=0 } /^Revision: / { if($2>x) x=$2; } END { printf "%d\n",x }'`
    if test "$SVN_REVISION" -ne "0" ; then
        AC_MSG_RESULT([from svn info: $SVN_REVISION])
    fi
fi
if test "$SVN_REVISION" -eq 0 -a -e "$srcdir/ChangeLog"; then
    SVN_REVISION=`awk 'BEGIN { x=0 } /Revision: / { if($2>x) x=$2; } END { printf "%d\n",x }' "$srcdir/ChangeLog"`
    if test "$SVN_REVISION" -ne "0" ; then
        AC_MSG_RESULT([from ChangeLog: $SVN_REVISION])
    fi
fi
if test "$SVN_REVISION" -eq "0" ; then
    AC_MSG_RESULT([unknown, using 0])
fi
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
	src/flash/Makefile
	src/cmd/Makefile
	src/svf/Makefile
	src/bsdl/Makefile
	src/jim/Makefile
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

AC_CHECK_FUNCS(getline getdelim)
AC_CHECK_FUNCS(swprintf)

AC_CHECK_FUNC(clock_gettime, [], [ AC_CHECK_LIB(rt, clock_gettime) ])

AC_CHECK_HEADERS([linux/ppdev.h], [HAVE_LINUX_PPDEV_H="yes"])
AC_CHECK_HEADERS([dev/ppbus/ppi.h], [HAVE_DEV_PPBUS_PPI_H="yes"])
AC_CHECK_HEADERS(stropts.h)
AC_CHECK_HEADERS(wchar.h)

VL_LIB_READLINE


dnl check for libusb-config

AC_ARG_WITH([libusb],
  [AS_HELP_STRING([--with-libusb],
    [use libusb for some USB JTAG cables])],,
    [with_libusb=check])

AS_IF([test "x$with_libusb" != xno], [
  save_LIBS=$LIBS
  save_CPPFLAGS=$CPPFLAGS
  AS_IF([test "x$with_libusb" != xyes -a "x$with_libusb" != xcheck], [
    case $host in
    *cygwin*)
      USBLIBS="-L$with_libusb/lib/gcc -lusb"
      USBCLAGS="-I$with_libusb/include"
      ;;
    *)
      USBLIBS="-Lwith_libusb -lusb"
      USBCFLAGS="-I$with_libusb"
      ;;
    esac
  ],[
    AC_PATH_TOOL(LIBUSB_CONFIG, libusb-config, no) 
    AS_IF([test "$LIBUSB_CONFIG" != "no"],[
      USBLIBS=`$LIBUSB_CONFIG --libs`
      USBCFLAGS=`$LIBUSB_CONFIG --cflags`
    ],)
  ])
  LIBS="$LIBS $USBLIBS"
  CPPFLAGS="$CPPFLAGS $USBCFLAGS"
  AC_CHECK_FUNC([usb_find_devices], [
    AC_DEFINE(HAVE_LIBUSB, 1, [Define if you have libusb])
    HAVELIBUSB=yes
  ],[
    AC_MSG_WARN([*** libusb not detected. No support for USB JTAG cables via libusb.])
    LIBS=$save_LIBS
    CPPFLAGS=$save_CPPFLAGS
  ])
],)

AS_IF([test "x$HAVELIBUSB" = "xyes"],[
  AM_CONDITIONAL(HAVE_LIBUSB, true)
],[
  AM_CONDITIONAL(HAVE_LIBUSB, false)
])


dnl Use FTDI library?

AC_ARG_WITH([libftdi],
  [AS_HELP_STRING([--with-libftdi],
    [use libftdi for FTDI-based cables])],
    [], [with_libftdi=check])

AS_IF([test "x$with_libftdi" != xno], [
  save_LIBS=$LIBS
  save_CPPFLAGS=$CPPFLAGS
  AS_IF([test "x$with_libftdi" != xyes -a "x$with_libftdi" != xcheck], [
    FTDILIBS="-L$with_libftdi -lftdi"
    FTDICFLAGS="-I$with_libftdi"
  ],[
    AC_PATH_TOOL(LIBFTDI_CONFIG, libftdi-config, no) 
    AS_IF([test "$LIBFTDI_CONFIG" != "no"],[
      FTDILIBS=`$LIBFTDI_CONFIG --libs`
      FTDICFLAGS=`$LIBFTDI_CONFIG --cflags`
    ],)
  ])
  LIBS="$FTDILIBS $LIBS"
  CPPFLAGS="$CPPFLAGS $FTDICFLAGS"
  AC_CHECK_FUNC([ftdi_usb_open], [
    AC_DEFINE(HAVE_LIBFTDI, 1, [Define if you have libftdi])
    HAVELIBFTDI=yes
  ],[
    AC_MSG_WARN([*** libftdi not detected. No support for FTDI-based USB JTAG cables via libftdi.])
    LIBS=$save_LIBS
    CPPFLAGS=$save_CPPFLAGS
  ])
],)

AS_IF([test "x$HAVELIBFTDI" = "xyes"],[
  AM_CONDITIONAL(HAVE_LIBFTDI, true)
],[
  AM_CONDITIONAL(HAVE_LIBFTDI, false)
])


dnl Use FTDI ftd2xx library?

AC_ARG_WITH([ftd2xx],
  [AS_HELP_STRING([--with-ftd2xx],
    [use ftd2xx library for FTDI-based cables])],
    [], [with_ftd2xx=check])

FTD2XXLIB=
AS_IF([test "x$with_ftd2xx" = xyes -o "x$with_ftd2xx" = xcheck], [
  AC_CHECK_LIB([ftd2xx], [FT_Open], [
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


CFLAGS="$CFLAGS -Wall"
CPPFLAGS="$CPPFLAGS -I\$(top_srcdir) -I\$(top_srcdir)/include"


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
   AS_IF([test -r src/svf/svf_flex.c], [
      svf_lexer=true
   ])
])

dnl Enable SVF player?
AC_ARG_ENABLE(svf,
[AS_HELP_STRING([--enable-svf], [Enable SVF player (default is enabled)])],
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
])


dnl If the transformed bsdl_flex.c is already existing, it doesn't matter
dnl that flex is too old for building the lexer.
AS_IF([test "x$bsdl_lexer" = xfalse], [
   AS_IF([test -r src/bsdl/bsdl_flex.c], [
      bsdl_lexer=true
   ])
])

dnl Enable BSDL subsystem?
AC_ARG_ENABLE(bsdl,
[AS_HELP_STRING([--enable-bsdl], [Enable BSDL subsystem (default is enabled)])],
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


# Macro for driver include check
AC_DEFUN([CHECK_DRIVER], [
    AS_IF([echo "$1" | $GREP -q $3], [
       AC_DEFINE([$4], 1, [define if $3 is enabled])
       AM_CONDITIONAL([$4], true)
       $2="${$2}$3 "
    ], [
       AM_CONDITIONAL([$4], false)
    ])
])


# Enable bus drivers
AC_DEFUN([DEF_ENABLE_BUSDRIVERS], [\
au1500 avr32 bcm1250 bf527_ezkit bf533_stamp bf533_ezkit bf537_stamp bf537_ezkit bf548_ezkit bf561_ezkit ejtag \
fjmem ixp425 jopcyc h7202 lh7a400  mpc5200 mpc824x ppc405ep ppc440gx_ebc8 prototype pxa2x0 pxa27x \
s3c4510 sa1110 sh7727 sh7750r sh7751r sharc_21065L slsup3 tx4925 zefant_xs3])
AC_ARG_ENABLE(bus,
[AS_HELP_STRING([--enable-bus], [Enable default set or specific bus drivers:])]
[AS_HELP_STRING([],             ['default' enables:])]
[AS_HELP_STRING([],DEF_ENABLE_BUSDRIVERS)]
,
[AS_CASE([${enableval}],
    [yes],  [busdrivers=default],
    [no],   [busdrivers=none],
    [none], [busdrivers=none],
            [busdrivers=`echo ${enableval} | $SED -e 's/,/ /g'`])],
[busdrivers=default])
# expand 'default' to default enabled busdrivers
busdrivers=`echo ${busdrivers} | $SED -e "s/default/DEF_ENABLE_BUSDRIVERS/"`
#
enabled_bus_drivers=''
CHECK_DRIVER([$busdrivers], [enabled_bus_drivers], [au1500],        [ENABLE_BUS_AU1500])
CHECK_DRIVER([$busdrivers], [enabled_bus_drivers], [avr32],         [ENABLE_BUS_AVR32])
CHECK_DRIVER([$busdrivers], [enabled_bus_drivers], [bcm1250],       [ENABLE_BUS_BCM1250])
CHECK_DRIVER([$busdrivers], [enabled_bus_drivers], [bf527_ezkit],   [ENABLE_BUS_BF527_EZKIT])
CHECK_DRIVER([$busdrivers], [enabled_bus_drivers], [bf533_stamp],   [ENABLE_BUS_BF533_STAMP])
CHECK_DRIVER([$busdrivers], [enabled_bus_drivers], [bf533_ezkit],   [ENABLE_BUS_BF533_EZKIT])
CHECK_DRIVER([$busdrivers], [enabled_bus_drivers], [bf537_stamp],   [ENABLE_BUS_BF537_STAMP])
CHECK_DRIVER([$busdrivers], [enabled_bus_drivers], [bf537_ezkit],   [ENABLE_BUS_BF537_EZKIT])
CHECK_DRIVER([$busdrivers], [enabled_bus_drivers], [bf548_ezkit],   [ENABLE_BUS_BF548_EZKIT])
CHECK_DRIVER([$busdrivers], [enabled_bus_drivers], [bf561_ezkit],   [ENABLE_BUS_BF561_EZKIT])
CHECK_DRIVER([$busdrivers], [enabled_bus_drivers], [ejtag],         [ENABLE_BUS_EJTAG])
CHECK_DRIVER([$busdrivers], [enabled_bus_drivers], [fjmem],         [ENABLE_BUS_FJMEM])
CHECK_DRIVER([$busdrivers], [enabled_bus_drivers], [ixp425],        [ENABLE_BUS_IXP425])
CHECK_DRIVER([$busdrivers], [enabled_bus_drivers], [jopcyc],        [ENABLE_BUS_JOPCYC])
CHECK_DRIVER([$busdrivers], [enabled_bus_drivers], [h7202],         [ENABLE_BUS_H7202])
CHECK_DRIVER([$busdrivers], [enabled_bus_drivers], [lh7a400],       [ENABLE_BUS_LH7A400])
CHECK_DRIVER([$busdrivers], [enabled_bus_drivers], [mpc5200],       [ENABLE_BUS_MPC5200])
CHECK_DRIVER([$busdrivers], [enabled_bus_drivers], [mpc824x],       [ENABLE_BUS_MPC824X])
CHECK_DRIVER([$busdrivers], [enabled_bus_drivers], [ppc405ep],      [ENABLE_BUS_PPC405EP])
CHECK_DRIVER([$busdrivers], [enabled_bus_drivers], [ppc440gx_ebc8], [ENABLE_BUS_PPC440GX_EBC8])
CHECK_DRIVER([$busdrivers], [enabled_bus_drivers], [prototype],     [ENABLE_BUS_PROTOTYPE])
CHECK_DRIVER([$busdrivers], [enabled_bus_drivers], [pxa2x0],        [ENABLE_BUS_PXA2X0])
CHECK_DRIVER([$busdrivers], [enabled_bus_drivers], [pxa27x],        [ENABLE_BUS_PXA27X])
CHECK_DRIVER([$busdrivers], [enabled_bus_drivers], [s3c4510],       [ENABLE_BUS_S3C4510])
CHECK_DRIVER([$busdrivers], [enabled_bus_drivers], [sa1110],        [ENABLE_BUS_SA1110])
CHECK_DRIVER([$busdrivers], [enabled_bus_drivers], [sh7727],        [ENABLE_BUS_SH7727])
CHECK_DRIVER([$busdrivers], [enabled_bus_drivers], [sh7750r],       [ENABLE_BUS_SH7750R])
CHECK_DRIVER([$busdrivers], [enabled_bus_drivers], [sh7751r],       [ENABLE_BUS_SH7751R])
CHECK_DRIVER([$busdrivers], [enabled_bus_drivers], [sharc_21065L],  [ENABLE_BUS_SHARC_21065L])
CHECK_DRIVER([$busdrivers], [enabled_bus_drivers], [slsup3],        [ENABLE_BUS_SLSUP3])
CHECK_DRIVER([$busdrivers], [enabled_bus_drivers], [tx4925],        [ENABLE_BUS_TX4925])
CHECK_DRIVER([$busdrivers], [enabled_bus_drivers], [zefant_xs3],    [ENABLE_BUS_ZEFANT_XS3])

# Enable cable drivers
AC_DEFUN([DEF_ENABLE_CABLEDRIVERS], [\
arcom byteblaster dlc5 ea253 ei012 ft2232 igloo jlink keithkoep lattice mpcbdm triton usbblaster wiggler xpc])
AC_DEFUN([DEF_DISABLE_CABLEDRIVERS], [ep9307 jim])
AC_ARG_ENABLE(cable,
[AS_HELP_STRING([--enable-cable], [Enable default set or specific cable drivers.])]
[AS_HELP_STRING([],               ['default' enables:])]
[AS_HELP_STRING([],DEF_ENABLE_CABLEDRIVERS)]
[AS_HELP_STRING([],               [Disabled by default are:])]
[AS_HELP_STRING([],DEF_DISABLE_CABLEDRIVERS)]
,
[AS_CASE([${enableval}],
    [yes],  [cabledrivers=default],
    [no],   [cabledrivers=none],
    [none], [cabledrivers=none],
            [cabledrivers=`echo ${enableval} | $SED -e 's/,/ /g'`])],
[cabledrivers=default])
# expand 'default' to default enabled cabledrivers
cabledrivers=`echo ${cabledrivers} | $SED -e "s/default/DEF_ENABLE_CABLEDRIVERS/"`
# automatically disable cable drivers when a required feature is not available
AS_IF([test "x$HAVELIBFTDI" != "xyes" -a "x$HAVELIBFTD2XX" != "xyes"], [
    cabledrivers=`echo ${cabledrivers} | $SED -e "s/ft2232//" -e "s/usbblaster//"`
])
AS_IF([test "x$HAVELIBUSB" != "xyes"], [
    cabledrivers=`echo ${cabledrivers} | $SED -e "s/jlink//" -e "s/xpc//"`
])
#
enabled_cable_drivers=''
CHECK_DRIVER([$cabledrivers], [enabled_cable_drivers], [arcom],       [ENABLE_CABLE_ARCOM])
CHECK_DRIVER([$cabledrivers], [enabled_cable_drivers], [byteblaster], [ENABLE_CABLE_BYTEBLASTER])
CHECK_DRIVER([$cabledrivers], [enabled_cable_drivers], [dlc5],        [ENABLE_CABLE_DLC5])
CHECK_DRIVER([$cabledrivers], [enabled_cable_drivers], [ea253],       [ENABLE_CABLE_EA253])
CHECK_DRIVER([$cabledrivers], [enabled_cable_drivers], [ei012],       [ENABLE_CABLE_EI012])
CHECK_DRIVER([$cabledrivers], [enabled_cable_drivers], [ep9307],      [ENABLE_CABLE_EP9307])
CHECK_DRIVER([$cabledrivers], [enabled_cable_drivers], [ft2232],      [ENABLE_CABLE_FT2232])
CHECK_DRIVER([$cabledrivers], [enabled_cable_drivers], [igloo],       [ENABLE_CABLE_IGLOO])
CHECK_DRIVER([$cabledrivers], [enabled_cable_drivers], [jim],         [ENABLE_JIM])
CHECK_DRIVER([$cabledrivers], [enabled_cable_drivers], [jlink],       [ENABLE_CABLE_JLINK])
CHECK_DRIVER([$cabledrivers], [enabled_cable_drivers], [keithkoep],   [ENABLE_CABLE_KEITHKOEP])
CHECK_DRIVER([$cabledrivers], [enabled_cable_drivers], [lattice],     [ENABLE_CABLE_LATTICE])
CHECK_DRIVER([$cabledrivers], [enabled_cable_drivers], [mpcbdm],      [ENABLE_CABLE_MPCBDM])
CHECK_DRIVER([$cabledrivers], [enabled_cable_drivers], [triton],      [ENABLE_CABLE_TRITON])
CHECK_DRIVER([$cabledrivers], [enabled_cable_drivers], [usbblaster],  [ENABLE_CABLE_USBBLASTER])
CHECK_DRIVER([$cabledrivers], [enabled_cable_drivers], [wiggler],     [ENABLE_CABLE_WIGGLER])
CHECK_DRIVER([$cabledrivers], [enabled_cable_drivers], [xpc],         [ENABLE_CABLE_XPC])


# Enable lowlevel drivers
AC_DEFUN([DEF_ENABLE_LOWLEVELDRIVERS], [\
direct ftdi ftd2xx ppdev ppi])
AC_ARG_ENABLE(lowlevel,
[AS_HELP_STRING([--enable-lowlevel], [Enable default set or specific lowlevel drivers:])]
[AS_HELP_STRING([],                  ['default' enables:])]
[AS_HELP_STRING([],DEF_ENABLE_LOWLEVELDRIVERS)]
,
[AS_CASE([${enableval}],
    [yes],  [lowleveldrivers=default],
    [no],   [lowleveldrivers=none],
    [none], [lowleveldrivers=none],
            [lowleveldrivers=`echo ${enableval} | $SED -e 's/,/ /g'`])],
[lowleveldrivers=default])
# expand 'default' to default enabled lowlevel drivers
lowleveldrivers=`echo ${lowleveldrivers} | $SED -e "s/default/DEF_ENABLE_LOWLEVELDRIVERS/"`
# automatically disable lowlevel drivers when a required feature is not available
AS_IF([test "x$HAVELIBFTDI" != "xyes"], [
    lowleveldrivers=`echo ${lowleveldrivers} | $SED -e "s/ftdi//"`
])
AS_IF([test "x$HAVELIBFTD2XX" != "xyes"], [
    lowleveldrivers=`echo ${lowleveldrivers} | $SED -e "s/ftd2xx//"`
])
AS_IF([test "x$HAVE_LINUX_PPDEV_H" != "xyes"], [
    lowleveldrivers=`echo ${lowleveldrivers} | $SED -e "s/ppdev//"`
])
AS_IF([test "x$HAVE_DEV_PPBUS_PPI_H" != "xyes"], [
    lowleveldrivers=`echo ${lowleveldrivers} | $SED -e "s/ppi//"`
])
AS_IF([test "x$HAVE_IOPERM" != "xyes" -a "x$HAVE_I386_SET_IOPERM" != "xyes"], [
    lowleveldrivers=`echo ${lowleveldrivers} | $SED -e "s/direct//"`
])
#
enabled_lowlevel_drivers=''
CHECK_DRIVER([$lowleveldrivers], [enabled_lowlevel_drivers], [direct], [ENABLE_LOWLEVEL_DIRECT])
CHECK_DRIVER([$lowleveldrivers], [enabled_lowlevel_drivers], [ftd2xx], [ENABLE_LOWLEVEL_FTD2XX])
CHECK_DRIVER([$lowleveldrivers], [enabled_lowlevel_drivers], [ftdi],   [ENABLE_LOWLEVEL_FTDI])
CHECK_DRIVER([$lowleveldrivers], [enabled_lowlevel_drivers], [ppdev],  [ENABLE_LOWLEVEL_PPDEV])
CHECK_DRIVER([$lowleveldrivers], [enabled_lowlevel_drivers], [ppi],    [ENABLE_LOWLEVEL_PPI])


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


dnl Is fmax() provided by libm?
AC_CHECK_LIB([m], [fmax], [
  AC_DEFINE(HAVE_FMAX, 1, [Define if -lm provides fmax()])
])


AC_OUTPUT

dnl
dnl Configuration summary
dnl
AS_IF([test ${HAVELIBFTDI:-no} != no], [
   FLAG_HAVELIBFTDI=yes
], [
   FLAG_HAVELIBFTDI=no
])
AS_IF([test ${HAVELIBUSB:-no} != no], [
   FLAG_HAVELIBUSB=yes
], [
   FLAG_HAVELIBUSB=no
])
AS_IF([test ${HAVELIBFTD2XX:-no} != no], [
   FLAG_HAVELIBFTD2XX=yes
], [
   FLAG_HAVELIBFTD2XX=no
])
AS_IF([test ${svf:-false} != false], [
   FLAG_SVF=yes
], [
   FLAG_SVF=no
])
AS_IF([test ${bsdl:-false} != false], [
   FLAG_BSDL=yes
], [
   FLAG_BSDL=no
])
AC_MSG_NOTICE([

jtag is now configured for

  Detected libusb      : $FLAG_HAVELIBUSB
  Detected libftdi     : $FLAG_HAVELIBFTDI
  Detected libftd2xx   : $FLAG_HAVELIBFTD2XX
  Build SVF player     : $FLAG_SVF
  Build BSDL subsystem : $FLAG_BSDL
  Bus drivers          : $enabled_bus_drivers
  Cable drivers        : $enabled_cable_drivers
  Lowlevel drivers     : $enabled_lowlevel_drivers
])
