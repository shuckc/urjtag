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

noinst_LIBRARIES = libtap.a

libtap_a_SOURCES = \
	tap.c \
	register.c \
	state.c \
	chain.c \
	detect.c \
	discovery.c \
	parport.c \
	usbconn.c \
	cable.c \
	cable/generic.h \
	cable/generic.c \
	cable/generic_usbconn.h \
	cable/generic_usbconn.c \
	cable/generic_parport.h \
	cable/generic_parport.c

if ENABLE_CABLE_ARCOM
libtap_a_SOURCES += \
	cable/arcom.c
endif

if ENABLE_CABLE_BYTEBLASTER
libtap_a_SOURCES += \
	cable/byteblaster.c
endif

if ENABLE_CABLE_DLC5
libtap_a_SOURCES += \
	cable/dlc5.c
endif

if ENABLE_CABLE_EA253
libtap_a_SOURCES += \
	cable/ea253.c
endif

if ENABLE_CABLE_EI012
libtap_a_SOURCES += \
	cable/ei012.c
endif

if ENABLE_CABLE_KEITHKOEP
libtap_a_SOURCES += \
	cable/keithkoep.c
endif

if ENABLE_CABLE_LATTICE
libtap_a_SOURCES += \
	cable/lattice.c
endif

if ENABLE_CABLE_MPCBDM
libtap_a_SOURCES += \
	cable/mpcbdm.c
endif

if ENABLE_CABLE_TRITON
libtap_a_SOURCES += \
	cable/triton.c
endif

if ENABLE_CABLE_WIGGLER
libtap_a_SOURCES += \
	cable/wiggler.c \
	cable/wiggler2.c
endif

if ENABLE_CABLE_JLINK
libtap_a_SOURCES += \
	cable/jlink.c
endif

if ENABLE_CABLE_XPC
libtap_a_SOURCES += \
	cable/xpc.c
endif

if ENABLE_CABLE_USBBLASTER
libtap_a_SOURCES += \
	cable/usbblaster.c
endif

if ENABLE_CABLE_FT2232
libtap_a_SOURCES += \
	cable/ft2232.c
endif


if HAVE_LIBUSB
libtap_a_SOURCES += \
	usbconn/libusb.c
endif


if ENABLE_JIM
libtap_a_SOURCES += \
	cable/jim.c
endif

if ENABLE_PARPORT_FTDI
libtap_a_SOURCES += \
	parport/ftdi.c
endif

if ENABLE_PARPORT_FTD2XX
libtap_a_SOURCES += \
	parport/ftd2xx.c
endif

if ENABLE_PARPORT_DIRECT
libtap_a_SOURCES += \
	parport/direct.c
endif

if ENABLE_PARPORT_PPDEV
libtap_a_SOURCES += \
	parport/ppdev.c
endif

if ENABLE_PARPORT_PPI
libtap_a_SOURCES += \
	parport/ppi.c
endif

if ENABLE_CABLE_EP9307
libtap_a_SOURCES += \
	cable/vision_ep9307.c
endif
