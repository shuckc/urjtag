/*
 * $Id$
 *
 * StrongARM SA-1110 Power Manager Registers
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

#ifndef	SA11X0_PM_H
#define	SA11X0_PM_H

#include <common.h>

#if LANGUAGE == C
#include <stdint.h>
#endif

/* Power Manager Registers */

#define	PM_BASE		0x90020000

#if LANGUAGE == C
typedef volatile struct PM_registers {
	uint32_t pmcr;
	uint32_t pssr;
	uint32_t pspr;
	uint32_t pwer;
	uint32_t pcfr;
	uint32_t ppcr;
	uint32_t pgsr;
	uint32_t posr;
} PM_registers;

#ifdef SA11X0_UNMAPPED
#define	PM_pointer	((PM_registers*) PM_BASE)
#endif

#define	PMCR		PM_pointer->pmcr
#define	PSSR		PM_pointer->pssr
#define	PSPR		PM_pointer->pspr
#define	PWER		PM_pointer->pwer
#define	PCFR		PM_pointer->pcfr
#define	PPCR		PM_pointer->ppcr
#define	PGSR		PM_pointer->pgsr
#define	POSR		PM_pointer->posr
#endif /* LANGUAGE == C */

#define	PMCR_OFFSET	0x00
#define	PSSR_OFFSET	0x04
#define	PSPR_OFFSET	0x08
#define	PWER_OFFSET	0x0C
#define	PCFR_OFFSET	0x10
#define	PPCR_OFFSET	0x14
#define	PGSR_OFFSET	0x18
#define	POSR_OFFSET	0x1C

/* PMCR bits */

#define	PMCR_SF		bit(0)

/* PCFR bits */

#define	PCFR_FO		bit(3)
#define	PCFR_FS		bit(2)
#define	PCFR_FP		bit(1)
#define	PCFR_OPDE	bit(0)

/* PPCR bits */

#define	PPCR_CCF_MASK	0x1F
#define	PPCR_CCF(x)	(x & PPCR_CCF_MASK)

#define	PPCR_CCF_59_0	PPCR_CCF(0x00)
#define	PPCR_CCF_73_7	PPCR_CCF(0x01)
#define	PPCR_CCF_88_5	PPCR_CCF(0x02)
#define	PPCR_CCF_103_2	PPCR_CCF(0x03)
#define	PPCR_CCF_118_0	PPCR_CCF(0x04)
#define	PPCR_CCF_132_7	PPCR_CCF(0x05)
#define	PPCR_CCF_147_5	PPCR_CCF(0x06)
#define	PPCR_CCF_162_2	PPCR_CCF(0x07)
#define	PPCR_CCF_176_9	PPCR_CCF(0x08)
#define	PPCR_CCF_191_7	PPCR_CCF(0x09)
#define	PPCR_CCF_206_4	PPCR_CCF(0x0A)
#define	PPCR_CCF_221_2	PPCR_CCF(0x0B)

/* PSSR bits */

#define	PSSR_PH		bit(4)
#define	PSSR_DH		bit(3)
#define	PSSR_VFS	bit(2)
#define	PSSR_BFS	bit(1)
#define	PSSR_SSS	bit(0)

/* POSR bits */

#define	POSR_OOK	bit(0)

#endif /* SA11X0_PM_H */
