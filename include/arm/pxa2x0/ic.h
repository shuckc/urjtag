/*
 * $Id$
 *
 * XScale PXA250/PXA210 Interrupt Control Registers
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

#ifndef	PXA2X0_IC_H
#define	PXA2X0_IC_H

#include <common.h>

#if LANGUAGE == C
#include <stdint.h>
#endif

/* Interrupt Control Registers */

#define	IC_BASE		0x40D00000

#if LANGUAGE == C
typedef volatile struct IC_registers {
	uint32_t icip;
	uint32_t icmr;
	uint32_t iclr;
	uint32_t icfp;
	uint32_t icpr;
	uint32_t iccr;
} IC_registers_t;

#ifdef PXA2X0_UNMAPPED
#define	IC_pointer	((IC_registers_t*) IC_BASE)
#endif

#define	ICIP		IC_pointer->icip
#define	ICMR		IC_pointer->icmr
#define	ICLR		IC_pointer->iclr
#define	ICFP		IC_pointer->icfp
#define	ICPR		IC_pointer->icpr
#define	ICCR		IC_pointer->iccr
#endif /* LANGUAGE == C */

#define	ICIP_OFFSET	0x00
#define	ICMR_OFFSET	0x04
#define	ICLR_OFFSET	0x08
#define	ICFP_OFFSET	0x0C
#define	ICPR_OFFSET	0x10
#define	ICCR_OFFSET	0x14

/* IRQ bits */

#define	IC_IRQ31	bit(31)
#define	IC_IRQ30	bit(30)
#define	IC_IRQ29	bit(29)
#define	IC_IRQ28	bit(28)
#define	IC_IRQ27	bit(27)
#define	IC_IRQ26	bit(26)
#define	IC_IRQ25	bit(25)
#define	IC_IRQ24	bit(24)
#define	IC_IRQ23	bit(23)
#define	IC_IRQ22	bit(22)
#define	IC_IRQ21	bit(21)
#define	IC_IRQ20	bit(20)
#define	IC_IRQ19	bit(19)
#define	IC_IRQ18	bit(18)
#define	IC_IRQ17	bit(17)
#define	IC_IRQ14	bit(14)
#define	IC_IRQ13	bit(13)
#define	IC_IRQ12	bit(12)
#define	IC_IRQ11	bit(11)
#define	IC_IRQ10	bit(10)
#define	IC_IRQ9		bit(9)
#define	IC_IRQ8		bit(8)

/* ICCR bits - see Table 4-33 in [1] */

#define	ICCR_DIM	bit(0)

#endif /* PXA2X0_IC_H */
