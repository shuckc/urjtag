/*
 * $Id$
 *
 * XScale PXA26x/PXA250/PXA210 Memory Controller Registers
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
 *     Specification Update", October 2002, Order Number: 278534-009
 * [3] Intel Corporation, "Intel PXA26x Processor Family Developer's Manual",
 *     October 2002, Order Number: 278638-001
 *
 */

#ifndef	PXA2X0_MC_H
#define	PXA2X0_MC_H

#include <common.h>

#if LANGUAGE == C
#include <stdint.h>
#endif

#if defined(PXA2X0_NOPXA250) && !defined(PXA2X0_NOPXA26X)
#define	PXA2X0_NOPXA26X
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
#if !defined(PXA2X0_NOPXA26X)
	uint32_t __reserved3[4];
	uint32_t mdmrslp;
	uint32_t __reserved4[2];
	uint32_t sa1111cr;
#endif /* PXA26x only */
} MC_registers_t;

#ifdef PXA2X0_UNMAPPED
#define	MC_pointer	((MC_registers_t*) MC_BASE)
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
#if !defined(PXA2X0_NOPXA26X)
#define	MDMRSLP		MC_pointer->mdmrslp
#define	SA1111CR	MC_pointer->sa1111cr
#endif /* PXA26x only */
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
#define	BOOT_DEF_OFFSET	0x44
#if !defined(PXA2X0_NOPXA26X)
#define	MDMRSLP_OFFSET	0x58
#define	SA1111CR_OFFSET	0x64
#endif /* PXA26x only */

/* MDCNFG bits - see Table 6-3 in [1] and D25 in [2], Table 6-3 in [3] */

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

/* MDREFR bits - see Table 6-5 in [1], Table 6-6 in [3] */

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
#define	MDREFR_DRI(x)		bits_val(11,0,x)

/* MSC0 bits - see Table 6-21 in [1], Table 6-25 in [3] */

#define	MSC0_RBUFF1		bit(31)
#define	MSC0_RRR1_MASK		bits(30,28)
#define	MSC0_RRR1(x)		bits_val(30,28,x)
#define	MSC0_RDN1_MASK		bits(27,24)
#define	MSC0_RDN1(x)		bits_val(27,24,x)
#define	MSC0_RDF1_MASK		bits(23,20)
#define	MSC0_RDF1(x)		bits_val(23,20,x)
#define	MSC0_RBW1		bit(19)
#define	MSC0_RT1_MASK		bits(18,16)
#define	MSC0_RT1(x)		bits_val(18,16,x)
#define	MSC0_RBUFF0		bit(15)
#define	MSC0_RRR0_MASK		bits(14,12)
#define	MSC0_RRR0(x)		bits_val(14,12,x)
#define	MSC0_RDN0_MASK		bits(11,9)
#define	MSC0_RDN0(x)		bits_val(11,8,x)
#define	MSC0_RDF0_MASK		bits(7,4)
#define	MSC0_RDF0(x)		bits_val(7,4,x)
#define	MSC0_RBW0		bit(3)
#define	MSC0_RT0_MASK		bits(2,0)
#define	MSC0_RT0(x)		bits_val(2,0,x)

/* MSC1 bits - see Table 6-21 in [1], Table 6-25 in [3] */

#define	MSC1_RBUFF3		bit(31)
#define	MSC1_RRR3_MASK		bits(30,28)
#define	MSC1_RRR3(x)		bits_val(30,28,x)
#define	MSC1_RDN3_MASK		bits(27,24)
#define	MSC1_RDN3(x)		bits_val(27,24,x)
#define	MSC1_RDF3_MASK		bits(23,20)
#define	MSC1_RDF3(x)		bits_val(23,20,x)
#define	MSC1_RBW3		bit(19)
#define	MSC1_RT3_MASK		bits(18,16)
#define	MSC1_RT3(x)		bits_val(18,16,x)
#define	MSC1_RBUFF2		bit(15)
#define	MSC1_RRR2_MASK		bits(14,12)
#define	MSC1_RRR2(x)		bits_val(14,12,x)
#define	MSC1_RDN2_MASK		bits(11,9)
#define	MSC1_RDN2(x)		bits_val(11,8,x)
#define	MSC1_RDF2_MASK		bits(7,4)
#define	MSC1_RDF2(x)		bits_val(7,4,x)
#define	MSC1_RBW2		bit(3)
#define	MSC1_RT2_MASK		bits(2,0)
#define	MSC1_RT2(x)		bits_val(2,0,x)

/* MSC2 bits - see Table 6-21 in [1], Table 6-25 in [3] */

#define	MSC2_RBUFF5		bit(31)
#define	MSC2_RRR5_MASK		bits(30,28)
#define	MSC2_RRR5(x)		bits_val(30,28,x)
#define	MSC2_RDN5_MASK		bits(27,24)
#define	MSC2_RDN5(x)		bits_val(27,24,x)
#define	MSC2_RDF5_MASK		bits(23,20)
#define	MSC2_RDF5(x)		bits_val(23,20,x)
#define	MSC2_RBW5		bit(19)
#define	MSC2_RT5_MASK		bits(18,16)
#define	MSC2_RT5(x)		bits_val(18,16,x)
#define	MSC2_RBUFF4		bit(15)
#define	MSC2_RRR4_MASK		bits(14,12)
#define	MSC2_RRR4(x)		bits_val(14,12,x)
#define	MSC2_RDN4_MASK		bits(11,9)
#define	MSC2_RDN4(x)		bits_val(11,8,x)
#define	MSC2_RDF4_MASK		bits(7,4)
#define	MSC2_RDF4(x)		bits_val(7,4,x)
#define	MSC2_RBW4		bit(3)
#define	MSC2_RT4_MASK		bits(2,0)
#define	MSC2_RT4(x)		bits_val(2,0,x)

/* MDMRS bits - see Table 6-4 in [1], Table 6-4 in [3] */

#define	MDMRS_MDMRS2_MASK	bits(30,23)
#define	MDMRS_MDMRS2(x)		bits_val(30,23,x)
#define	MDMRS_MDCL2_MASK	bits(22,20)
#define	MDMRS_MDCL2(x)		bits_val(22,20,x)
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

#if !defined(PXA2X0_NOPXA26X)
/* MDMRSLP bits - see Table 6-5 in [3] */

#define	MDMRSLP_MDLPEN2		bit(31)
#define	MDMRSLP_MDMRSLP2_MASK	bits(30,16)
#define	MDMRSLP_MDMRSLP2(x)	bits_val(30,16,x)
#define	MDMRSLP_MDLPEN0		bit(15)
#define	MDMRSLP_MDMRSLP0_MASK	bits(14,0)
#define	MDMRSLP_MDMRSLP0(x)	bits_val(14,0,x)

/* SA1111CR bits - see Table 6-24 in [3] */

#define	SA1111CR_SA1111_5	bit(5)
#define	SA1111CR_SA1111_4	bit(4)
#define	SA1111CR_SA1111_3	bit(3)
#define	SA1111CR_SA1111_2	bit(2)
#define	SA1111CR_SA1111_1	bit(1)
#define	SA1111CR_SA1111_0	bit(0)
#endif /* PXA26x only */

#endif /* PXA2X0_MC_H */
