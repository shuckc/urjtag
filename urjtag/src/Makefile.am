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
	bfin \
	bus \
	cmd \
	pld \
	global

if ENABLE_SVF
SUBDIRS += svf
endif

if ENABLE_BSDL
SUBDIRS += bsdl
endif

if ENABLE_JIM
SUBDIRS += jim
endif

if ENABLE_STAPL
SUBDIRS += stapl
endif

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = urjtag.pc

lib_LTLIBRARIES = liburjtag.la
liburjtag_la_LDFLAGS = -version-info 0:0:0

liburjtag_la_SOURCES =

liburjtag_la_LIBADD = \
	tap/libtap.la \
	part/libpart.la \
	lib/libjtaglib.la \
	flash/libflash.la \
	bfin/libbfin.la \
	bus/libbus.la \
	cmd/libcmd.la \
	pld/libpld.la \
	global/libglobal.la \
	-lm \
	$(LIBFTDI_LIBS) \
	@FTD2XXLIB@ \
	$(LIBUSB_LIBS) \
	@LIBINTL@

if ENABLE_SVF
liburjtag_la_LIBADD += svf/libsvf.la
liburjtag_la_LIBADD += svf/libsvf_flex.la
endif

if ENABLE_BSDL
liburjtag_la_LIBADD +=  bsdl/libbsdl.la
liburjtag_la_LIBADD +=  bsdl/libbsdl_flex.la
endif

if ENABLE_JIM
liburjtag_la_LIBADD += jim/libjim.la
endif

if ENABLE_STAPL
liburjtag_la_LIBADD += stapl/libstapl.la
endif

localedir = $(datadir)/locale
AM_CPPFLAGS = -DLOCALEDIR=\"$(localedir)\"

AM_CFLAGS = $(WARNINGCFLAGS)
