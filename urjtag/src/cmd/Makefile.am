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

noinst_LTLIBRARIES = libcmd.la

all_cmd_files = \
	cmd_frequency.c \
	cmd_cable.c \
	cmd_reset.c \
	cmd_discovery.c \
	cmd_idcode.c \
	cmd_detect.c \
	cmd_detectflash.c \
	cmd_help.c \
	cmd_quit.c \
	cmd_scan.c \
	cmd_signal.c \
	cmd_salias.c \
	cmd_bit.c \
	cmd_register.c \
	cmd_initbus.c \
	cmd_print.c \
	cmd_part.c \
	cmd_bus.c \
	cmd_instruction.c \
	cmd_shift.c \
	cmd_dr.c \
	cmd_get.c \
	cmd_test.c \
	cmd_debug.c \
	cmd_shell.c \
	cmd_set.c \
	cmd_endian.c \
	cmd_peekpoke.c \
	cmd_pod.c \
	cmd_readmem.c \
	cmd_writemem.c \
	cmd_flashmem.c \
	cmd_eraseflash.c \
	cmd_include.c \
	cmd_addpart.c \
	cmd_cmd.c \
	cmd_usleep.c \
	cmd_bfin.c \
	cmd_pld.c

if ENABLE_SVF
all_cmd_files += cmd_svf.c
endif

if ENABLE_BSDL
all_cmd_files += cmd_bsdl.c
endif

libcmd_la_SOURCES = \
	cmd.h \
	cmd_list.h \
	$(all_cmd_files)

$(libcmd_la_OBJECTS): generated_cmd_list.h
generated_cmd_list.h: generated_cmd_list.h.stamp ; @true
generated_cmd_list.h.stamp: $(all_cmd_files)
	$(AM_V_GEN)set -e; \
	cmds=`$(SED) -n '/^const urj_cmd_t urj_cmd_/{s:.*urj_cmd_::;s: =.*::;p}' $^`; \
	for c in $$cmds ; do \
		printf '#ifndef URJ_CMD_SKIP_%s\n_URJ_CMD(%s)\n#endif\n' $$c $$c; \
	done > generated_cmd_list.h.tmp; \
	if cmp -s generated_cmd_list.h generated_cmd_list.h.tmp; then \
		echo generated_cmd_list.h is unchanged; \
		rm -f generated_cmd_list.h.tmp; \
	else \
		echo updating generated_cmd_list.h; \
		mv generated_cmd_list.h.tmp generated_cmd_list.h; \
	fi; \
	touch $@


EXTRA_DIST = generated_cmd_list.h generated_cmd_list.h.stamp
MAINTAINERCLEANFILES = generated_cmd_list.h generated_cmd_list.h.stamp

AM_CFLAGS = $(WARNINGCFLAGS)
