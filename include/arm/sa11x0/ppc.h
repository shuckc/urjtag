/*
 * $Id$
 *
 * StrongARM SA-1110 PPC Registers
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

#ifndef	SA11X0_PPC_H
#define	SA11X0_PPC_H

#include <openwince.h>

#if LANGUAGE == C
#include <stdint.h>
#endif

/* PPC Registers */

#define	PPC_BASE	0x90060000

#if LANGUAGE == C
typedef volatile struct PPC_registers {
	uint32_t ppdr;
	uint32_t ppsr;
	uint32_t ppar;
	uint32_t psdr;
	uint32_t ppfr;
	uint32_t __reserved1[5];
	uint32_t hscr2;
	uint32_t __reserved2;
	uint32_t mccr1;
} PPC_registers_t;

#ifdef SA11X0_UNMAPPED
#define	PPC_pointer	((PPC_registers_t*) PPC_BASE)
#endif

#define	PPDR		PPC_pointer->ppdr
#define	PPSR		PPC_pointer->ppsr
#define	PPAR		PPC_pointer->ppar
#define	PSDR		PPC_pointer->psdr
#define	PPFR		PPC_pointer->ppfr
#define	HSCR2		PPC_pointer->hscr2
#define	MCCR1		PPC_pointer->mccr1
#endif /* LANGUAGE == C */

#define	PPDR_OFFSET	0x00
#define	PPSR_OFFSET	0x04
#define	PPAR_OFFSET	0x08
#define	PSDR_OFFSET	0x0C
#define	PPFR_OFFSET	0x10
#define	HSCR2_OFFSET	0x28
#define	MCCR1_OFFSET	0x30

#endif /* SA11X0_PPC_H */
