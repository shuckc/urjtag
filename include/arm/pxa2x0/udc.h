/*
 * $Id$
 *
 * XScale PXA26x/PXA255/PXA250/PXA210 UDC Registers
 * Copyright (C) 2002, 2003 ETC s.r.o.
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
 * Written by Marcel Telka <marcel@telka.sk>, 2002, 2003.
 *
 * Documentation:
 * [1] Intel Corporation, "Intel PXA250 and PXA210 Application Processors
 *     Developer's Manual", February 2002, Order Number: 278522-001
 * [2] Intel Corporation, "Intel PXA26x Processor Family Developer's Manual",
 *     March 2003, Order Number: 278638-002
 * [3] Intel Corporation, "Intel PXA255 Processor Developer's Manual"
 *     March 2003, Order Number: 278693-001
 *
 */

#ifndef	PXA2X0_UDC_H
#define	PXA2X0_UDC_H

#include <common.h>

#if LANGUAGE == C
#include <stdint.h>
#endif

#if defined(PXA2X0_NOPXA250) && !defined(PXA2X0_NOPXA255)
#define PXA2X0_NOPXA255
#endif

#if defined(PXA2X0_NOPXA255) && !defined(PXA2X0_NOPXA260)
#define PXA2X0_NOPXA260
#endif

/* UDC Registers */

#define	UDC_BASE	0x40600000

#if LANGUAGE == C
typedef volatile struct UDC_registers {
	uint32_t udccr;
	uint32_t __reserved1;
#if !defined(PXA2X0_NOPXA255)
	uint32_t udccfr;
#else /* PXA255 and above only */
	uint32_t __reserved2;
#endif
	uint32_t __reserved3;
	uint32_t udccs[16];
	uint32_t uicr0;
	uint32_t uicr1;
	uint32_t usir0;
	uint32_t usir1;
	uint32_t ufnhr;				/* see 12.6.12 in [1] */
	uint32_t ufnlr;				/* see 12.6.13 in [1] */
	uint32_t ubcr2;
	uint32_t ubcr4;
	uint32_t ubcr7;
	uint32_t ubcr9;
	uint32_t ubcr12;
	uint32_t ubcr14;
	uint32_t uddr0;
	uint32_t __reserved4[7];
	uint32_t uddr5;
	uint32_t __reserved5[7];
	uint32_t uddr10;
	uint32_t __reserved6[7];
	uint32_t uddr15;
	uint32_t __reserved7[7];
	uint32_t uddr1;
	uint32_t __reserved8[31];
	uint32_t uddr2;
	uint32_t __reserved9[31];
	uint32_t uddr3;
	uint32_t __reserved10[127];
	uint32_t uddr4;
	uint32_t __reserved11[127];
	uint32_t uddr6;
	uint32_t __reserved12[31];
	uint32_t uddr7;
	uint32_t __reserved13[31];
	uint32_t uddr8;
	uint32_t __reserved14[127];
	uint32_t uddr9;
	uint32_t __reserved15[127];
	uint32_t uddr11;
	uint32_t __reserved16[31];
	uint32_t uddr12;
	uint32_t __reserved17[31];
	uint32_t uddr13;
	uint32_t __reserved18[127];
	uint32_t uddr14;
} UDC_registers_t;

#ifdef PXA2X0_UNMAPPED
#define	UDC_pointer	((UDC_registers_t*) UDC_BASE)
#endif

#define	UDCCR			UDC_pointer->udccr
#if !defined(PXA2X0_NOPXA255)
#define	UDCCFR			UDC_pointer->udccfr
#endif /* PXA255 and above only */
#define	UDCCS(i)		UDC_pointer->udccs[i]
#define	UDCCS0			UDCCS(0)
#define	UDCCS1			UDCCS(1)
#define	UDCCS2			UDCCS(2)
#define	UDCCS3			UDCCS(3)
#define	UDCCS4			UDCCS(4)
#define	UDCCS5			UDCCS(5)
#define	UDCCS6			UDCCS(6)
#define	UDCCS7			UDCCS(7)
#define	UDCCS8			UDCCS(8)
#define	UDCCS9			UDCCS(9)
#define	UDCCS10			UDCCS(10)
#define	UDCCS11			UDCCS(11)
#define	UDCCS12			UDCCS(12)
#define	UDCCS13			UDCCS(13)
#define	UDCCS14			UDCCS(14)
#define	UDCCS15			UDCCS(15)
#define	UFNHR			UDC_pointer->ufnhr
#define	UFNLR			UDC_pointer->ufnlr
#define	UBCR2			UDC_pointer->ubcr2
#define	UBCR4			UDC_pointer->ubcr4
#define	UBCR7			UDC_pointer->ubcr7
#define	UBCR9			UDC_pointer->ubcr9
#define	UBCR12			UDC_pointer->ubcr12
#define	UBCR14			UDC_pointer->ubcr14
#define	UDDR0			UDC_pointer->uddr0
#define	UDDR1			UDC_pointer->uddr1
#define	UDDR2			UDC_pointer->uddr2
#define	UDDR3			UDC_pointer->uddr3
#define	UDDR4			UDC_pointer->uddr4
#define	UDDR5			UDC_pointer->uddr5
#define	UDDR6			UDC_pointer->uddr6
#define	UDDR7			UDC_pointer->uddr7
#define	UDDR8			UDC_pointer->uddr8
#define	UDDR9			UDC_pointer->uddr9
#define	UDDR10			UDC_pointer->uddr10
#define	UDDR11			UDC_pointer->uddr11
#define	UDDR12			UDC_pointer->uddr12
#define	UDDR13			UDC_pointer->uddr13
#define	UDDR14			UDC_pointer->uddr14
#define	UDDR15			UDC_pointer->uddr15
#define	UICR0			UDC_pointer->uicr0
#define	UICR1			UDC_pointer->uicr1
#define	USIR0			UDC_pointer->usir0
#define	USIR1			UDC_pointer->usir1
#endif /* LANGUAGE == C */

#define	UDCCR_OFFSET		0x000
#if !defined(PXA2X0_NOPXA255)
#define	UDCCFR_OFFSET		0x008
#endif /* PXA255 and above only */
#define	UDCCS_OFFSET(i)		(0x010 + ((i) << 2))
#define	UDCCS0_OFFSET		UDCCS_OFFSET(0)
#define	UDCCS1_OFFSET		UDCCS_OFFSET(1)
#define	UDCCS2_OFFSET		UDCCS_OFFSET(2)
#define	UDCCS3_OFFSET		UDCCS_OFFSET(3)
#define	UDCCS4_OFFSET		UDCCS_OFFSET(4)
#define	UDCCS5_OFFSET		UDCCS_OFFSET(5)
#define	UDCCS6_OFFSET		UDCCS_OFFSET(6)
#define	UDCCS7_OFFSET		UDCCS_OFFSET(7)
#define	UDCCS8_OFFSET		UDCCS_OFFSET(8)
#define	UDCCS9_OFFSET		UDCCS_OFFSET(9)
#define	UDCCS10_OFFSET		UDCCS_OFFSET(10)
#define	UDCCS11_OFFSET		UDCCS_OFFSET(11)
#define	UDCCS12_OFFSET		UDCCS_OFFSET(12)
#define	UDCCS13_OFFSET		UDCCS_OFFSET(13)
#define	UDCCS14_OFFSET		UDCCS_OFFSET(14)
#define	UDCCS15_OFFSET		UDCCS_OFFSET(15)
#define	UFNHR_OFFSET		0x060
#define	UFNLR_OFFSET		0x064
#define	UBCR2_OFFSET		0x068
#define	UBCR4_OFFSET		0x06C
#define	UBCR7_OFFSET		0x070
#define	UBCR9_OFFSET		0x074
#define	UBCR12_OFFSET		0x078
#define	UBCR14_OFFSET		0x07C
#define	UDDR0_OFFSET		0x080
#define	UDDR1_OFFSET		0x100
#define	UDDR2_OFFSET		0x180
#define	UDDR3_OFFSET		0x200
#define	UDDR4_OFFSET		0x400
#define	UDDR5_OFFSET		0x0A0
#define	UDDR6_OFFSET		0x600
#define	UDDR7_OFFSET		0x680
#define	UDDR8_OFFSET		0x700
#define	UDDR9_OFFSET		0x900
#define	UDDR10_OFFSET		0x0C0
#define	UDDR11_OFFSET		0xB00
#define	UDDR12_OFFSET		0xB80
#define	UDDR13_OFFSET		0xC00
#define	UDDR14_OFFSET		0xE00
#define	UDDR15_OFFSET		0x0E0
#define	UICR0_OFFSET		0x050
#define	UICR1_OFFSET		0x054
#define	USIR0_OFFSET		0x058
#define	USIR1_OFFSET		0x05C

/* UDCCR bits - see Table 12-20 in [1], Table 12-12 in [2], Table 12-12 in [3] */

#define	UDCCR_REM		bit(7)
#define	UDCCR_RSTIR		bit(6)
#define	UDCCR_SRM		bit(5)
#define	UDCCR_SUSIR		bit(4)
#define	UDCCR_RESIR		bit(3)
#define	UDCCR_RSM		bit(2)
#define	UDCCR_UDA		bit(1)
#define	UDCCR_UDE		bit(0)

#if !defined(PXA2X0_NOPXA255)
/* UDCCFR bits - see Table 12-13 in [3] */
#define	UDCCFR_AREN		bit(7)
#define	UDCCFR_ACM		bit(2)
#endif /* PXA255 and above only */

/* UDCCS0 bits - see Table 12-21 in [1], Table 12-13 in [2], Table 12-14 in [3] */

#define	UDCCS0_SA		bit(7)
#define	UDCCS0_RNE		bit(6)
#define	UDCCS0_FST		bit(5)
#define	UDCCS0_SST		bit(4)
#define	UDCCS0_DRWF		bit(3)
#define	UDCCS0_FTF		bit(2)
#define	UDCCS0_IPR		bit(1)
#define	UDCCS0_OPR		bit(0)

/* UDCCS1 bits - see Table 12-22 in [1], Table 12-14 in [2], Table 12-15 in [3] */

#define	UDCCS1_TSP		bit(7)
#define	UDCCS1_FST		bit(5)
#define	UDCCS1_SST		bit(4)
#define	UDCCS1_TUR		bit(3)
#define	UDCCS1_FTF		bit(2)
#define	UDCCS1_TPC		bit(1)
#define	UDCCS1_TFS		bit(0)

/* UDCCS2 bits - see Table 12-23 in [1], Table 12-15 in [2], Table 12-16 in [3] */

#define	UDCCS2_RSP		bit(7)
#define	UDCCS2_RNE		bit(6)
#define	UDCCS2_FST		bit(5)
#define	UDCCS2_SST		bit(4)
#define	UDCCS2_DME		bit(3)
#define	UDCCS2_RPC		bit(1)
#define	UDCCS2_RFS		bit(0)

/* UDCCS3 bits - see Table 12-24 in [1], Table 12-16 in [2], Table 12-17 in [3] */

#define	UDCCS3_TSP		bit(7)
#define	UDCCS3_TUR		bit(3)
#define	UDCCS3_FTF		bit(2)
#define	UDCCS3_TPC		bit(1)
#define	UDCCS3_TFS		bit(0)

/* UDCCS4 bits - see Table 12-25 in [1], Table 12-17 in [2], Table 12-18 in [3] */

#define	UDCCS4_RSP		bit(7)
#define	UDCCS4_RNE		bit(6)
#define	UDCCS4_DME		bit(3)
#define	UDCCS4_ROF		bit(2)
#define	UDCCS4_RPC		bit(1)
#define	UDCCS4_RFS		bit(0)

/* UDCCS5 bits - see Table 12-26 in [1], Table 12-18 in [2], Table 12-19 in [3] */

#define	UDCCS5_TSP		bit(7)
#define	UDCCS5_FST		bit(5)
#define	UDCCS5_SST		bit(4)
#define	UDCCS5_TUR		bit(3)
#define	UDCCS5_FTF		bit(2)
#define	UDCCS5_TPC		bit(1)
#define	UDCCS5_TFS		bit(0)

/* UDCCS6 bits - see Table 12-22 in [1], Table 12-14 in [2], Table 12-15 in [3] */

#define	UDCCS6_TSP		bit(7)
#define	UDCCS6_FST		bit(5)
#define	UDCCS6_SST		bit(4)
#define	UDCCS6_TUR		bit(3)
#define	UDCCS6_FTF		bit(2)
#define	UDCCS6_TPC		bit(1)
#define	UDCCS6_TFS		bit(0)

/* UDCCS7 bits - see Table 12-23 in [1], Table 12-15 in [2], Table 12-16 in [3] */

#define	UDCCS7_RSP		bit(7)
#define	UDCCS7_RNE		bit(6)
#define	UDCCS7_FST		bit(5)
#define	UDCCS7_SST		bit(4)
#define	UDCCS7_DME		bit(3)
#define	UDCCS7_RPC		bit(1)
#define	UDCCS7_RFS		bit(0)

/* UDCCS8 bits - see Table 12-24 in [1], Table 12-16 in [2], Table 12-17 in [3] */

#define	UDCCS8_TSP		bit(7)
#define	UDCCS8_TUR		bit(3)
#define	UDCCS8_FTF		bit(2)
#define	UDCCS8_TPC		bit(1)
#define	UDCCS8_TFS		bit(0)

/* UDCCS9 bits - see Table 12-25 in [1], Table 12-17 in [2], Table 12-18 in [3] */

#define	UDCCS9_RSP		bit(7)
#define	UDCCS9_RNE		bit(6)
#define	UDCCS9_DME		bit(3)
#define	UDCCS9_ROF		bit(2)
#define	UDCCS9_RPC		bit(1)
#define	UDCCS9_RFS		bit(0)

/* UDCCS10 bits - see Table 12-26 in [1], Table 12-18 in [2], Table 12-19 in [3] */

#define	UDCCS10_TSP		bit(7)
#define	UDCCS10_FST		bit(5)
#define	UDCCS10_SST		bit(4)
#define	UDCCS10_TUR		bit(3)
#define	UDCCS10_FTF		bit(2)
#define	UDCCS10_TPC		bit(1)
#define	UDCCS10_TFS		bit(0)

/* UDCCS11 bits - see Table 12-22 in [1], Table 12-14 in [2], Table 12-15 in [3] */

#define	UDCCS11_TSP		bit(7)
#define	UDCCS11_FST		bit(5)
#define	UDCCS11_SST		bit(4)
#define	UDCCS11_TUR		bit(3)
#define	UDCCS11_FTF		bit(2)
#define	UDCCS11_TPC		bit(1)
#define	UDCCS11_TFS		bit(0)

/* UDCCS12 bits - see Table 12-23 in [1], Table 12-15 in [2], Table 12-16 in [3] */

#define	UDCCS12_RSP		bit(7)
#define	UDCCS12_RNE		bit(6)
#define	UDCCS12_FST		bit(5)
#define	UDCCS12_SST		bit(4)
#define	UDCCS12_DME		bit(3)
#define	UDCCS12_RPC		bit(1)
#define	UDCCS12_RFS		bit(0)

/* UDCCS13 bits - see Table 12-24 in [1], Table 12-16 in [2], Table 12-17 in [3] */

#define	UDCCS13_TSP		bit(7)
#define	UDCCS13_TUR		bit(3)
#define	UDCCS13_FTF		bit(2)
#define	UDCCS13_TPC		bit(1)
#define	UDCCS13_TFS		bit(0)

/* UDCCS14 bits - see Table 12-25 in [1], Table 12-17 in [2], Table 12-18 in [3] */

#define	UDCCS14_RSP		bit(7)
#define	UDCCS14_RNE		bit(6)
#define	UDCCS14_DME		bit(3)
#define	UDCCS14_ROF		bit(2)
#define	UDCCS14_RPC		bit(1)
#define	UDCCS14_RFS		bit(0)

/* UDCCS15 bits - see Table 12-26 in [1], Table 12-18 in [2], Table 12-19 in [3] */

#define	UDCCS15_TSP		bit(7)
#define	UDCCS15_FST		bit(5)
#define	UDCCS15_SST		bit(4)
#define	UDCCS15_TUR		bit(3)
#define	UDCCS15_FTF		bit(2)
#define	UDCCS15_TPC		bit(1)
#define	UDCCS15_TFS		bit(0)

/* UICR0 bits - see Table 12-27 in [1], Table 12-19 in [2], Table 12-20 in [3] */

#define	UICR0_IM7		bit(7)
#define	UICR0_IM6		bit(6)
#define	UICR0_IM5		bit(5)
#define	UICR0_IM4		bit(4)
#define	UICR0_IM3		bit(3)
#define	UICR0_IM2		bit(2)
#define	UICR0_IM1		bit(1)
#define	UICR0_IM0		bit(0)

/* UICR1 bits - see Table 12-28 in [1], Table 12-20 in [2], Table 12-21 in [3] */

#define	UICR1_IM15		bit(7)
#define	UICR1_IM14		bit(6)
#define	UICR1_IM13		bit(5)
#define	UICR1_IM12		bit(4)
#define	UICR1_IM11		bit(3)
#define	UICR1_IM10		bit(2)
#define	UICR1_IM9		bit(1)
#define	UICR1_IM8		bit(0)

/* USIR0 bits - see Table 12-29 in [1], Table 12-21 in [2], Table 12-22 in [3] */

#define	USIR0_IR7		bit(7)
#define	USIR0_IR6		bit(6)
#define	USIR0_IR5		bit(5)
#define	USIR0_IR4		bit(4)
#define	USIR0_IR3		bit(3)
#define	USIR0_IR2		bit(2)
#define	USIR0_IR1		bit(1)
#define	USIR0_IR0		bit(0)

/* USIR1 bits - see Table 12-30 in [1], Table 12-22 in [2], Table 12-23 in [3] */

#define	USIR1_IR15		bit(7)
#define	USIR1_IR14		bit(6)
#define	USIR1_IR13		bit(5)
#define	USIR1_IR12		bit(4)
#define	USIR1_IR11		bit(3)
#define	USIR1_IR10		bit(2)
#define	USIR1_IR9		bit(1)
#define	USIR1_IR8		bit(0)

/* UFNHR bits - see Table 12-31 in [1], Table 12-23 in [2], Table 12-24 in [3] */

#define	UFNHR_SIR		bit(7)
#define	UFNHR_SIM		bit(6)
#define	UFNHR_IPE14		bit(5)
#define	UFNHR_IPE9		bit(4)
#define	UFNHR_IPE4		bit(3)
#define	UFNHR_FNMSB_MASK	bits(2,0)
#define	UFNHR_FNMSB(x)		bits_val(2,0,x)
#define	get_UFNHR_FNMSB(x)	bits_get(2,0,x)

/* UFNLR bits - see Table 12-32 in [1], Table 12-24 in [2], Table 12-25 in [3] */

#define	UNFLR_FNLSB_MASK	bits(7,0)
#define	UFNLR_FNLSB(x)		bits_val(7,0,x)
#define	get_UFNLR_FNLSB(x)	bits_get(7,0,x)

/* UBCRx bits - see Table 12-33 in [1], Table 12-25 in [2], Table 12-26 in [3] */

#define	UBCR_BC_MASK		bits(7,0)
#define	UBCR_BC(x)		bits_val(7,0,x)
#define	get_UBCR_BC(x)		bits_get(7,0,x)

/* UDDRx bits - see 12.6.15 - 12.6.20 in [1], 12.6.15 - 12.6.20 in [2], 12.6.16 - 12.6.21 in [3] */

#define	UDDR_DATA_MASK		bits(7,0)
#define	UDDR_DATA(x)		bits_val(7,0,x)
#define	get_UDDR_DATA(x)	bits_get(7,0,x)

#endif /* PXA2X0_UDC_H */
