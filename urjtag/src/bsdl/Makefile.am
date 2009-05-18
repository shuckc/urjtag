#
# $Id$
#
# Copyright (C) 2007, Arnim Laeuger
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

include $(top_srcdir)/Makefile.rules

OBJEXT = lo
noinst_LTLIBRARIES = \
	libbsdl.la \
	libbsdl_flex.la

libbsdl_la_SOURCES =  \
	vhdl_bison.y \
	bsdl_bison.y \
	bsdl.c       \
	bsdl_sem.c

libbsdl_flex_la_SOURCES = \
	vhdl_flex.l  \
	bsdl_flex.l

noinst_HEADERS = \
	bsdl_bison.h \
	bsdl_config.h \
	bsdl_msg.h \
	bsdl_parser.h \
	bsdl_sysdep.h \
	bsdl_types.h \
	vhdl_bison.h \
	vhdl_parser.h

AM_CFLAGS = $(WARNINGCFLAGS)

libbsdl_flex_la_CFLAGS = \
	$(AM_CFLAGS) \
	-Wno-error

# additional dependencies
# - all files depend on bsdl_config.h which dynamically generated
# - *_flex files must be processed after their *_bison counterparts
#   to ensure that *_bison.h is present
vhdl_flex.$(OBJEXT): vhdl_bison.$(OBJEXT) bsdl_config.h
vhdl_bison.$(OBJEXT): bsdl_config.h
bsdl_flex.$(OBJEXT): bsdl_bison.$(OBJEXT) bsdl_config.h
bsdl_bison.$(OBJEXT): bsdl_config.h
bsdl.$(OBJEXT) : bsdl_config.h
bsdl_sem.$(OBJEXT): bsdl_config.h

vhdl_bison.h: vhdl_bison.c
bsdl_bison.h: bsdl_bison.c

bsdl_config.h: ../../config.h
	grep 'DMALLOC' $< > $@

AM_LFLAGS = -i

MAINTAINERCLEANFILES = \
	vhdl_bison.c \
	vhdl_bison.h \
	bsdl_bison.c \
	bsdl_bison.h \
	vhdl_flex.c \
	bsdl_config.h
