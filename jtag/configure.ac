#
# $Id$
#
# Copyright (C) 2002 ETC s.r.o.
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
# Written by Marcel Telka <marcel@telka.sk>, 2002.
#

AC_INIT(jtag,0.2.2)

AC_PREREQ(2.54)
AC_COPYRIGHT([Copyright (C) 2002, 2003 ETC s.r.o.])
AC_REVISION($Revision$)

AC_CONFIG_AUX_DIR(tools)

AM_INIT_AUTOMAKE([check-news dist-bzip2])

AC_CONFIG_FILES(
	Makefile
	data/Makefile
	include/Makefile
	src/Makefile
	src/tap/Makefile
	src/part/Makefile
)

AM_MAINTAINER_MODE

AC_GNU_SOURCE

AC_PROG_CC
AC_PROG_RANLIB

AC_SUBST(OPENWINCE_INC)
AC_ARG_WITH(include,
	AC_HELP_STRING([--with-include=PATH], [Path to the openwince includes]),
	openwince_inc="$withval",
	openwince_inc="/usr/include/openwince")
OPENWINCE_INC="-I$openwince_inc -I$openwince_inc/device"

AC_SEARCH_LIBS([ioperm], [ioperm])

CFLAGS="$CFLAGS -Wall"

AC_OUTPUT
