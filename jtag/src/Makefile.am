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

SUBDIRS = \
	lib \
	tap \
	part \
	bus

bin_PROGRAMS = jtag

jtag_SOURCES = \
	jtag.c \
	jtag.h \
	detect.c \
	discovery.c \
	readmem.c \
	cfi.c \
	flash.h \
	flash.c \
	flash-intel.c \
	flash-amd.c \
	help.c

jtag_DEPENDENCIES = lib/libjtaglib.a tap/libtap.a part/libpart.a bus/libbus.a

jtag_LDADD = -Ltap -ltap -Lpart -lpart -Llib -ljtaglib -Lbus -lbus @LIBINTL@

localedir = $(datadir)/locale
INCLUDES = -I$(top_srcdir)/include $(OPENWINCE_INC) -DLOCALEDIR=\"$(localedir)\" -DJTAG_DATA_DIR=\"$(pkgdatadir)\"
