/*
 * $Id$
 *
 * XScale PXA250/PXA210 Memory Controller Registers
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
 * [2] Intel Corporation, "Intel PXA250 and PXA210 Application Processors
 *     Specification Update", June 2002, Order Number: 278534-007
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

/* MDCNFG bits - see Table 6-3 in [1] and D25. in [2] */

#define	MDCNFG_DSA1111_2	bit(28)
#define	MDCNFG_DLATCH2		bit(27)
#define	MDCNFG_DTC2_MASK	bits(25,24)
#define	MDCNFG_DTC2(x)		bits_val(25,24,x)
#define	MDCNFG_DNB2		bit(23)
#define	MDCNFG_DRAC2_MASK	bits(22,21)
#define	MDCNFG_DRAC2(x)		bits_val(22,21,x)
#define	MDCNFG_DCAC2_MASK	bits(20,19)
#define	MDCNFG_DCAC2(x)		bits_val(20,19,x)
#define	MDCNFG_DWID2		bit(18)
#define	MDCNFG_DE3		bit(17)
#define	MDCNFG_DE2		bit(16)
#define	MDCNFG_DSA1111_0	bit(12)
#define	MDCNFG_DLATCH0		bit(11)
#define	MDCNFG_DTC0_MASK	bits(9,8)
#define	MDCNFG_DTC0(x)		bits_val(9,8,x)
#define	MDCNFG_DNB0		bit(7)
#define	MDCNFG_DRAC0_MASK	bits(6,5)
#define	MDCNFG_DRAC0(x)		bits_val(6,5,x)
#define	MDCNFG_DCAC0_MASK	bits(4,3)
#define	MDCNFG_DCAC0(x)		bits_val(4,3,x)
#define	MDCNFG_DWID0		bit(2)
#define	MDCNFG_DE1		bit(1)
#define	MDCNFG_DE0		bit(0)

/* MDREFR bits - see Table 6-5 in [1] */

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
#define	MDREFR_DRI_MASK		bits(11,0)
#define	MDREFR_DRI(x)		bits(11,0,x)

/* MDMRS bits - see Table 6-4 in [1] */

#define	MDMRS_MDMRS2_MASK	bits(30,23)
#define	MDMRS_MDMRS2(x)		bits_val(30,23,x)
#define	MDMRS_MDCL2_MASK	bits(22,20)
#define	MDMRS_MDCL2(x)		bits_val(22,20)
#define	MDMRS_MDADD2		bit(19)
#define	MDMRS_MDBL2_MASK	bits(18,16)
#define	MDMRS_MDBL2(x)		bits_val(18,16,x)
#define	MDMRS_MDMRS0_MASK	bits(14,7)
#define	MDMRS_MDMRS0(x)		bits_val(14,7,x)
#define	MDMRS_MDCL0_MASK	bits(6,4)
#define	MDMRS_MDCL0(x)		bits_val(6,4,x)
#define	MDMRS_MDADD0		bit(3)
#define	MDMRS_MDBL0_MASK	bits(2,0)
#define	MDMRS_MDBL0(x)		bits_val(2,0,x)

#endif /* PXA2X0_MC_H */
