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
	bsdl_msg.h \
	bsdl_parser.h \
	bsdl_sysdep.h \
	bsdl_types.h \
	vhdl_bison.h \
	vhdl_parser.h

AM_CFLAGS = $(WARNINGCFLAGS)

# additional dependencies
# - *_flex files must be processed after their *_bison counterparts
#   to ensure that *_bison.h is present
# - we use variables to workaround automake rule/dependency limitations
VHDL_BISON_OBJS = vhdl_flex.lo
BSDL_BISON_OBJS = bsdl_flex.lo bsdl_sem.lo
$(VHDL_BISON_OBJS): vhdl_bison.h
$(BSDL_BISON_OBJS): bsdl_bison.h
vhdl_bison.h: vhdl_bison.c ; @true
bsdl_bison.h: bsdl_bison.c ; @true

AM_LFLAGS = -i

MAINTAINERCLEANFILES = \
	vhdl_bison.c \
	vhdl_bison.h \
	bsdl_bison.c \
	bsdl_bison.h \
	vhdl_flex.c
