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

#include <common.h>

#if LANGUAGE == C
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

#if LANGUAGE == C
typedef struct ITU_registers {
	uint8_t tcr;
	uint8_t tior;
	uint8_t tier;
	uint8_t tsr;
	uint8_t tcnth;
	uint8_t tcntl;
	uint8_t grah;
	uint8_t gral;
	uint8_t grbh;
	uint8_t grbl;
	uint8_t brah;	/* only ITU channel 3 and 4 */
	uint8_t bral;	/* only ITU channel 3 and 4 */
	uint8_t brbh;	/* only ITU channel 3 and 4 */
	uint8_t brbl;	/* only ITU channel 3 and 4 */
} ITU_registers_t;

typedef struct ITU_common1_registers {
	uint8_t tstr;
	uint8_t tsnc;
	uint8_t tmdr;
	uint8_t tfcr;
} ITU_common1_registers_t;

typedef struct ITU_common2_registers {
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
#define TCNT0H		ITU0_pointer->tcnth
#define TCNT0L		ITU0_pointer->tcntl
#define GRA0H		ITU0_pointer->grah
#define GRA0L		ITU0_pointer->gral
#define GRB0H		ITU0_pointer->grbh
#define GRB0L		ITU0_pointer->grbl

#define TCR1		ITU1_pointer->tcr
#define TIOR1		ITU1_pointer->tior
#define TIER1		ITU1_pointer->tier
#define TSR1		ITU1_pointer->tsr
#define TCNT1H		ITU1_pointer->tcnth
#define TCNT1L		ITU1_pointer->tcntl
#define GRA1H		ITU1_pointer->grah
#define GRA1L		ITU1_pointer->gral
#define GRB1H		ITU1_pointer->grbh
#define GRB1L		ITU1_pointer->grbl

#define TCR2		ITU2_pointer->tcr
#define TIOR2		ITU2_pointer->tior
#define TIER2		ITU2_pointer->tier
#define TSR2		ITU2_pointer->tsr
#define TCNT2H		ITU2_pointer->tcnth
#define TCNT2L		ITU2_pointer->tcntl
#define GRA2H		ITU2_pointer->grah
#define GRA2L		ITU2_pointer->gral
#define GRB2H		ITU2_pointer->grbh
#define GRB2L		ITU2_pointer->grbl

#define TCR3		ITU3_pointer->tcr
#define TIOR3		ITU3_pointer->tior
#define TIER3		ITU3_pointer->tier
#define TSR3		ITU3_pointer->tsr
#define TCNT3H		ITU3_pointer->tcnth
#define TCNT3L		ITU3_pointer->tcntl
#define GRA3H		ITU3_pointer->grah
#define GRA3L		ITU3_pointer->gral
#define GRB3H		ITU3_pointer->grbh
#define GRB3L		ITU3_pointer->grbl
#define BRA3H		ITU3_pointer->brah
#define BRA3L		ITU3_pointer->bral
#define BRB3H		ITU3_pointer->brbh
#define BRB3L		ITU3_pointer->brbl

#define TCR4		ITU4_pointer->tcr
#define TIOR4		ITU4_pointer->tior
#define TIER4		ITU4_pointer->tier
#define TSR4		ITU4_pointer->tsr
#define TCNT4H		ITU4_pointer->tcnth
#define TCNT4L		ITU4_pointer->tcntl
#define GRA4H		ITU4_pointer->grah
#define GRA4L		ITU4_pointer->gral
#define GRB4H		ITU4_pointer->grbh
#define GRB4L		ITU4_pointer->grbl
#define BRA4H		ITU4_pointer->brah
#define BRA4L		ITU4_pointer->bral
#define BRB4H		ITU4_pointer->brbh
#define BRB4L		ITU4_pointer->brbl

#define TSTR		ITU_COMMON1_pointer->tstr
#define TSNC		ITU_COMMON1_pointer->tsnc
#define TMDR		ITU_COMMON1_pointer->tmdr
#define TFCR		ITU_COMMON1_pointer->tfcr

#define TOER		ITU_COMMON2_pointer->toer
#define TOCR		ITU_COMMON2_pointer->tocr
#endif /* LANGUAGE == C */

#define TCR_OFFSET	0x00
#define TIOR_OFFSET	0x01
#define TIER_OFFSET	0x02
#define TSR_OFFSET	0x03
#define TCNTH_OFFSET	0x04
#define TCNTL_OFFSET	0x05
#define GRAH_OFFSET	0x06
#define GRAL_OFFSET	0x07
#define GRBH_OFFSET	0x08
#define GRBL_OFFSET	0x09
#define BRAH_OFFSET	0x0a
#define BRAL_OFFSET	0x0b
#define BRBH_OFFSET	0x0c
#define BRBL_OFFSET	0x0d

#define TSTR_OFFSET	0x00
#define TSNC_OFFSET	0x01
#define TMDR_OFFSET	0x02
#define TFCR_OFFSET	0x03

#define TOER_OFFSET	0x00
#define TOCR_OFFSET	0x01


#endif /* H83048_ITU_H */
