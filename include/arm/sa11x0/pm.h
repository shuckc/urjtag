/*
 * $Id$
 *
 * StrongARM SA-1110 Power Manager Registers
 * Copyright (C) 2002 ETC s.r.o.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 *
 * Written by Marcel Telka <marcel@telka.sk>, 2002.
 *
 * Documentation:
 * [1] Intel Corporation, "Intel StrongARM SA-1110 Microprocessor
 *     Developer's Manual", October 2001, Order Number: 278240-004
 * [2] Intel Corporation, "Intel StrongARM SA-1110 Microprocessor
 *     Specification Update", December 2001, Order Number: 278259-023
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
