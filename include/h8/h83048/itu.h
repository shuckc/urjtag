/*
 * $Id$
 *
 * H8/3048 ITU Registers
 * Copyright (C) 2005 Elcom s.r.o.
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
 * 3. Neither the name of the copyright holders nor the names of its contributors
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
 * Written by Branislav Petrovsky <brano111@szm.sk>, 2005.
 *
 * Documentation:
 * [1] Renesas Technology Corp., "Hitachi Single-Chip Microcomputer
 *     H8/3048 Series, H8/3048F-ZTAT Hardware Manual",
 *     Rev. 6.0, 9/3/2002, Order Number: ADE-602-073E
 *
 */

#ifndef H83048_ITU_H
#define H83048_ITU_H

#include <openwince.h>

#ifndef __ASSEMBLY__
#include <stdint.h>
#endif

/* ITU registers */

#define ITU_COMMON1_BASE	0xffff60
#define ITU_COMMON2_BASE	0xffff90
#define ITU0_BASE		0xffff64
#define ITU1_BASE		0xffff6e
#define ITU2_BASE		0xffff78
#define ITU3_BASE		0xffff82
#define ITU4_BASE		0xffff92

#ifndef __ASSEMBLY__
typedef volatile struct ITU_registers {
	uint8_t tcr;
	uint8_t tior;
	uint8_t tier;
	uint8_t tsr;
	uint16_t tcnt;
	uint16_t gra;
	uint16_t grb;
	uint16_t bra;	/* only ITU channel 3 and 4 */
	uint16_t brb;	/* only ITU channel 3 and 4 */
} ITU_registers_t;

typedef volatile struct ITU_common1_registers {
	uint8_t tstr;
	uint8_t tsnc;
	uint8_t tmdr;
	uint8_t tfcr;
} ITU_common1_registers_t;

typedef volatile struct ITU_common2_registers {
	uint8_t toer;
	uint8_t tocr;
} ITU_common2_registers_t;

#define ITU_COMMON1_pointer	((ITU_common1_registers_t*) ITU_COMMON1_BASE)
#define ITU_COMMON2_pointer	((ITU_common2_registers_t*) ITU_COMMON2_BASE)
#define ITU0_pointer		((ITU_registers_t*) ITU0_BASE)
#define ITU1_pointer		((ITU_registers_t*) ITU1_BASE)
#define ITU2_pointer		((ITU_registers_t*) ITU2_BASE)
#define ITU3_pointer		((ITU_registers_t*) ITU3_BASE)
#define ITU4_pointer		((ITU_registers_t*) ITU4_BASE)

#define TCR0		ITU0_pointer->tcr
#define TIOR0		ITU0_pointer->tior
#define TIER0		ITU0_pointer->tier
#define TSR0		ITU0_pointer->tsr
#define TCNT0		ITU0_pointer->tcnt
#define GRA0		ITU0_pointer->gra
#define GRB0		ITU0_pointer->grb

#define TCR1		ITU1_pointer->tcr
#define TIOR1		ITU1_pointer->tior
#define TIER1		ITU1_pointer->tier
#define TSR1		ITU1_pointer->tsr
#define TCNT1		ITU1_pointer->tcnt
#define GRA1		ITU1_pointer->gra
#define GRB1		ITU1_pointer->grb

#define TCR2		ITU2_pointer->tcr
#define TIOR2		ITU2_pointer->tior
#define TIER2		ITU2_pointer->tier
#define TSR2		ITU2_pointer->tsr
#define TCNT2		ITU2_pointer->tcnt
#define GRA2		ITU2_pointer->gra
#define GRB2		ITU2_pointer->grb

#define TCR3		ITU3_pointer->tcr
#define TIOR3		ITU3_pointer->tior
#define TIER3		ITU3_pointer->tier
#define TSR3		ITU3_pointer->tsr
#define TCNT3		ITU3_pointer->tcnt
#define GRA3		ITU3_pointer->gra
#define GRB3		ITU3_pointer->grb
#define BRA3		ITU3_pointer->bra
#define BRB3		ITU3_pointer->brb

#define TCR4		ITU4_pointer->tcr
#define TIOR4		ITU4_pointer->tior
#define TIER4		ITU4_pointer->tier
#define TSR4		ITU4_pointer->tsr
#define TCNT4		ITU4_pointer->tcnt
#define GRA4		ITU4_pointer->gra
#define GRB4		ITU4_pointer->grb
#define BRA4		ITU4_pointer->bra
#define BRB4		ITU4_pointer->brb

#define TSTR		ITU_COMMON1_pointer->tstr
#define TSNC		ITU_COMMON1_pointer->tsnc
#define TMDR		ITU_COMMON1_pointer->tmdr
#define TFCR		ITU_COMMON1_pointer->tfcr

#define TOER		ITU_COMMON2_pointer->toer
#define TOCR		ITU_COMMON2_pointer->tocr
#endif /* __ASSEMBLY__ */

#define TCR_OFFSET	0x00
#define TIOR_OFFSET	0x01
#define TIER_OFFSET	0x02
#define TSR_OFFSET	0x03
#define TCNT_OFFSET	0x04
#define GRA_OFFSET	0x06
#define GRB_OFFSET	0x08
#define BRA_OFFSET	0x0a
#define BRB_OFFSET	0x0c

#define TSTR_OFFSET	0x00
#define TSNC_OFFSET	0x01
#define TMDR_OFFSET	0x02
#define TFCR_OFFSET	0x03

#define TOER_OFFSET	0x00
#define TOCR_OFFSET	0x01

/* TCR bits */
#define TCR_CCLR_MASK		bits(6,5)
#define TCR_CCLR(x)		bits_val(6,5,x)
#define get_TCR_CCLR(x)		bits_get(6,5,x)
#define TCR_CKEG_MASK		bits(4,3)
#define TCR_CKEG(x)		bits_val(4,3,x)
#define get_TCR_CKEG(x)		bits_get(4,3,x)
#define TCR_TPSC_MASK		bits(2,0)
#define TCR_TPSC(x)		bits_val(2,0,x)
#define get_TCR_TPSC(x)		bits_get(2,0,x)

/* TIOR bits */
#define TIOR_IOB_MASK		bits(6,4)
#define TIOR_IOB(x)		bits_val(6,4,x)
#define get_TIOR_IOB(x)		bits_get(6,4,x)
#define TIOR_IOA_MASK		bits(2,0)
#define TIOR_IOA(x)		bits_val(2,0,x)
#define get_TIOR_IOA(x)		bits_get(2,0,x)

/* TIER bits */
#define TIER_OVIE		bit(2)
#define TIER_IMIEB		bit(1)
#define TIER_IMIEA		bit(0)

/* TSR bits */
#define TSR_OVF			bit(2)
#define TSR_IMFB		bit(1)
#define TSR_IMFA		bit(0)

/* TSTR bits */
#define TSTR_STR4		bit(4)
#define TSTR_STR3		bit(3)
#define TSTR_STR2		bit(2)
#define TSTR_STR1		bit(1)
#define TSTR_STR0		bit(0)

/* TSNC bits */
#define TSNC_SYNC4		bit(4)
#define TSNC_SYNC3		bit(3)
#define TSNC_SYNC2		bit(2)
#define TSNC_SYNC1		bit(1)
#define TSNC_SYNC0		bit(0)

/* TMDR bits */
#define TMDR_MDF		bit(6)
#define TMDR_FDIR		bit(5)
#define TMDR_PWM4		bit(4)
#define TMDR_PWM3		bit(3)
#define TMDR_PWM2		bit(2)
#define TMDR_PWM1		bit(1)
#define TMDR_PWM0		bit(0)

/* TFCR bits */
#define TFCR_CMD_MASK		bits(5,4)
#define TFCR_CMD(x)		bits_val(5,4,x)
#define get_TFCR_CMD(x)		bits_get(5,4,x)
#define TFCR_BFB4		bit(3)
#define TFCR_BFA4		bit(2)
#define TFCR_BFB3		bit(1)
#define TFCR_BFA3		bit(0)

/* TOER bits */
#define TOER_EXB4		bit(5)
#define TOER_EXA4		bit(4)
#define TOER_EB3		bit(3)
#define TOER_EB4		bit(2)
#define TOER_EA4		bit(1)
#define TOER_EA3		bit(0)

/* TOCR bits */
#define TOCR_XTGD		bit(4)
#define TOCR_OLS4		bit(1)
#define TOCR_OLS3		bit(0)

#endif /* H83048_ITU_H */
