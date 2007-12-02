/*
 * $Id$
 *
 * H8S/2357 BUS Controller Registers
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

#ifndef H8S2357_BUS_H
#define H8S2357_BUS_H

#include <openwince.h>

#if LANGUAGE == C
#include <stdint.h>
#endif

/* BUS registers */

#define BUS_BASE	0xfffffed0

#if LANGUAGE == C
typedef volatile struct BUS_registers {
	uint8_t abwcr;
	uint8_t astcr;
	uint8_t wcrh;
	uint8_t wcrl;
	uint8_t bcrh;
	uint8_t bcrl;
	uint8_t mcr;
	uint8_t dramcr;
	uint8_t rtcnt;
	uint8_t rtcor;
	uint8_t __reserved;
	uint8_t ramer;
} BUS_registers_t;

#define BUS_pointer	((BUS_registers_t*) BUS_BASE)

#define ABWCR		BUS_pointer->abwcr
#define ASTCR		BUS_pointer->astcr
#define WCRH		BUS_pointer->wcrh
#define WCRL		BUS_pointer->wcrl
#define BCRH		BUS_pointer->bcrh
#define BCRL		BUS_pointer->bcrl
#define MCR		BUS_pointer->mcr
#define DRAMCR		BUS_pointer->dramcr
#define RTCNT		BUS_pointer->rtcnt
#define RTCOR		BUS_pointer->rtcor
#define RAMER		BUS_pointer->ramer
#endif /* LANGUAGE == C */

#define ABWCR_OFFSET	0x00
#define ASTCR_OFFSET	0x01
#define WCRH_OFFSET	0x02
#define WCRL_OFFSET	0x03
#define BCRH_OFFSET	0x04
#define BCRL_OFFSET	0x05
#define MCR_OFFSET	0x06
#define DRAMCR_OFFSET	0x07
#define RTCNT_OFFSET	0x08
#define RTCOR_OFFSET	0x09
#define RAMER_OFFSET	0x0b

/* ABWCR bits */
#define ABWCR_ABW7		bit(7)
#define ABWCR_ABW6		bit(6)
#define ABWCR_ABW5		bit(5)
#define ABWCR_ABW4		bit(4)
#define ABWCR_ABW3		bit(3)
#define ABWCR_ABW2		bit(2)
#define ABWCR_ABW1		bit(1)
#define ABWCR_ABW0		bit(0)

/* ASTCR bits */
#define ASTCR_AST7		bit(7)
#define ASTCR_AST6		bit(6)
#define ASTCR_AST5		bit(5)
#define ASTCR_AST4		bit(4)
#define ASTCR_AST3		bit(3)
#define ASTCR_AST2		bit(2)
#define ASTCR_AST1		bit(1)
#define ASTCR_AST0		bit(0)

/* WCRH bits */
#define WCRH_W7_MASK		bits(7,6)
#define WCRH_W7(x)		bits_val(7,6,x)
#define get_WCRH_W7(x)		bits_get(7,6,x)
#define WCRH_W6_MASK		bits(5,4)
#define WCRH_W6(x)		bits_val(5,4,x)
#define get_WCRH_W6(x)		bits_get(5,4,x)
#define WCRH_W5_MASK		bits(3,2)
#define WCRH_W5(x)		bits_val(3,2,x)
#define get_WCRH_W5(x)		bits_get(3,2,x)
#define WCRH_W4_MASK		bits(1,0)
#define WCRH_W4(x)		bits_val(1,0,x)
#define get_WCRH_W4(x)		bits_get(1,0,x)

/* WCRL bits */
#define WCRL_W3_MASK		bits(7,6)
#define WCRL_W3(x)		bits_val(7,6,x)
#define get_WCRL_W3(x)		bits_get(7,6,x)
#define WCRL_W2_MASK		bits(5,4)
#define WCRL_W2(x)		bits_val(5,4,x)
#define get_WCRL_W2(x)		bits_get(5,4,x)
#define WCRL_W1_MASK		bits(3,2)
#define WCRL_W1(x)		bits_val(3,2,x)
#define get_WCRL_W1(x)		bits_get(3,2,x)
#define WCRL_W0_MASK		bits(1,0)
#define WCRL_W0(x)		bits_val(1,0,x)
#define get_WCRL_W0(x)		bits_get(1,0,x)

/* BCRH bits */
#define BCRH_ICIS1		bit(7)
#define BCRH_ICIS0		bit(6)
#define BCRH_BRSTRM		bit(5)
#define BCRH_BRSTS1		bit(4)
#define BCRH_BRSTS0		bit(3)
#define BCRH_RMTS_MASK		bits(2,0)
#define BCRH_RMTS(x)		bits_val(2,0,x)
#define get_BCRH_RMTS(x)	bits_get(2,0,x)

/* BCRL bits */
#define BCRL_BRLE		bit(7)
#define BCRL_BREQOE		bit(6)
#define BCRL_EAE		bit(5)
#define BCRL_LCASS		bit(4)
#define BCRL_DDS		bit(3)
#define BCRL_WDBE		bit(1)
#define BCRL_WAITE		bit(0)

/* MCR bits */
#define MCR_TPC			bit(7)
#define MCR_BE			bit(6)
#define MCR_RCDM		bit(5)
#define MCR_CW2			bit(4)
#define MCR_MXC_MASK		bits(3,2)
#define MCR_MXC(x)		bits_val(3,2,x)
#define get_MCR_MXC(x)		bits_get(3,2,x)
#define MCR_RLW_MASK		bits(1,0)
#define MCR_RLW(x)		bits_val(1,0,x)
#define get_MCR_RLW(x)		bits_get(1,0,x)

/* DRAMCR bits */
#define DRAMCR_RFSHE		bit(7)
#define DRAMCR_RCW		bit(6)
#define DRAMCR_RMODE		bit(5)
#define DRAMCR_CMF		bit(4)
#define DRAMCR_CMIE		bit(3)
#define DRAMCR_CKS_MASK		bits(2,0)
#define DRAMCR_CKS(x)		bits_val(2,0,x)
#define get_DRAMCR_CKS(x)	bits_get(2,0,x)

/* RAMER bits */
#define RAMER_RAMS		bit(3)
#define RAMER_RAM_MASK		bits(2,0)
#define RAMER_RAM(x)		bits_val(2,0,x)
#define get_RAMER_RAM(x)	bits_get(2,0,x)

#endif /* H8S2357_BUS_H */
