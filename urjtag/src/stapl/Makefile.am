#
# $Id$
#
# Copyright (C) 2011, Michael Vacek
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
# Written by Michael Vacek <michael.vacek@gmail.com>, 2011.
#

include $(top_srcdir)/Makefile.rules

AM_YFLAGS = -d

OBJEXT = lo
noinst_LTLIBRARIES = \
    libstapl.la

libstapl_la_SOURCES = \
    jamexec.c \
    jamnote.c \
    jamcrc.c \
    jamsym.c \
    jamstack.c \
    jamheap.c \
    jamarray.c \
    jamcomp.c \
    jamjtag.c \
    jamexp.c \
    jamexec.h \
    jamnote.h \
    jamcrc.h \
    jamsym.h \
    jamstack.h \
    jamheap.h \
    jamarray.h \
    jamcomp.h \
    jamjtag.h \
    jamutil.h \
    jamexp.h \
    stapl.h \
    stapl.c

AM_CFLAGS = $(WARNINGCFLAGS)

