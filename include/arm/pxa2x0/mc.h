/*
 * $Id$
 *
 * XScale PXA250/PXA210 Memory Controller Registers
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
 * [1] Intel Corporation, "Intel PXA250 and PXA210 Application Processors
 *     Developer's Manual", February 2002, Order Number: 278522-001
 * [2] Intel Corporation, "Intel PXA250 and PXA210 Application Processors
 *     Specification Update", May 2002, Order Number: 278534-005
 *
 */

#ifndef	PXA2X0_MC_H
#define	PXA2X0_MC_H

#include <common.h>

#if LANGUAGE == C
#include <stdint.h>
#endif

/* Memory Controller Registers */

#define	MC_BASE		0x48000000

#if LANGUAGE == C
typedef volatile struct MC_registers {
	uint32_t mdcnfg;
	uint32_t mdrefr;
	uint32_t msc0;
	uint32_t msc1;
	uint32_t msc2;
	uint32_t mecr;
	uint32_t __reserved1;
	uint32_t sxcnfg;
	uint32_t __reserved2;
	uint32_t sxmrs;
	uint32_t mcmem0;
	uint32_t mcmem1;
	uint32_t mcatt0;
	uint32_t mcatt1;
	uint32_t mcio0;
	uint32_t mcio1;
	uint32_t mdmrs;
	uint32_t boot_def;
} MC_registers;

#ifdef PXA2X0_UNMAPPED
#define	MC_pointer	((MC_registers*) MC_BASE)
#endif

#define	MDCNFG		MC_pointer->mdcnfg
#define	MDREFR		MC_pointer->mdrefr
#define	MSC0		MC_pointer->msc0
#define	MSC1		MC_pointer->msc1
#define	MSC2		MC_pointer->msc2
#define	MECR		MC_pointer->mecr
#define	SXCNFG		MC_pointer->sxcnfg
#define	SXMRS		MC_pointer->sxmrs
#define	MCMEM0		MC_pointer->mcmem0
#define	MCMEM1		MC_pointer->mcmem1
#define	MCATT0		MC_pointer->mcatt0
#define	MCATT1		MC_pointer->mcatt1
#define	MCIO0		MC_pointer->mcio0
#define	MCIO1		MC_pointer->mcio1
#define	MDMRS		MC_pointer->mdmrs
#define	BOOT_DEF	MC_pointer->boot_def
#endif /* LANGUAGE == C */

#define	MDCNFG_OFFSET	0x00
#define	MDREFR_OFFSET	0x04
#define	MSC0_OFFSET	0x08
#define	MSC1_OFFSET	0x0C
#define	MSC2_OFFSET	0x10
#define	MECR_OFFSET	0x14
#define	SXCNFG_OFFSET	0x1C
#define	SXMRS_OFFSET	0x24
#define	MCMEM0_OFFSET	0x28
#define	MCMEM1_OFFSET	0x2C
#define	MCATT0_OFFSET	0x30
#define	MCATT1_OFFSET	0x34
#define	MCIO0_OFFSET	0x38
#define	MCIO1_OFFSET	0x3C
#define	MDMRS_OFFSET	0x40
#define	BOOT_DEF	0x44

/* MDCNFG bits */

#define	MDCNFG_DSA1111_2	bit(28)
#define	MDCNFG_DLATCH2		bit(27)
#define	MDCNFG_DADDR2		bit(26)
#define	MDCNFG_DTC2_MASK	0x03000000
#define	MDCNFG_DTC2(x)		((x << 24) & MDCNFG_DTC2_MASK)
#define	MDCNFG_DNB2		bit(23)
#define	MDCNFG_DRAC2_MASK	0x00600000
#define	MDCNFG_DRAC2(x)		((x << 21) & MDCNFG_DRAC2_MASK)
#define	MDCNFG_DCAC2_MASK	0x00180000
#define	MDCNFG_DCAC2(x)		((x << 19) & MDCNFG_DCAC2_MASK)
#define	MDCNFG_DWID2		bit(18)
#define	MDCNFG_DE3		bit(17)
#define	MDCNFG_DE2		bit(16)
#define	MDCNFG_DSA1111_0	bit(12)
#define	MDCNFG_DLATCH0		bit(11)
#define	MDCNFG_DADDR0		bit(10)
#define	MDCNFG_DTC0_MASK	0x00000300
#define	MDCNFG_DTC0(x)		((x << 8) & MDCNFG_DTC0_MASK)
#define	MDCNFG_DNB0		bit(7)
#define	MDCNFG_DRAC0_MASK	0x00000060
#define	MDCNFG_DRAC0(x)		((x << 5) & MDCNFG_DRAC0_MASK)
#define	MDCNFG_DCAC0_MASK	0x00000018
#define	MDCNFG_DCAC0(x)		((x << 3) & MDCNFG_DCAC0_MASK)
#define	MDCNFG_DWID0		bit(2)
#define	MDCNFG_DE1		bit(1)
#define	MDCNFG_DE0		bit(0)

/* MDREFR bits */

#define	MDREFR_K2FREE		bit(25)
#define	MDREFR_K1FREE		bit(24)
#define	MDREFR_K0FREE		bit(23)
#define	MDREFR_SLFRSH		bit(22)
#define	MDREFR_APD		bit(20)
#define	MDREFR_K2DB2		bit(19)
#define	MDREFR_K2RUN		bit(18)
#define	MDREFR_K1DB2		bit(17)
#define	MDREFR_K1RUN		bit(16)
#define	MDREFR_E1PIN		bit(15)
#define	MDREFR_K0DB2		bit(14)
#define	MDREFR_K0RUN		bit(13)
#define	MDREFR_E0PIN		bit(12)
#define	MDREFR_DRI_MASK		0x00000FFF
#define	MDREFR_DRI(x)		(x & MDREFR_DRI_MASK)

/* MDMRS bits */

#define	MDMRS_MDMRS2_MASK	0x7F800000
#define	MDMRS_MDMRS2(x)		((x << 23) & MDMRS_MDMRS2_MASK)
#define	MDMRS_MDCL2_MASK	0x00700000
#define	MDMRS_MDCL2(x)		((x << 20) & MDMRS_MDCL2_MASK)
#define	MDMRS_MDADD2		bit(19)
#define	MDMRS_MDBL2_MASK	0x00070000
#define	MDMRS_MDBL2(x)		((x << 16) & MDMRS_MDBL2_MASK)
#define	MDMRS_MDMRS0_MASK	0x00007F80
#define	MDMRS_MDMRS0(x)		((x << 7) & MDMRS_MDMRS0_MASK)
#define	MDMRS_MDCL0_MASK	0x00000070
#define	MDMRS_MDCL0(x)		((x << 4) & MDMRS_MDCL0_MASK)
#define	MDMRS_MDADD0		bit(3)
#define	MDMRS_MDBL0_MASK	0x00000007
#define	MDMRS_MDBL0(x)		(x & MDMRS_MDBL0_MASK)

#endif /* PXA2X0_MC_H */
