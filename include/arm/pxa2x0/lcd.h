/*
 * $Id$
 *
 * XScale PXA250/PXA210 LCD Controller Registers
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
 *
 */

#ifndef	PXA2X0_LCD_H
#define	PXA2X0_LCD_H

#include <common.h>

#if LANGUAGE == C
#include <stdint.h>
#endif

/* LCD Controller Registers */

#define	LCD_BASE	0x44000000

#if LANGUAGE == C
typedef volatile struct LCD_registers {
	uint32_t lccr[4];
	uint32_t __reserved1[4];
	uint32_t fbr0;
	uint32_t fbr1;
	uint32_t __reserved2[4];
	uint32_t lcsr;
	uint32_t liidr;
	uint32_t trgbr;
	uint32_t tcr;
	uint32_t __reserved3[110];
	uint32_t fdadr0;
	uint32_t fsadr0;
	uint32_t fidr0;
	uint32_t ldcmd0;
	uint32_t fdadr1;
	uint32_t fsadr1;
	uint32_t fidr1;
	uint32_t ldcmd1;
} LCD_registers_t;

#ifdef PXA2X0_UNMAPPED
#define	LCD_pointer	((LCD_registers_t*) LCD_BASE)
#endif

#define	LCCR(i)		LCD_pointer->lccr[i]
#define	LCCR0		LCCR(0)
#define	LCCR1		LCCR(1)
#define	LCCR2		LCCR(2)
#define	LCCR3		LCCR(3)
#define	FDADR0		LCD_pointer->fdadr0
#define	FSADR0		LCD_pointer->fsadr0
#define	FIDR0		LCD_pointer->fidr0
#define	LDCMD0		LCD_pointer->ldcmd0
#define	FDADR1		LCD_pointer->fdadr1
#define	FSADR1		LCD_pointer->fsadr1
#define	FIDR1		LCD_pointer->fidr1
#define	LDCMD1		LCD_pointer->ldcmd1
#define	FBR0		LCD_pointer->fbr0
#define	FBR1		LCD_pointer->fbr1
#define	LCSR		LCD_pointer->lcsr
#define	LIIDR		LCD_pointer->liidr
#define	TRGBR		LCD_pointer->trgbr
#define	TCR		LCD_pointer->tcr
#endif /* LANGUAGE == C */

#define	LCCR0_OFFSET	0x000
#define	LCCR1_OFFSET	0x004
#define	LCCR2_OFFSET	0x008
#define	LCCR3_OFFSET	0x00C
#define	FBR0_OFFSET	0x020
#define	FBR1_OFFSET	0x024
#define	LCSR_OFFSET	0x038
#define	LIIDR_OFFSET	0x03C
#define	TRGBR_OFFSET	0x040
#define	TCR_OFFSET	0x044
#define	FDADR0_OFFSET	0x200
#define	FSADR0_OFFSET	0x204
#define	FIDR0_OFFSET	0x208
#define	LDCMD0_OFFSET	0x20C
#define	FDADR1_OFFSET	0x210
#define	FSADR1_OFFSET	0x214
#define	FIDR1_OFFSET	0x218
#define	LDCMD1_OFFSET	0x21C

/* LCCR0 bits - see Table 7-2 in [1] */

#define	LCCR0_OUM	bit(21)
#define	LCCR0_BM	bit(20)
#define	LCCR0_PDD_MASK	bits(19,12)
#define	LCCR0_PDD(x)	bits_val(19,12,x)
#define	LCCR0_QDM	bit(11)
#define	LCCR0_DIS	bit(10)
#define	LCCR0_DPD	bit(9)
#define	LCCR0_PAS	bit(7)
#define	LCCR0_EFM	bit(6)
#define	LCCR0_IUM	bit(5)
#define	LCCR0_SFM	bit(4)
#define	LCCR0_LDM	bit(3)
#define	LCCR0_SDS	bit(2)
#define	LCCR0_CMS	bit(1)
#define	LCCR0_ENB	bit(0)

/* LCCR1 bits - see Table 7-5 in [1] */

#define	LCCR1_BLW_MASK	bits(31,24)
#define	LCCR1_BLW(x)	bits_val(31,24,x)
#define	LCCR1_ELW_MASK	bits(23,16)
#define	LCCR1_ELW(x)	bits_val(23,16,x)
#define	LCCR1_HSW_MASK	bits(15,10)
#define	LCCR1_HSW(x)	bits_val(15,10,x)
#define	LCCR1_PPL_MASK	bits(9,0)
#define	LCCR1_PPL(x)	bits_val(9,0,x)

/* LCCR2 bits - see Table 7-6 in [1] */

#define	LCCR2_BFW_MASK	bits(31,24)
#define	LCCR2_BFW(x)	bits_val(31,24,x)
#define	LCCR2_EFW_MASK	bits(23,16)
#define	LCCR2_EFW(x)	bits_val(23,16,x)
#define	LCCR2_VSW_MASK	bits(15,10)
#define	LCCR2_VSW(x)	bits_val(15,10,x)
#define	LCCR2_LPP_MASK	bits(9,0)
#define	LCCR2_LPP(x)	bits_val(9,0,x)

/* LCCR3 bits - see Table 7-7 in [1] */

#define	LCCR3_DPC	bit(27)
#define	LCCR3_BPP_MASK	bits(26,24)
#define	LCCR3_BPP(x)	bits_val(26,24,x)
#define	LCCR3_OEP	bit(23)
#define	LCCR3_PCP	bit(22)
#define	LCCR3_HSP	bit(21)
#define	LCCR3_VSP	bit(20)
#define	LCCR3_API_MASK	bits(19,16)
#define	LCCR3_API(x)	bits_val(19,16,x)
#define	LCCR3_ACB_MASK	bits(15,8)
#define	LCCR3_ACB(x)	bits_val(15,8,x)
#define	LCCR3_PCD_MASK	bits(7,0)
#define	LCCR3_PCD(x)	bits_val(7,0,x)

/* FBR0 bits - see Table 7-12 in [1] */

#define	FBR0_BINT	bit(1)
#define	FBR0_BRA	bit(0)

/* FBR1 bits - see Table 7-12 in [1] */

#define	FBR1_BINT	bit(1)
#define	FBR1_BRA	bit(0)

/* LCSR bits - see Table 7-13 in [1] */

#define	LCSR_SINT	bit(10)
#define	LCSR_BS		bit(9)
#define	LCSR_EOF	bit(8)
#define	LCSR_QD		bit(7)
#define	LCSR_OU		bit(6)
#define	LCSR_IUU	bit(5)
#define	LCSR_IUL	bit(4)
#define	LCSR_ABC	bit(3)
#define	LCSR_BER	bit(2)
#define	LCSR_SOF	bit(1)
#define	LCSR_LDD	bit(0)

/* LIIDR bits - see Table 7-14 in [1] */

#define	LIIDR_IFRAMEID_MASK	bits(31,3)
#define	LIIDR_IFRAMEID(x)	bits_val(31,3,x)

/* TRGBR bits - see Table 7-15 in [1] */

#define	TRGBR_TBS_MASK	bits(23,16)
#define	TRGBR_TBS(x)	bits_val(23,16,x)
#define	TRGBR_TGS_MASK	bits(15,8)
#define	TRGBR_TGS(x)	bits_val(15,8,x)
#define	TRGBR_TRS_MASK	bits(7,0)
#define	TRGBR_TRS(x)	bits_val(7,0,x)

/* TCR bits - see Table 7-16 in [1] */

#define	TCR_TED		bit(14)
#define	TCR_THBS_MASK	bits(11,8)
#define	TCR_THBS(x)	bits_val(11,8,x)
#define	TCR_TVBS_MASK	bits(7,4)
#define	TCR_TVBS(x)	bits(7,4,x)
#define	TCR_FNAME	bit(3)
#define	TCR_COAE	bit(2)
#define	TCR_FNAM	bit(1)
#define	TCR_COAM	bit(0)

/* LDCMD0 bits - see Table 7-11 in [1] */

#define	LDCMD0_PAL	bit(26)
#define	LDCMD0_SOFINT	bit(22)
#define	LDCMD0_EOFINT	bit(21)
#define	LDCMD0_LEN_MASK	bits(20,0)
#define	LDCMD0_LEN(x)	bits_val(20,0,x)

/* LDCMD1 bits - see Table 7-11 in [1] */

#define	LDCMD1_PAL	bit(26)
#define	LDCMD1_SOFINT	bit(22)
#define	LDCMD1_EOFINT	bit(21)
#define	LDCMD1_LEN_MASK	bits(20,0)
#define	LDCMD1_LEN(x)	bits_val(20,0,x)

#endif /* PXA2X0_LCD_H */
