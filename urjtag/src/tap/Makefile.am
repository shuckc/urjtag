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

noinst_LTLIBRARIES = libtap.la

libtap_la_SOURCES = \
	tap.c \
	register.c \
	state.c \
	chain.c \
	detect.c \
	discovery.c \
	idcode.c \
	parport.c \
	parport.h \
	parport_list.h \
	usbconn.c \
	usbconn.h \
	usbconn_list.h \
	cable.c \
	cable.h \
	cable_list.h \
	cable/generic.h \
	cable/generic.c \
	cable/generic_usbconn.h \
	cable/generic_usbconn.c \
	cable/generic_usbconn_list.h \
	cable/generic_parport.h \
	cable/generic_parport.c \
	cable/cmd_xfer.h \
	cable/cmd_xfer.c

if ENABLE_CABLE_ARCOM
libtap_la_SOURCES += \
	cable/arcom.c
endif

if ENABLE_CABLE_BYTEBLASTER
libtap_la_SOURCES += \
	cable/byteblaster.c
endif

if ENABLE_CABLE_DLC5
libtap_la_SOURCES += \
	cable/dlc5.c
endif

if ENABLE_CABLE_EA253
libtap_la_SOURCES += \
	cable/ea253.c
endif

if ENABLE_CABLE_EI012
libtap_la_SOURCES += \
	cable/ei012.c
endif

if ENABLE_CABLE_GPIO
libtap_la_SOURCES += \
	cable/gpio.c
endif

if ENABLE_CABLE_KEITHKOEP
libtap_la_SOURCES += \
	cable/keithkoep.c
endif

if ENABLE_CABLE_LATTICE
libtap_la_SOURCES += \
	cable/lattice.c
endif

if ENABLE_CABLE_MPCBDM
libtap_la_SOURCES += \
	cable/mpcbdm.c
endif

if ENABLE_CABLE_TRITON
libtap_la_SOURCES += \
	cable/triton.c
endif

if ENABLE_CABLE_WIGGLER
libtap_la_SOURCES += \
	cable/minimal.c \
	cable/wiggler.c \
	cable/wiggler2.c
endif

if ENABLE_CABLE_JLINK
libtap_la_SOURCES += \
	cable/jlink.c
endif

if ENABLE_CABLE_XPC
libtap_la_SOURCES += \
	cable/xpc.c
endif

if ENABLE_CABLE_USBBLASTER
libtap_la_SOURCES += \
	cable/usbblaster.c
endif

if ENABLE_CABLE_FT2232
libtap_la_SOURCES += \
	cable/ft2232.c
endif

if ENABLE_CABLE_TS7800
libtap_la_SOURCES += \
	cable/ts7800.c
endif

if ENABLE_CABLE_ICE100
libtap_la_SOURCES += \
	cable/ice100.c
endif

if ENABLE_CABLE_VSLLINK
libtap_la_SOURCES += \
	cable/vsllink.c
endif

if HAVE_LIBUSB
libtap_la_SOURCES += \
	usbconn/libusb.c \
	usbconn/libusb.h
endif


if ENABLE_JIM
libtap_la_SOURCES += \
	cable/jim.c
endif

if ENABLE_LOWLEVEL_FTDI
libtap_la_SOURCES += \
	usbconn/libftdi.c \
	usbconn/libftdx.h
endif

if ENABLE_LOWLEVEL_FTD2XX
libtap_la_SOURCES += \
	usbconn/libftd2xx.c
endif

if ENABLE_LOWLEVEL_DIRECT
libtap_la_SOURCES += \
	parport/direct.c
endif

if ENABLE_LOWLEVEL_PPDEV
libtap_la_SOURCES += \
	parport/ppdev.c
endif

if ENABLE_LOWLEVEL_PPI
libtap_la_SOURCES += \
	parport/ppi.c
endif

if ENABLE_CABLE_EP9307
libtap_la_SOURCES += \
	cable/vision_ep9307.c
endif

AM_CFLAGS = $(WARNINGCFLAGS)
