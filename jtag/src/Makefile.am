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
	tap \
	part

bin_PROGRAMS = jtag

jtag_SOURCES = jtag.c detect.c readmem.c detect.h bus.h pxa250.c sa1110.c cfi.c flash.c help.c

jtag_DEPENDENCIES = tap/libtap.a part/libpart.a

jtag_LDADD = -lreadline -lncurses -Ltap -ltap -Lpart -lpart

INCLUDES = -I$(top_srcdir)/include $(OPENWINCE_INC) -DJTAG_DATA_DIR=\"$(pkgdatadir)\"
