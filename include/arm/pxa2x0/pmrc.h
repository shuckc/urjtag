/*
 * $Id$
 *
 * XScale PXA26x/PXA250/PXA210 Power Manager and Reset Control Registers
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

#ifndef	PXA2X0_PMRC_H
#define	PXA2X0_PMRC_H

#include <common.h>

#if LANGUAGE == C
#include <stdint.h>
#endif

/* Power Manager and Reset Control Registers */

#define	PMRC_BASE	0x40F00000

#if LANGUAGE == C
typedef volatile struct PMRC_registers {
	uint32_t pmcr;
	uint32_t pssr;
	uint32_t pspr;
	uint32_t pwer;
	uint32_t prer;
	uint32_t pfer;
	uint32_t pedr;
	uint32_t pcfr;
	uint32_t pgsr0;
	uint32_t pgsr1;
	uint32_t pgsr2;
	uint32_t __reserved;
	uint32_t rcsr;
} PMRC_registers_t;

#ifdef PXA2X0_UNMAPPED
#define	PMRC_pointer	((PMRC_registers_t*) PMRC_BASE)
#endif

#define	PMCR		PMRC_pointer->pmcr
#define	PSSR		PMRC_pointer->pssr
#define	PSPR		PMRC_pointer->pspr
#define	PWER		PMRC_pointer->pwer
#define	PRER		PMRC_pointer->prer
#define	PFER		PMRC_pointer->pfer
#define	PEDR		PMRC_pointer->pedr
#define	PCFR		PMRC_pointer->pcfr
#define	PGSR0		PMRC_pointer->pgsr0
#define	PGSR1		PMRC_pointer->pgsr1
#define	PGSR2		PMRC_pointer->pgsr2
#define	RCSR		PMRC_pointer->rcsr
#endif /* LANGUAGE == C */

#define	PMCR_OFFSET	0x00
#define	PSSR_OFFSET	0x04
#define	PSPR_OFFSET	0x08
#define	PWER_OFFSET	0x0C
#define	PRER_OFFSET	0x10
#define	PFER_OFFSET	0x14
#define	PEDR_OFFSET	0x18
#define	PCFR_OFFSET	0x1C
#define	PGSR0_OFFSET	0x20
#define	PGSR1_OFFSET	0x24
#define	PGSR2_OFFSET	0x28
#define	RCSR_OFFSET	0x30

/* PMCR bits - see Table 3-7 in [1], Table 3-7 in [2] */

#define	PMCR_IDAE	bit(0)

/* PSSR bits - see Table 3-13 in [1], Table 3-13 in [2] */

#define	PSSR_RDH	bit(5)
#define	PSSR_PH		bit(4)
#define	PSSR_VFS	bit(2)
#define	PSSR_BFS	bit(1)
#define	PSSR_SSS	bit(0)

/* PWER bits - see Table 3-9 in [1], Table 3-9 in [2] */

#define	PWER_WERTC	bit(31)
#define	PWER_WE15	bit(15)
#define	PWER_WE14	bit(14)
#define	PWER_WE13	bit(13)
#define	PWER_WE12	bit(12)
#define	PWER_WE11	bit(11)
#define	PWER_WE10	bit(10)
#define	PWER_WE9	bit(9)
#define	PWER_WE8	bit(8)
#define	PWER_WE7	bit(7)
#define	PWER_WE6	bit(6)
#define	PWER_WE5	bit(5)
#define	PWER_WE4	bit(4)
#define	PWER_WE3	bit(3)
#define	PWER_WE2	bit(2)
#define	PWER_WE1	bit(1)
#define	PWER_WE0	bit(0)

/* PCFR bits - see Table 3-8 in [1], Table 3-8 in [2] */

#define	PCFR_FS		bit(2)
#define	PCFR_FP		bit(1)
#define	PCFR_OPDE	bit(0)

/* RCSR bits - see Table 3-18 in [1], Table 3-18 in [2] */

#define	RCSR_GPR	bit(3)
#define	RCSR_SMR	bit(2)
#define	RCSR_WDR	bit(1)
#define	RCSR_HWR	bit(0)

#endif /* PXA2X0_PMRC_H */
