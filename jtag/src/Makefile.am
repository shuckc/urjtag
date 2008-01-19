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

include $(top_srcdir)/Makefile.rules

SUBDIRS = \
	flash \
	lib \
	tap \
	part \
	bus \
	cmd

if ENABLE_SVF
SUBDIRS += svf
endif

if ENABLE_BSDL
SUBDIRS += bsdl
endif

bin_PROGRAMS = \
	jtag \
	bsdl2jtag

jtag_SOURCES = \
	jtag.c \
	detect.c \
	discovery.c \
	readmem.c \
	writemem.c \
	flash.c

bsdl2jtag_SOURCES = \
	bsdl2jtag.c

jtag_DEPENDENCIES = \
	flash/libflash.a \
	lib/libjtaglib.a \
	tap/libtap.a \
	part/libpart.a \
	bus/libbus.a \
	cmd/libcmd.a

if ENABLE_SVF
jtag_DEPENDENCIES += svf/libsvf.a
endif

if ENABLE_BSDL
jtag_DEPENDENCIES +=  bsdl/libbsdl.a
endif

jtag_LDADD = \
	-Ltap -ltap \
	-Lpart -lpart \
	-Llib -ljtaglib \
	-Lflash -lflash \
	-Lcmd -lcmd \
	-Lbus -lbus \
	-lm \
	@FTD2XXLIB@ \
	@LIBINTL@

if ENABLE_SVF
jtag_LDADD += -Lsvf -lsvf
endif

if ENABLE_BSDL
jtag_LDADD += -Lbsdl -lbsdl
endif

if ENABLE_JIM
SUBDIRS += jim
jtag_LDADD += -Ljim -ljim
jtag_DEPENDENCIES += jim/libjim.a
endif

localedir = $(datadir)/locale
INCLUDES = -DLOCALEDIR=\"$(localedir)\"
