/*
 * $Id$
 *
 * XScale PXA250/PXA210 OS Timer Registers
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
 * [1] Intel Corporation, "Intel PXA250 and PXA210 Application Processors
 *     Developer's Manual", February 2002, Order Number: 278522-001
 *
 */

#ifndef	PXA2X0_OST_H
#define	PXA2X0_OST_H

#include <common.h>

#if LANGUAGE == C
#include <stdint.h>
#endif

/* OS Timer Registers */

#define	OST_BASE	0x40A00000

#if LANGUAGE == C
typedef volatile struct OST_registers {
	uint32_t osmr[4];
	uint32_t oscr;
	uint32_t ossr;
	uint32_t ower;
	uint32_t oier;
} OST_registers_t;

#ifdef PXA2X0_UNMAPPED
#define	OST_pointer	((OST_registers_t*) OST_BASE)
#endif

#define	OSMR(i)		OST_pointer->osmr[i]
#define	OSCR		OST_pointer->oscr
#define	OSSR		OST_pointer->ossr
#define	OWER		OST_pointer->ower
#define	OIER		OST_pointer->oier
#endif /* LANGUAGE == C */

#define	OSMR0_OFFSET	0x00
#define	OSMR1_OFFSET	0x04
#define	OSMR2_OFFSET	0x08
#define	OSMR3_OFFSET	0x0C
#define	OSCR_OFFSET	0x10
#define	OSSR_OFFSET	0x14
#define	OWER_OFFSET	0x18
#define	OIER_OFFSET	0x1C

/* OSSR bits - see 4.4.2.5 in [1] */

#define	OSSR_M3		bit(3)
#define	OSSR_M2		bit(2)
#define	OSSR_M1		bit(1)
#define	OSSR_M0		bit(0)

/* OWER bits - see Table 4-46 in [1] */

#define	OWER_WME	bit(0)

/* OIER bits - see Table 4-45 in [1] */

#define	OIER_E3		bit(3)
#define	OIER_E2		bit(2)
#define	OIER_E1		bit(1)
#define	OIER_E0		bit(0)

#endif /* PXA2X0_OST_H */
