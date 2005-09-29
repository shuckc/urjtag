/*
 * $Id$
 *
 * StrongARM SA-1110 Real-Time Clock Registers
 * Copyright (C) 2002 ETC s.r.o.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the ETC s.r.o. nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Written by Marcel Telka <marcel@telka.sk>, 2002.
 *
 * Documentation:
 * [1] Intel Corporation, "Intel StrongARM SA-1110 Microprocessor
 *     Developer's Manual", October 2001, Order Number: 278240-004
 *
 */

#ifndef	SA11X0_RTC_H
#define	SA11X0_RTC_H

#include <openwince.h>

#if LANGUAGE == C
#include <stdint.h>
#endif

/* Real-Time Clock Registers */

#define	RTC_BASE	0x90010000

#if LANGUAGE == C
typedef volatile struct RTC_registers {
	uint32_t rtar;
	uint32_t rcnr;
	uint32_t rttr;
	uint32_t __reserved;
	uint32_t rtsr;
} RTC_registers_t;

#ifdef SA11X0_UNMAPPED
#define	RTC_pointer	((RTC_registers_t*) RTC_BASE)
#endif

#define	RTAR		RTC_pointer->rtar
#define	RCNR		RTC_pointer->rcnr
#define	RTTR		RTC_pointer->rttr
#define	RTSR		RTC_pointer->rtsr
#endif /* LANGUAGE == C */

#define	RTAR_OFFSET	0x00
#define	RCNR_OFFSET	0x04
#define	RTTR_OFFSET	0x08
#define	RTSR_OFFSET	0x10

/* RTSR bits */

#define	RTSR_HZE	bit(3)
#define	RTSR_ALE	bit(2)
#define	RTSR_HZ		bit(1)
#define	RTSR_AL		bit(0)

#endif /* SA11X0_RTC_H */
