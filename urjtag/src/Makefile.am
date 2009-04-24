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

if ENABLE_JIM
SUBDIRS += jim
endif

bin_PROGRAMS = \
	jtag

dist_bin_SCRIPTS = \
	bsdl2jtag

jtag_SOURCES = \
	jtag.c

lib_LTLIBRARIES = liburjtag.la
liburjtag_la_LDFLAGS = -version-info 0:0:0

liburjtag_la_SOURCES =

liburjtag_la_LIBADD = \
	tap/libtap.la \
	part/libpart.la \
	lib/libjtaglib.la \
	flash/libflash.la \
	bus/libbus.la \
	cmd/libcmd.la \
	-lm \
	@FTD2XXLIB@ \
	@LIBINTL@

if ENABLE_SVF
liburjtag_la_LIBADD += svf/libsvf.la
endif

if ENABLE_BSDL
liburjtag_la_LIBADD +=  bsdl/libbsdl.la
endif

if ENABLE_JIM
liburjtag_la_LIBADD += jim/libjim.la
endif

if DMALLOC
liburjtag_la_LIBADD += -ldmalloc
endif

jtag_LDADD = \
	liburjtag.la \
	@LIBINTL@

localedir = $(datadir)/locale
INCLUDES = -DLOCALEDIR=\"$(localedir)\"
