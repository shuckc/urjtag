/*
 * $Id$
 *
 * StrongARM SA-1110 Memory Controller Registers
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

#ifndef	SA11X0_MC_H
#define	SA11X0_MC_H

#include <common.h>

#if LANGUAGE == C
#include <stdint.h>
#endif

/* Memory Controller Registers */

#define	MC_BASE		0xA0000000

#if LANGUAGE == C
typedef volatile struct MC_registers {
	uint32_t mdcnfg;
	uint32_t mdcas00;
	uint32_t mdcas01;
	uint32_t mdcas02;
	uint32_t msc0;
	uint32_t msc1;
	uint32_t mecr;
	uint32_t mdrefr;
	uint32_t mdcas20;
	uint32_t mdcas21;
	uint32_t mdcas22;
	uint32_t msc2;
	uint32_t smcnfg;
} MC_registers;

#ifdef SA11X0_UNMAPPED
#define	MC_pointer	((MC_registers*) MC_BASE)
#endif

#define	MDCNFG		MC_pointer->mdcnfg
#define	MDCAS00		MC_pointer->mdcas00
#define	MDCAS01		MC_pointer->mdcas01
#define	MDCAS02		MC_pointer->mdcas02
#define	MSC0		MC_pointer->msc0
#define	MSC1		MC_pointer->msc1
#define	MECR		MC_pointer->mecr
#define	MDREFR		MC_pointer->mdrefr
#define	MDCAS20		MC_pointer->mdcas20
#define	MDCAS21		MC_pointer->mdcas21
#define	MDCAS22		MC_pointer->mdcas22
#define	MSC2		MC_pointer->msc2
#define	SMCNFG		MC_pointer->smcnfg
#endif /* LANGUAGE == C */

#define	MDCNFG_OFFSET	0x00
#define	MDCAS00_OFFSET	0x04
#define	MDCAS01_OFFSET	0x08
#define	MDCAS02_OFFSET	0x0C
#define	MSC0_OFFSET	0x10
#define	MSC1_OFFSET	0x14
#define	MECR_OFFSET	0x18
#define	MDREFR_OFFSET	0x1C
#define	MDCAS20_OFFSET	0x20
#define	MDCAS21_OFFSET	0x24
#define	MDCAS22_OFFSET	0x28
#define	MSC2_OFFSET	0x2C
#define	SMCNFG_OFFSET	0x30

/* MDCNFG bits */

#define	MDCNFG_TWR2_MASK	0xC0000000
#define	MDCNFG_TWR2(x)		((x << 30) & MDCNFG_TWR2_MASK)
#define	MDCNFG_TDL2_MASK	0x30000000
#define	MDCNFG_TDL2(x)		((x << 28) & MDCNFG_TDL2_MASK)
#define	MDCNFG_TRP2_MASK	0x0F000000
#define	MDCNFG_TRP2(x)		((x << 24) & MDCNFG_TRP2_MASK)
#define	MDCNFG_CDB22		bit(23)
#define	MDCNFG_DRAC2_MASK	0x00700000
#define	MDCNFG_DRAC2(x)		((x << 20) & MDCNFG_DRAC2_MASK)
#define	MDCNFG_DWID2		bit(19)
#define	MDCNFG_DTIM2		bit(18)
#define	MDCNFG_DE3		bit(17)
#define	MDCNFG_DE2		bit(16)
#define	MDCNFG_TWR0_MASK	0x0000C000
#define	MDCNFG_TWR0(x)		((x << 14) & MDCNFG_TWR0_MASK)
#define	MDCNFG_TDL0_MASK	0x00003000
#define	MDCNFG_TDL0(x)		((x << 12) & MDCNFG_TDL0_MASK)
#define	MDCNFG_TRP0_MASK	0x00000F00
#define	MDCNFG_TRP0(x)		((x << 8) & MDCNFG_TRP0_MASK)
#define	MDCNFG_CDB20		bit(7)
#define	MDCNFG_DRAC0_MASK	0x00000070
#define	MDCNFG_DRAC0(x)		((x << 4) & MDCNFG_DRAC0_MASK)
#define	MDCNFG_DWID0		bit(3)
#define	MDCNFG_DTIM0		bit(2)
#define	MDCNFG_DE1		bit(1)
#define	MDCNFG_DE0		bit(0)

/* MDREFR bits */

#define	MDREFR_SLFRSH		bit(31)
#define	MDREFR_KAPD		bit(29)
#define	MDREFR_EAPD		bit(28)
#define	MDREFR_K2DB2		bit(26)
#define	MDREFR_K2RUN		bit(25)
#define	MDREFR_K1DB2		bit(22)
#define	MDREFR_K1RUN		bit(21)
#define	MDREFR_E1PIN		bit(20)
#define	MDREFR_K0DB2		bit(18)
#define	MDREFR_K0RUN		bit(17)
#define	MDREFR_E0PIN		bit(16)
#define	MDREFR_DRI_MASK		0x0000FFF0
#define	MDREFR_DRI(x)		((x << 4) & MDREFR_DRI_MASK)
#define	MDREFR_TRASR_MASK	0x0000000F
#define	MDREFR_TRASR(x)		(x & MDREFR_TRASR_MASK)

#endif /* SA11X0_MC_H */
