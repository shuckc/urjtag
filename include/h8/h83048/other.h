/*
 * $Id$
 *
 * H8/3048 Other Registers (bus, system, ...)
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

#ifndef H83048_OTHER_H
#define H83048_OTHER_H

#include <common.h>

#if LANGUAGE == C
#include <stdint.h>
#endif

/* OTHER registers */

#define OTHER1_BASE	0xffff5c
#define OTHER2_BASE	0xffffec

#if LANGUAGE == C
typedef struct OTHER1_registers {
	uint8_t dastcr;
	uint8_t divcr;
	uint8_t mstcr;
	uint8_t cscr;
} OTHER1_registers_t;

typedef struct OTHER2_registers {
	uint8_t abwcr;
	uint8_t astcr;
	uint8_t wcr;
	uint8_t wcer;
	uint8_t __reserved;
	uint8_t mdcr;
	uint8_t syscr;
	uint8_t brcr;
} OTHER2_registers_t;

#define OTHER1_pointer	((OTHER1_registers_t*) OTHER1_BASE)
#define OTHER2_pointer	((OTHER2_registers_t*) OTHER2_BASE)

#define DASTCR		OTHER1_pointer->dastcr
#define DIVCR		OTHER1_pointer->divcr
#define MSTCR		OTHER1_pointer->mstcr
#define CSCR		OTHER1_pointer->cscr

#define ABWCR		OTHER2_pointer->abwcr
#define ASTCR		OTHER2_pointer->astcr
#define WCR		OTHER2_pointer->wcr
#define WCER		OTHER2_pointer->wcer
#define MDCR		OTHER2_pointer->mdcr
#define SYSCR		OTHER2_pointer->syscr
#define BRCR		OTHER2_pointer->brcr
#endif /* LANGUAGE == C */

#define DASTCR_OFFSET	0x00
#define DIVCR_OFFSET	0x01
#define MSTCR_OFFSET	0x02
#define CSCR_OFFSET	0x03
                                  
#define ABWCR_OFFSET	0x00
#define ASTCR_OFFSET	0x01
#define WCR_OFFSET	0x02
#define WCER_OFFSET	0x03
#define MDCR_OFFSET	0x05
#define SYSCR_OFFSET	0x06
#define BRCR_OFFSET	0x07

/* DASTCR bits */
#define DASTCR_DASTE		bit(0)

/* DIVCR bits */
#define DIVCR_DIV_MASK		bits(1,0)
#define DIVCR_DIV(x)		bits_val(1,0,x)
#define get_DIVCR_DIV(x)	bits_get(1,0,x)

/* MSTCR bits */
#define MSTCR_PSTOP		bit(7)
#define MSTCR_MSTOP5		bit(5)
#define MSTCR_MSTOP4		bit(4)
#define MSTCR_MSTOP3		bit(3)
#define MSTCR_MSTOP2		bit(2)
#define MSTCR_MSTOP1		bit(1)
#define MSTCR_MSTOP0		bit(0)

/* CSCR bits */
#define CSCR_CS7E		bit(7)
#define CSCR_CS6E		bit(6)
#define CSCR_CS5E		bit(5)
#define CSCR_CS4E		bit(4)

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

/* WCR bits */
#define WCR_WMS_MASK		bits(3,2)
#define WCR_WMS(x)		bits_val(3,2,x)
#define get_WCR_WMS(x)		bits_get(3,2,x)
#define WCR_WC_MASK		bits(1,0)
#define WCR_WC(x)		bits_val(1,0,x)
#define get_WCR_WC(x)		bits_get(1,0,x)

/* WCER bits */
#define WCER_WCE7		bit(7)
#define WCER_WCE6		bit(6)
#define WCER_WCE5		bit(5)
#define WCER_WCE4		bit(4)
#define WCER_WCE3		bit(3)
#define WCER_WCE2		bit(2)
#define WCER_WCE1		bit(1)
#define WCER_WCE0		bit(0)

/* MDCR bits */
#define MDCR_MDS_MASK		bits(2,0)
#define MDCR_MDS(x)		bits_val(2,0,x)
#define get_MDCR_MDS(x)		bits_get(2,0,x)

/* SYSCR bits */
#define SYSCR_SSBY		bit(7)
#define SYSCR_STS_MASK		bits(6,4)
#define SYSCR_STS(x)		bits_val(6,4,x)
#define get_SYSCR_STS(x)	bits_get(6,4,x)
#define SYSCR_UE		bit(3)
#define SYSCR_NMIEG		bit(2)
#define SYSCR_RAME		bit(0)

/* BRCR bits */
#define BRCR_A23E		bit(7)
#define BRCR_A22E		bit(6)
#define BRCR_A21E		bit(5)
#define BRCR_BRLE		bit(0)

#endif /* H83048_OTHER_H */
