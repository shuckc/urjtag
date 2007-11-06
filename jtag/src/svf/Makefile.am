#
# $Id: Makefile.am,v 1.16 2003/09/14 20:04:24 telka Exp $
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

noinst_LIBRARIES = libsvf.a

libsvf_a_SOURCES = \
	svf_bison.y \
	svf_flex.l \
	svf.c

AM_YFLAGS = -d

CLEANFILES = \
	svf_bison.c \
	svf_bison.h \
	svf_flex.c
