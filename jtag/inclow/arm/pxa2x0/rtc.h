/*
 * $Id$
 *
 * XScale PXA26x/PXA255/PXA250/PXA210 RTC Registers
 * Copyright (C) 2002, 2003 ETC s.r.o.
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
 * Written by Marcel Telka <marcel@telka.sk>, 2002, 2003.
 *
 * Documentation:
 * [1] Intel Corporation, "Intel PXA250 and PXA210 Application Processors
 *     Developer's Manual", February 2002, Order Number: 278522-001
 * [2] Intel Corporation, "Intel PXA26x Processor Family Developer's Manual",
 *     March 2003, Order Number: 278638-002
 * [3] Intel Corporation, "Intel PXA255 Processor Developer's Manual"
 *     March 2003, Order Number: 278693-001
 *
 */

#ifndef	PXA2X0_RTC_H
#define	PXA2X0_RTC_H

#include <openwince.h>

#if LANGUAGE == C
#include <stdint.h>
#endif

#if defined(PXA2X0_NOPXA250) && !defined(PXA2X0_NOPXA255)
#define PXA2X0_NOPXA255
#endif

#if defined(PXA2X0_NOPXA255) && !defined(PXA2X0_NOPXA260)
#define PXA2X0_NOPXA260
#endif

/* RTC Registers */

#define	RTC_BASE	0x40900000

#if LANGUAGE == C
typedef volatile struct RTC_registers {
	uint32_t rcnr;
	uint32_t rtar;
	uint32_t rtsr;
	uint32_t rttr;
} RTC_registers_t;

#ifdef PXA2X0_UNMAPPED
#define	RTC_pointer	((RTC_registers_t*) RTC_BASE)
#endif

#define	RCNR			RTC_pointer->rcnr
#define	RTAR			RTC_pointer->rtar
#define	RTSR			RTC_pointer->rtsr
#define	RTTR			RTC_pointer->rttr
#endif /* LANGUAGE == C */

#define	RCNR_OFFSET		0x00
#define	RTAR_OFFSET		0x04
#define	RTSR_OFFSET		0x08
#define	RTTR_OFFSET		0x0C

/* RTSR bits - see Table 4-42 in [1], Table 4-42 in [2], Table 4-40 in [3] */

#define	RTSR_HZE		bit(3)
#define	RTSR_ALE		bit(2)
#define	RTSR_HZ			bit(1)
#define	RTSR_AL			bit(0)

/* RTTR bits - see Table 4-39 in [1], Table 4-39 in [2], Table 4-37 in [3] */

#define	RTTR_LCK		bit(31)
#define	RTTR_DEL_MASK		bits(25,16)
#define	RTTR_DEL(x)		bits_val(25,16,x)
#define	get_RTTR_DEL(x)		bits_get(25,16,x)
#define	RTTR_CK_DIV_MASK	bits(15,0)
#define	RTTR_CK_DIV(x)		bits_val(15,0,x)
#define	get_RTTR_CK_DIV(x)	bits_get(15,0,x)

#endif /* PXA2X0_RTC_H */
