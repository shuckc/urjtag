#
# $Id$
#
# Copyright (C) 2002 ETC s.r.o.
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
# Written by Marcel Telka <marcel@telka.sk>, 2002.
#

openwinceincdir = $(includedir)/openwince

nobase_openwinceinc_HEADERS = \
	common.h \
	openwince.h \
	cexcept.h \
	arm/arm.h \
	arm/sa11x0/gpclk.h \
	arm/sa11x0/gpio.h \
	arm/sa11x0/hssp.h \
	arm/sa11x0/ic.h \
	arm/sa11x0/lcd.h \
	arm/sa11x0/mc.h \
	arm/sa11x0/mcp.h \
	arm/sa11x0/ost.h \
	arm/sa11x0/pm.h \
	arm/sa11x0/ppc.h \
	arm/sa11x0/rc.h \
	arm/sa11x0/rtc.h \
	arm/sa11x0/ssp.h \
	arm/sa11x0/uart.h \
	arm/sa11x0/udc.h \
	arm/pxa2x0/ac97.h \
	arm/pxa2x0/cm.h \
	arm/pxa2x0/dma.h \
	arm/pxa2x0/gpio.h \
	arm/pxa2x0/i2c.h \
	arm/pxa2x0/i2s.h \
	arm/pxa2x0/ic.h \
	arm/pxa2x0/icp.h \
	arm/pxa2x0/lcd.h \
	arm/pxa2x0/mc.h \
	arm/pxa2x0/mmc.h \
	arm/pxa2x0/ost.h \
	arm/pxa2x0/pmrc.h \
	arm/pxa2x0/pwm.h \
	arm/pxa2x0/rtc.h \
	arm/pxa2x0/ssp.h \
	arm/pxa2x0/uart.h \
	arm/pxa2x0/udc.h \
	brux/bus.h \
	brux/cfi.h \
	brux/cmd.h \
	brux/flash.h \
	device/codec/ac97.h \
	device/codec/ucb1400.h \
	device/flash/28fxxxj.h \
	device/flash/28fxxxk.h \
	device/flash/cfi.h \
	device/flash/intel.h \
	device/other/hd64461/afe.h \
	device/other/hd64461/gpio.h \
	device/other/hd64461/intc.h \
	device/other/hd64461/lcdc.h \
	device/other/hd64461/pcc.h \
	device/other/hd64461/stbsys.h \
	device/other/hd64461/timer.h \
	h8/h83048/adc.h \
	h8/h83048/dac.h \
	h8/h83048/dmac.h \
	h8/h83048/flash.h \
	h8/h83048/ic.h \
	h8/h83048/itu.h \
	h8/h83048/other.h \
	h8/h83048/ports.h \
	h8/h83048/rc.h \
	h8/h83048/sci.h \
	h8/h83048/tpc.h \
	h8/h83048/wdt.h \
	h8/h8s2357/adc.h \
	h8/h8s2357/bus.h \
	h8/h8s2357/dac.h \
	h8/h8s2357/dmac.h \
	h8/h8s2357/dtc.h \
	h8/h8s2357/flash.h \
	h8/h8s2357/ic.h \
	h8/h8s2357/mcu.h \
	h8/h8s2357/ports.h \
	h8/h8s2357/ppg.h \
	h8/h8s2357/sci.h \
	h8/h8s2357/timer.h \
	h8/h8s2357/tpu.h \
	h8/h8s2357/wdt.h \
	sh/sh7709s/intc.h \
	sh/sh7750/ccn.h \
	std/mic.h

noinst_HEADERS = \
	stdint/stdint-hwbench.h \
	stdint/stdint-win32.h

if GENERATE_STDINT_H
nodist_include_HEADERS = stdint.h

stdint.h: owce-stdint.h
	cp $< $@
endif

if HAVE_HWBENCH
nodist_include_HEADERS = stdint.h

stdint.h: stdint/stdint-hwbench.h
	cp $< $@
endif

CLEANFILES = stdint.h
