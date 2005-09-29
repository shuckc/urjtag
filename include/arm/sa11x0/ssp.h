/*
 * $Id$
 *
 * StrongARM SA-1110 SSP Registers
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

#ifndef	SA11X0_SSP_H
#define	SA11X0_SSP_H

#include <openwince.h>

#if LANGUAGE == C
#include <stdint.h>
#endif

/* SSP Registers (Serial Port 4) */

#define	SSP_BASE	0x80070060

#if LANGUAGE == C
typedef volatile struct SSP_registers {
	uint32_t sscr0;
	uint32_t sscr1;
	uint32_t __reserved1;
	uint32_t ssdr;
	uint32_t __reserved2;
	uint32_t sssr;
} SSP_registers_t;

#ifdef SA11X0_UNMAPPED
#define	SSP_pointer	((SSP_registers_t*) SSP_BASE)
#endif

#define	SSCR0		SSP_pointer->sscr0
#define	SSCR1		SSP_pointer->sscr1
#define	SSDR		SSP_pointer->ssdr
#define	SSSR		SSP_pointer->sssr
#endif /* LANGUAGE == C */

#define	SSCR0_OFFSET	0x00
#define	SSCR1_OFFSET	0x04
#define	SSDR_OFFSET	0x0C
#define	SSSR_OFFSET	0x14

/* SSCR0 bits - see 11.12.9 */

#define	SSCR0_SCR_MASK	bits(15,8)
#define	SSCR0_SCR(x)	bits_val(15,8,x)
#define	SSCR0_SSE	bit(7)
#define	SSCR0_FRF_MASK	bits(5,4)
#define	SSCR0_FRF(x)	bits_val(5,4,x)
#define	SSCR0_DSS_MASK	bits(3,0)
#define	SSCR0_DSS(x)	bits_val(3,0,x)

/* SSCR1 bits */

#define	SSCR1_ECS	bit(5)
#define	SSCR1_SPH	bit(4)
#define	SSCR1_SPO	bit(3)
#define	SSCR1_LBM	bit(2)
#define	SSCR1_TIE	bit(1)
#define	SSCR1_RIE	bit(0)

/* SSSR bits */

#define	SSSR_ROR	bit(6)
#define	SSSR_RFS	bit(5)
#define	SSSR_TFS	bit(4)
#define	SSSR_BSY	bit(3)
#define	SSSR_RNE	bit(2)
#define	SSSR_TNF	bit(1)

#endif /* SA11X0_SSP_H */
