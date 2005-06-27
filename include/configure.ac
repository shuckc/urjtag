#
# $Id$
#
# Copyright (C) 2002 ETC s.r.o.
# Copyright (C) 2005 Elcom s.r.o.
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 3. Neither the name of the ETC s.r.o. nor the names of its contributors
#    may be used to endorse or promote products derived from this software
#    without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# Written by Marcel Telka <marcel@telka.sk>, 2002, 2005.
#

AC_INIT(include,0.3.3)

AC_PREREQ(2.53)
AC_REVISION($Revision$)

AC_CONFIG_AUX_DIR(tools)

AM_INIT_AUTOMAKE([check-news dist-bzip2])

AC_CONFIG_FILES(
	Makefile
)

AM_MAINTAINER_MODE

AC_ARG_WITH(hwbench,
	AC_HELP_STRING([--with-hwbench=PATH], [Absolute path to the hwbench installation]),
	with_hwbench="$withval")

AC_CACHE_CHECK([for hwbench installation], hwbench_path, [
	hwbench_path="no"
	for hwb_path in "$with_hwbench"; do
		if test -f "$hwb_path/IAR/EXE/ICCH83.EXE"; then
			hwbench_path="$hwb_path"
			export CC="$hwb_path/IAR/EXE/ICCH83.EXE"
			break
		fi
	done
])

generate_stdint_h="no"
AC_CHECK_HEADERS( stdint.h, [], [
		if test "$hwbench_path" = "no"; then
			AX_CREATE_STDINT_H( owce-stdint.h )
			generate_stdint_h="yes"
		fi
])
AM_CONDITIONAL( GENERATE_STDINT_H, test "$generate_stdint_h" = "yes" )

AC_OUTPUT
