/*
 * $Id$
 *
 * XScale PXA250/PXA210 SSP Registers
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

#ifndef	PXA2X0_SSP_H
#define	PXA2X0_SSP_H

#include <common.h>

#if LANGUAGE == C
#include <stdint.h>
#endif

/* SSP Registers */

#define	SSP_BASE	0x41000000

#if LANGUAGE == C
/* see Table 8-7 in [1] */
typedef volatile struct SSP_registers {
	uint32_t sscr0;
	uint32_t sscr1;
	uint32_t sssr;
	uint32_t __reserved;
	uint32_t ssdr;
} SSP_registers_t;

#ifdef PXA2X0_UNMAPPED
#define	SSP_pointer	((SSP_registers_t*) SSP_BASE)
#endif

#define	SSCR0		SSP_pointer->sscr0
#define	SSCR1		SSP_pointer->sscr1
#define	SSSR		SSP_pointer->sssr
#define	SSDR		SSP_pointer->ssdr
#endif /* LANGUAGE == C */

#define	SSCR0_OFFSET	0x00
#define	SSCR1_OFFSET	0x04
#define	SSSR_OFFSET	0x08
#define	SSDR_OFFSET	0x10

/* SSCR0 bits - see Table 8-2 in [1] */

#define	SSCR0_SCR_MASK	bits(15,8)
#define	SSCR0_SCR(x)	bits_val(15,8,x)
#define	SSCR0_SSE	bit(7)
#define	SSCR0_ECS	bit(6)
#define	SSCR0_FRF_MASK	bits(5,4)
#define	SSCR0_FRF(x)	bits_val(5,4,x)
#define	SSCR0_DSS_MASK	bits(3,0)
#define	SSCR0_DSS(x)	bits_val(3,0,x)

/* SSCR1 bits - see Table 8-3 in [1] */

#define	SSCR1_RFT_MASK	bits(13,10)
#define	SSCR1_RFT(x)	bits_val(13,10,x)
#define	SSCR1_TFT_MASK	bits(9,6)
#define	SSCR1_TFT(x)	bits_val(9,6,x)
#define	SSCR1_MWDS	bit(5)
#define	SSCR1_SPH	bit(4)
#define	SSCR1_SPO	bit(3)
#define	SSCR1_LBM	bit(2)
#define	SSCR1_TIE	bit(1)
#define	SSCR1_RIE	bit(0)

/* SSSR bits - see Table 8-6 in [1] */

#define	SSSR_RFL_MASK	bits(15,12)
#define	SSSR_RFL(x)	bits_val(15,12,x)
#define	SSSR_TFL_MASK	bits(11,8)
#define	SSSR_TFL(x)	bits_val(11,8,x)
#define	SSSR_ROR	bit(7)
#define	SSSR_RFS	bit(6)
#define	SSSR_TFS	bit(5)
#define	SSSR_BSY	bit(4)
#define	SSSR_RNE	bit(3)
#define	SSSR_TNF	bit(2)

#endif /* PXA2X0_SSP_H */
