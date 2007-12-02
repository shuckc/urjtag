/*
 * $Id$
 *
 * StrongARM SA-1110 Memory Controller Registers
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

#ifndef	SA11X0_MC_H
#define	SA11X0_MC_H

#include <openwince.h>

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
} MC_registers_t;

#ifdef SA11X0_UNMAPPED
#define	MC_pointer	((MC_registers_t*) MC_BASE)
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

/* MDCNFG bits - see 10.3.1 in [1] */

#define	MDCNFG_TWR2_MASK	bits(31,30)
#define	MDCNFG_TWR2(x)		bits_val(31,30,x)
#define	MDCNFG_TDL2_MASK	bits(29,28)
#define	MDCNFG_TDL2(x)		bits_val(29,28,x)
#define	MDCNFG_TRP2_MASK	bits(27,24)
#define	MDCNFG_TRP2(x)		bits_val(27,24,x)
#define	MDCNFG_CDB22		bit(23)
#define	MDCNFG_DRAC2_MASK	bits(22,20)
#define	MDCNFG_DRAC2(x)		bits_val(22,20,x)
#define	MDCNFG_DWID2		bit(19)
#define	MDCNFG_DTIM2		bit(18)
#define	MDCNFG_DE3		bit(17)
#define	MDCNFG_DE2		bit(16)
#define	MDCNFG_TWR0_MASK	bits(15,14)
#define	MDCNFG_TWR0(x)		bits_val(15,14,x)
#define	MDCNFG_TDL0_MASK	bits(13,12)
#define	MDCNFG_TDL0(x)		bits_val(13,12,x)
#define	MDCNFG_TRP0_MASK	bits(11,8)
#define	MDCNFG_TRP0(x)		bits_val(11,8,x)
#define	MDCNFG_CDB20		bit(7)
#define	MDCNFG_DRAC0_MASK	bits(6,4)
#define	MDCNFG_DRAC0(x)		bits_val(6,4,x)
#define	MDCNFG_DWID0		bit(3)
#define	MDCNFG_DTIM0		bit(2)
#define	MDCNFG_DE1		bit(1)
#define	MDCNFG_DE0		bit(0)

/* MDREFR bits - see 10.3.2 in [1] */

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
#define	MDREFR_DRI_MASK		bits(15,4)
#define	MDREFR_DRI(x)		bits_val(15,4,x)
#define	MDREFR_TRASR_MASK	bits(3,0)
#define	MDREFR_TRASR(x)		bits_val(3,0,x)

#endif /* SA11X0_MC_H */
