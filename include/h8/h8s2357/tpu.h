/*
 * $Id$
 *
 * H8S/2357 TPU0 to TPU5 Registers
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
 * [1] Renesas Technology Corp., "Hitachi 16-Bit Single-chip Microcomputer
 *     H8S/2357 Series, H8S/2357F-ZTAT, H8S/2398F-ZTAT Hardware Manual",
 *     Rev. 5.0, 11/22/02, Order Number: ADE-602-146D
 *
 */

#ifndef H8S2357_TPU_H
#define H8S2357_TPU_H

#include <common.h>

#if LANGUAGE == C
#include <stdint.h>
#endif

/* TPU registers */

#define TPU0_BASE		0xffffffd0
#define TPU1_BASE		0xffffffe0
#define TPU2_BASE		0xfffffff0
#define TPU3_BASE		0xfffffe80
#define TPU4_BASE		0xfffffe90
#define TPU5_BASE		0xfffffea0
#define TPU_COMMON_BASE		0xffffffc0

#if LANGUAGE == C
typedef struct TPU_registers {
	uint8_t tcr;
	uint8_t tmdr;
	uint8_t tiorh;	/* tior in TPU1, TPU2, TPU4, TPU5 */
	uint8_t tiorl;	/* only for TPU0 and TPU3 */
	uint8_t tier;
	uint8_t tsr;
	uint16_t tcnt;
	uint16_t tgra;
	uint16_t tgrb;
	uint16_t tgrc;	/* only for TPU0 and TPU3 */
	uint16_t tgrd;	/* only for TPU0 and TPU3 */
} TPU_registers_t;

typedef struct TPU_common_registers {
	uint8_t tstr;
	uint8_t tsyr;
} TPU_common_registers_t;

#define TPU0_pointer		((TPU_registers_t*) TPU0_BASE)
#define TPU1_pointer		((TPU_registers_t*) TPU1_BASE)
#define TPU2_pointer		((TPU_registers_t*) TPU2_BASE)
#define TPU3_pointer		((TPU_registers_t*) TPU3_BASE)
#define TPU4_pointer		((TPU_registers_t*) TPU4_BASE)
#define TPU5_pointer		((TPU_registers_t*) TPU5_BASE)
#define TPU_COMMON_pointer	((TPU_common_registers_t*) TPU_COMMON_BASE)

#define TCR0		TPU0_pointer->tcr
#define TMDR0		TPU0_pointer->tmdr
#define TIOR0H		TPU0_pointer->tiorh
#define TIOR0L		TPU0_pointer->tiorl
#define TIER0		TPU0_pointer->tier
#define TSR0		TPU0_pointer->tsr
#define TCNT0		TPU0_pointer->tcnt
#define TGR0A		TPU0_pointer->tgra
#define TGR0B		TPU0_pointer->tgrb
#define TGR0C		TPU0_pointer->tgrc
#define TGR0D		TPU0_pointer->tgrd

#define TCR1		TPU1_pointer->tcr
#define TMDR1		TPU1_pointer->tmdr
#define TIOR1		TPU1_pointer->tiorh
#define TIER1		TPU1_pointer->tier
#define TSR1		TPU1_pointer->tsr
#define TCNT1		TPU1_pointer->tcnt
#define TGR1A		TPU1_pointer->tgra
#define TGR1B		TPU1_pointer->tgrb

#define TCR2		TPU2_pointer->tcr
#define TMDR2		TPU2_pointer->tmdr
#define TIOR2		TPU2_pointer->tiorh
#define TIER2		TPU2_pointer->tier
#define TSR2		TPU2_pointer->tsr
#define TCNT2		TPU2_pointer->tcnt
#define TGR2A		TPU2_pointer->tgra
#define TGR2B		TPU2_pointer->tgrb

#define TCR3		TPU3_pointer->tcr
#define TMDR3		TPU3_pointer->tmdr
#define TIOR3H		TPU3_pointer->tiorh
#define TIOR3L		TPU3_pointer->tiorl
#define TIER3		TPU3_pointer->tier
#define TSR3		TPU3_pointer->tsr
#define TCNT3		TPU3_pointer->tcnt
#define TGR3A		TPU3_pointer->tgra
#define TGR3B		TPU3_pointer->tgrb
#define TGR3C		TPU3_pointer->tgrc
#define TGR3D		TPU3_pointer->tgrd

#define TCR4		TPU4_pointer->tcr
#define TMDR4		TPU4_pointer->tmdr
#define TIOR4		TPU4_pointer->tiorh
#define TIER4		TPU4_pointer->tier
#define TSR4		TPU4_pointer->tsr
#define TCNT4		TPU4_pointer->tcnt
#define TGR4A		TPU4_pointer->tgra
#define TGR4B		TPU4_pointer->tgrb

#define TCR5		TPU5_pointer->tcr
#define TMDR5		TPU5_pointer->tmdr
#define TIOR5		TPU5_pointer->tiorh
#define TIER5		TPU5_pointer->tier
#define TSR5		TPU5_pointer->tsr
#define TCNT5		TPU5_pointer->tcnt
#define TGR5A		TPU5_pointer->tgra
#define TGR5B		TPU5_pointer->tgrb

#define TSTR		TPU_COMMON_pointer->tstr
#define TSYR		TPU_COMMON_pointer->tsyr
#endif /* LANGUAGE == C */

#define TCR_OFFSET	0x00
#define TMDR_OFFSET	0x01
#define TIOR_OFFSET	0x02	/* TPU1, TPU2, TPU4, TPU5 */
#define TIORH_OFFSET	0x02	/* TPU0, TPU3 */
#define TIORL_OFFSET	0x03	/* TPU0, TPU3 */
#define TIER_OFFSET	0x04
#define TSR_OFFSET	0x05
#define TCNT_OFFSET	0x06
#define TGRA_OFFSET	0x08
#define TGRB_OFFSET	0x0a
#define TGRC_OFFSET	0x0c	/* TPU0, TPU3 */
#define TGRD_OFFSET	0x0e	/* TPU0, TPU3 */

#define TSTR_OFFSET	0x00
#define TSYR_OFFSET	0x01

/* TCR bits */
#define TCR_CCLR_MASK		bits(7,5) /* bit 7 used only in TPU0 and TPU3 */
#define TCR_CCLR(x)		bits_val(7,5,x)
#define get_TCR_CCLR(x)		bits_get(7,5,x)
#define TCR_CKEG_MASK		bits(4,3)
#define TCR_CKEG(x)		bits_val(4,3,x)
#define get_TCR_CKEG(x)		bits_get(4,3,x)
#define TCR_TPSC_MASK		bits(2,0)
#define TCR_TPSC(x)		bits_val(2,0,x)
#define get_TCR_TPSC(x)		bits_get(2,0,x)

/* TMDR bits */
#define TMDR_BFB		bit(5) /* only for TPU0 and TPU3 */
#define TMDR_BFA		bit(4) /* only for TPU0 and TPU3 */
#define TMDR_MD_MASK		bits(3,0)
#define TMDR_MD(x)		bits_val(3,0,x)
#define get_TMDR_MD(x)		bits_get(3,0,x)

/* TIOR bits (TPU1, TPU2, TPU4, TPU5) */
#define TIOR_IOB_MASK		bits(7,4)
#define TIOR_IOB(x)		bits_val(7,4,x)
#define get_TIOR_IOB(x)		bits_get(7,4,x)
#define TIOR_IOA_MASK		bits(3,0)
#define TIOR_IOA(x)		bits_val(3,0,x)
#define get_TIOR_IOA(x)		bits_get(3,0,x)

/* TIORH bits (TPU0, TPU3) */
#define TIORH_IOB_MASK		bits(7,4)
#define TIORH_IOB(x)		bits_val(7,4,x)
#define get_TIORH_IOB(x)	bits_get(7,4,x)
#define TIORH_IOA_MASK		bits(3,0)
#define TIORH_IOA(x)		bits_val(3,0,x)
#define get_TIORH_IOA(x)	bits_get(3,0,x)

/* TIORL bits (TPU0, TPU3) */
#define TIORL_IOD_MASK		bits(7,4)
#define TIORL_IOD(x)		bits_val(7,4,x)
#define get_TIORL_IOD(x)	bits_get(7,4,x)
#define TIORL_IOC_MASK		bits(3,0)
#define TIORL_IOC(x)		bits_val(3,0,x)
#define get_TIORL_IOC(x)	bits_get(3,0,x)

/* TSR bits */
#define TSR_TCFD		bit(7) /* only for TPU1, TPU2, TPU4, TPU5 */
#define TSR_TCFU		bit(5) /* only for TPU1, TPU2, TPU4, TPU5 */
#define TSR_TCFV		bit(4)
#define TSR_TGFD		bit(3) /* only for TPU0 and TPU3 */
#define TSR_TGFC		bit(2) /* only for TPU0 and TPU3 */
#define TSR_TGFB		bit(1)
#define TSR_TGFA		bit(0)

/* TSTR bits */
#define TSTR_CST5		bit(5)
#define TSTR_CST4		bit(4)
#define TSTR_CST3		bit(3)
#define TSTR_CST2		bit(2)
#define TSTR_CST1		bit(1)
#define TSTR_CST0		bit(0)

/* TSYR bits */
#define TSYR_SYNC5		bit(5)
#define TSYR_SYNC4		bit(4)
#define TSYR_SYNC3		bit(3)
#define TSYR_SYNC2		bit(2)
#define TSYR_SYNC1		bit(1)
#define TSYR_SYNC0		bit(0)

#endif /* H8S2357_TPU_H */
