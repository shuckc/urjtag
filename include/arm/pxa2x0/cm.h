/*
 * $Id$
 *
 * XScale PXA26x/PXA250/PXA210 Clocks Manager Registers
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
 * [2] Intel Corporation, "Intel PXA26x Processor Family Developer's Manual",
 *     October 2002, Order Number: 278638-001
 *
 */

#ifndef	PXA2X0_CM_H
#define	PXA2X0_CM_H

#include <common.h>

#if LANGUAGE == C
#include <stdint.h>
#endif

#if defined(PXA2X0_NOPXA250) && !defined(PXA2X0_NOPXA26X)
#define	PXA2X0_NOPXA26X
#endif

/* Clocks Manager Registers */

#define	CM_BASE		0x41300000

#if LANGUAGE == C
typedef volatile struct CM_registers {
	uint32_t cccr;
	uint32_t cken;
	uint32_t oscc;
} CM_registers_t;

#ifdef PXA2X0_UNMAPPED
#define	CM_pointer	((CM_registers_t*) CM_BASE)
#endif

#define	CCCR		CM_pointer->cccr
#define	CKEN		CM_pointer->cken
#define	OSCC		CM_pointer->oscc
#endif /* LANGUAGE == C */

#define	CCCR_OFFSET	0x00
#define	CKEN_OFFSET	0x04
#define	OSCC_OFFSET	0x08

/* CCCR bits - see Table 3-20 in [1], Table 3-20 in [2] */

#define	CCCR_N_MASK	bits(9,7)
#define	CCCR_N(x)	bits_val(9,7,x)
#define	CCCR_M_MASK	bits(6,5)
#define	CCCR_M(x)	bits_val(6,5,x)
#define	CCCR_L_MASK	bits(4,0)
#define	CCCR_L(x)	bits_val(4,0,x)

#define	CCCR_N_1_0	CCCR_N(0x2)
#define	CCCR_N_1_5	CCCR_N(0x3)
#define	CCCR_N_2_0	CCCR_N(0x4)
#define	CCCR_N_3_0	CCCR_N(0x6)

#define	CCCR_M_1	CCCR_M(0x1)
#define	CCCR_M_2	CCCR_M(0x2)

#define	CCCR_L_27	CCCR_L(0x01)
#define	CCCR_L_32	CCCR_L(0x02)
#define	CCCR_L_36	CCCR_L(0x03)
#define	CCCR_L_40	CCCR_L(0x04)
#define	CCCR_L_45	CCCR_L(0x05)

/* CKEN bits - see Table 3-21 in [1], Table 3-21 in [2] */

#define	CKEN_CKEN16	bit(16)
#define	CKEN_CKEN14	bit(14)
#define	CKEN_CKEN13	bit(13)
#define	CKEN_CKEN12	bit(12)
#define	CKEN_CKEN11	bit(11)
#if !defined(PXA2X0_NOPXA26X)
#define	CKEN_CKEN10	bit(10)
#define	CKEN_CKEN9	bit(9)
#endif /* PXA26x only */
#define	CKEN_CKEN8	bit(8)
#define	CKEN_CKEN7	bit(7)
#define	CKEN_CKEN6	bit(6)
#define	CKEN_CKEN5	bit(5)
#if !defined(PXA2X0_NOPXA26X)
#define	CKEN_CKEN4	bit(4)
#endif /* PXA26x only */
#define	CKEN_CKEN3	bit(3)
#define	CKEN_CKEN2	bit(2)
#define	CKEN_CKEN1	bit(1)
#define	CKEN_CKEN0	bit(0)

/* OSCC bits - see Table 3-22 in [1], Table 3-22 in [2] */

#define	OSCC_OON	bit(1)
#define	OSCC_OOK	bit(0)

#endif /* PXA2X0_CM_H */
