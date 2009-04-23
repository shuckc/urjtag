#
# $Id$
#
# Copyright (C) 2003 ETC s.r.o.
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
# Written by Marcel Telka <marcel@telka.sk>, 2003.
#

include $(top_srcdir)/Makefile.rules

noinst_LIBRARIES = libcmd.a

libcmd_a_SOURCES =
libcmd_a_SOURCES += cmd_frequency.c
libcmd_a_SOURCES += cmd_cable.c
libcmd_a_SOURCES += cmd_reset.c
libcmd_a_SOURCES += cmd_discovery.c
libcmd_a_SOURCES += cmd_idcode.c
libcmd_a_SOURCES += cmd_detect.c
libcmd_a_SOURCES += cmd_detectflash.c
libcmd_a_SOURCES += cmd_help.c
libcmd_a_SOURCES += cmd_quit.c
libcmd_a_SOURCES += cmd_scan.c
libcmd_a_SOURCES += cmd_signal.c
libcmd_a_SOURCES += cmd_salias.c
libcmd_a_SOURCES += cmd_bit.c
libcmd_a_SOURCES += cmd_register.c
libcmd_a_SOURCES += cmd_initbus.c
libcmd_a_SOURCES += cmd_print.c
libcmd_a_SOURCES += cmd_part.c
libcmd_a_SOURCES += cmd_bus.c
libcmd_a_SOURCES += cmd_instruction.c
libcmd_a_SOURCES += cmd_shift.c
libcmd_a_SOURCES += cmd_dr.c
libcmd_a_SOURCES += cmd_get.c
libcmd_a_SOURCES += cmd_test.c
libcmd_a_SOURCES += cmd_debug.c
libcmd_a_SOURCES += cmd_shell.c
libcmd_a_SOURCES += cmd_set.c
libcmd_a_SOURCES += cmd_endian.c
libcmd_a_SOURCES += cmd_parse.c
libcmd_a_SOURCES += cmd_peekpoke.c
libcmd_a_SOURCES += cmd_pod.c
libcmd_a_SOURCES += cmd_readmem.c
libcmd_a_SOURCES += cmd_writemem.c
libcmd_a_SOURCES += cmd_flashmem.c
libcmd_a_SOURCES += cmd_eraseflash.c
libcmd_a_SOURCES += cmd_include.c
libcmd_a_SOURCES += cmd_addpart.c
libcmd_a_SOURCES += cmd_cmd.c
libcmd_a_SOURCES += cmd_usleep.c
libcmd_a_SOURCES += cmd_jtag_data_dir.c

if ENABLE_SVF
libcmd_a_SOURCES += cmd_svf.c
endif

if ENABLE_BSDL
libcmd_a_SOURCES += cmd_bsdl.c
endif

AM_CPPFLAGS = -DJTAG_BIN_DIR=\"$(bindir)\" -DJTAG_DATA_DIR=\"$(pkgdatadir)\"
