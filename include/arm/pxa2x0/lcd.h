/*
 * $Id$
 *
 * XScale PXA250/PXA210 LCD Controller Registers
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
} LCD_registers;

#ifdef PXA2X0_UNMAPPED
#define	LCD_pointer	((LCD_registers*) LCD_BASE)
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
#define	LCCR0_PDD_MASK	0x000FF000
#define	LCCR0_PDD(x)	((x << 12) & LCCR0_PDD_MASK)
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

#define	LCCR1_BLW_MASK	0xFF000000
#define	LCCR1_BLW(x)	((x << 24) & LCCR1_BLW_MASK)
#define	LCCR1_ELW_MASK	0x00FF0000
#define	LCCR1_ELW(x)	((x << 16) & LCCR1_ELW_MASK)
#define	LCCR1_HSW_MASK	0x0000FC00
#define	LCCR1_HSW(x)	((x << 10) & LCCR1_HSW_MASK)
#define	LCCR1_PPL_MASK	0x000003FF
#define	LCCR1_PPL(x)	(x & LCCR1_PPL_MASK)

/* LCCR2 bits - see Table 7-6 in [1] */

#define	LCCR2_BFW_MASK	0xFF000000
#define	LCCR2_BFW(x)	((x << 24) & LCCR2_BFW_MASK)
#define	LCCR2_EFW_MASK	0x00FF0000
#define	LCCR2_EFW(x)	((x << 16) & LCCR2_EFW_MASK)
#define	LCCR2_VSW_MASK	0x0000FC00
#define	LCCR2_VSW(x)	((x << 10) & LCCR2_VSW_MASK)
#define	LCCR2_LPP_MASK	0x000003FF
#define	LCCR2_LPP(x)	(x & LCCR2_LPP_MASK)

/* LCCR3 bits - see Table 7-7 in [1] */

#define	LCCR3_DPC	bit(27)
#define	LCCR3_BPP_MASK	0x07000000
#define	LCCR3_BPP(x)	((x << 24) & LCCR3_BPP_MASK)
#define	LCCR3_OEP	bit(23)
#define	LCCR3_PCP	bit(22)
#define	LCCR3_HSP	bit(21)
#define	LCCR3_VSP	bit(20)
#define	LCCR3_API_MASK	0x000F0000
#define	LCCR3_API(x)	((x << 16) & LCCR3_API_MASK)
#define	LCCR3_ACB_MASK	0x0000FF00
#define	LCCR3_ACB(x)	((x << 8) & LCCR3_ACB_MASK)
#define	LCCR3_PCD_MASK	0x000000FF
#define	LCCR3_PCD(x)	(x & LCCR3_PD_MASK)

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

#define	LIIDR_IFRAMEID_MASK	0xFFFFFF80
#define	LIIDR_IFRAMEID(x)	((x << 3) & LIIDR_IFRAMEID_MASK)

/* TRGBR bits - see Table 7-15 in [1] */

#define	TRGBR_TBS_MASK	0x00FF0000
#define	TRGBR_TBS(x)	((x << 16) & TRGBR_TBS_MASK)
#define	TRGBR_TGS_MASK	0x0000FF00
#define	TRGBR_TGS(x)	((x << 8) & TRGBR_TGS_MASK)
#define	TRGBR_TRS_MASK	0x000000FF
#define	TRGBR_TRS(x)	(x & TRGBR_TRS_MASK)

/* TCR bits - see Table 7-16 in [1] */

#define	TCR_TED		bit(14)
#define	TCR_THBS_MASK	0x00000F00
#define	TCR_THBS(x)	((x << 8) & TCR_THBS_MASK)
#define	TCR_TVBS_MASK	0x000000F0
#define	TCR_TVBS(x)	((x << 4) & TCR_TVBS_MASK)
#define	TCR_FNAME	bit(3)
#define	TCR_COAE	bit(2)
#define	TCR_FNAM	bit(1)
#define	TCR_COAM	bit(0)

/* LDCMD0 bits - see Table 7-11 in [1] */

#define	LDCMD0_PAL	bit(26)
#define	LDCMD0_SOFINT	bit(22)
#define	LDCMD0_EOFINT	bit(21)
#define	LDCMD0_LEN_MASK	0x008FFFFF
#define	LDCMD0_LEN(x)	(x & LDCMD0_LEN_MASK)

/* LDCMD1 bits - see Table 7-11 in [1] */

#define	LDCMD1_PAL	bit(26)
#define	LDCMD1_SOFINT	bit(22)
#define	LDCMD1_EOFINT	bit(21)
#define	LDCMD1_LEN_MASK	0x008FFFFF
#define	LDCMD1_LEN(x)	(x & LDCMD1_LEN_MASK)

#endif /* PXA2X0_LCD_H */
