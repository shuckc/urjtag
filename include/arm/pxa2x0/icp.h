/*
 * $Id$
 *
 * XScale PXA250/PXA210 ICP Registers
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

#ifndef	PXA2X0_ICP_H
#define	PXA2X0_ICP_H

#include <common.h>

#if LANGUAGE == C
#include <stdint.h>
#endif

/* ICP Registers */

#define	ICP_BASE	0x40800000

#if LANGUAGE == C
typedef volatile struct ICP_registers {
	uint32_t iccr0;
	uint32_t iccr1;
	uint32_t iccr2;
	uint32_t icdr;
	uint32_t __reserved;
	uint32_t icsr0;
	uint32_t icsr1;
} ICP_registers;

#ifdef PXA2X0_UNMAPPED
#define	ICP_pointer	((ICP_registers*) ICP_BASE)
#endif

#define	ICCR0		ICP_pointer->iccr0
#define	ICCR1		ICP_pointer->iccr1
#define	ICCR2		ICP_pointer->iccr2
#define	ICDR		ICP_pointer->icdr
#define	ICSR0		ICP_pointer->icsr0
#define	ICSR1		ICP_pointer->icsr1
#endif /* LANGUAGE == C */

#define	ICCR0_OFFSET	0x00
#define	ICCR1_OFFSET	0x04
#define	ICCR2_OFFSET	0x08
#define	ICDR_OFFSET	0x0C
#define	ICSR0_OFFSET	0x14
#define	ICSR1_OFFSET	0x18

/* ICCR0 bits - see Table 11-2 in [1] */

#define	ICCR0_AME	bit(7)
#define	ICCR0_TIE	bit(6)
#define	ICCR0_RIE	bit(5)
#define	ICCR0_RXE	bit(4)
#define	ICCR0_TXE	bit(3)
#define	ICCR0_TUS	bit(2)
#define	ICCR0_LBM	bit(1)
#define	ICCR0_ITR	bit(0)

/* ICCR1 bits - see Table 11-3 in [1] */

#define	ICCR1_AMV_MASK	bits(7,0)
#define	ICCR1_AMV(x)	bits_val(7,0,x)

/* ICCR2 bits - see Table 11-4 in [1] */

#define	ICCR2_RXP	bit(3)
#define	ICCR2_TXP	bit(2)
#define	ICCR2_TRIG_MASK	bits(1,0)
#define	ICCR2_TRIG(x)	bits_val(1,0,x)

/* ICDR bits - see Table 11-5 in [1] */

#define	ICDR_DATA_MASK	bits(7,0)
#define	ICDR_DATA(x)	bits_val(7,0,x)

/* ICSR0 bits - see Table 11-6 in [1] */

#define	ICSR0_FRE	bit(5)
#define	ICSR0_RFS	bit(4)
#define	ICSR0_TFS	bit(3)
#define	ICSR0_RAB	bit(2)
#define	ICSR0_TUR	bit(1)
#define	ICSR0_EIF	bit(0)

/* ICSR1 bits - see Table 11-7 in [1] */

#define	ICSR1_ROR	bit(6)
#define	ICSR1_CRE	bit(5)
#define	ICSR1_EOF	bit(4)
#define	ICSR1_TNF	bit(3)
#define	ICSR1_RNE	bit(2)
#define	ICSR1_TBY	bit(1)
#define	ICSR1_RSY	bit(0)

#endif /* PXA2X0_ICP_H */
