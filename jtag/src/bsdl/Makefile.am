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

noinst_LIBRARIES = libbsdl.a

libbsdl_a_SOURCES = \
	bsdl.c \
	bsdl.h \
	bsdl_bison.y \
	bsdl_flex.l \
	bsdl_sem.c

AM_LFLAGS = -i

MAINTAINERCLEANFILES = \
	bsdl_bison.c \
	bsdl_bison.h \
	bsdl_flex.c

INCLUDES = -DJTAG_DATA_DIR=\"$(pkgdatadir)\"
