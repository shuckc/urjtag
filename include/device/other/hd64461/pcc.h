/*
 * $Id$
 *
 * Hitachi HD64461 PC Card Controller Registers
 * Copyright (C) 2004 Marcel Telka
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *     * Neither the name of the copyright holders nor the names of their
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Written by Marcel Telka <marcel@telka.sk>, 2004.
 *
 * Documentation:
 * [1] Hitachi, Ltd., "HD64461 Windows(R) CE Intelligent Peripheral Controller",
 *     1st Edition, July 1998, Order Number: ADE-602-076
 *
 */

#ifndef HD64461_PCC_H
#define	HD64461_PCC_H

#include <common.h>

#if LANGUAGE == C
#include <stdint.h>
#endif

/* PC Card Controller Registers */

#if LANGUAGE == C
typedef volatile struct PCC_registers {
	uint8_t pcc0isr;
	uint8_t __reserved1;
	uint8_t pcc0gcr;
	uint8_t __reserved2;
	uint8_t pcc0cscr;
	uint8_t __reserved3;
	uint8_t pcc0cscier;
	uint8_t __reserved4;
	uint8_t pcc0scr;
	uint8_t __reserved5[7];
	uint8_t pcc1isr;
	uint8_t __reserved6;
	uint8_t pcc1gcr;
	uint8_t __reserved7;
	uint8_t pcc1cscr;
	uint8_t __reserved8;
	uint8_t pcc1cscier;
	uint8_t __reserved9;
	uint8_t pcc1scr;
	uint8_t __reserved10[17];
	uint8_t p0ocr;
	uint8_t __reserved11;
	uint8_t p1ocr;
	uint8_t __reserved12;
	uint8_t pgcr;
} PCC_registers_t;
#endif /* LANGUAGE == C */

#define	PCC0ISR_OFFSET			0x00
#define	PCC0GCR_OFFSET			0x02
#define	PCC0CSCR_OFFSET			0x04
#define	PCC0CSCIER_OFFSET		0x06
#define	PCC0SCR_OFFSET			0x08
#define	PCC1ISR_OFFSET			0x10
#define	PCC1GCR_OFFSET			0x12
#define	PCC1CSCR_OFFSET			0x14
#define	PCC1CSCIER_OFFSET		0x16
#define	PCC1SCR_OFFSET			0x18
#define	P0OCR_OFFSET			0x2A
#define	P1OCR_OFFSET			0x2C
#define	PGCR_OFFSET			0x2E

/* PCC0ISR bits */
#define	PCC0ISR_P0READY			bit(7)
#define	PCC0ISR_P0MWP			bit(6)
#define	PCC0ISR_P0VS2			bit(5)
#define	PCC0ISR_P0VS1			bit(4)
#define	PCC0ISR_P0CD2			bit(3)
#define	PCC0ISR_P0CD1			bit(2)
#define	PCC0ISR_P0BVD2			bit(1)
#define	PCC0ISR_P0BVD1			bit(0)

/* PCC0GCR bits */
#define	PCC0GCR_P0DRVE			bit(7)
#define	PCC0GCR_P0PCCR			bit(6)
#define	PCC0GCR_P0PCCT			bit(5)
#define	PCC0GCR_P0VCC0			bit(4)
#define	PCC0GCR_P0MMOD			bit(3)
#define	PCC0GCR_P0PA25		`	bit(2)
#define	PCC0GCR_P0PA24			bit(1)
#define	PCC0GCR_P0REG			bit(0)

/* PCC0CSCR bits */
#define	PCC0CSCR_P0SCDI			bit(7)
#define	PCC0CSCR_P0IREQ			bit(5)
#define	PCC0CSCR_P0SC			bit(4)
#define	PCC0CSCR_P0CDC			bit(3)
#define	PCC0CSCR_P0RC			bit(2)
#define	PCC0CSCR_P0BW			bit(1)
#define	PCC0CSCR_P0BD			bit(0)

/* PCC0CSCIER bits */
#define	PCC0CSCIER_P0CRE		bit(7)
#define	PCC0CSCIER_P0IREQE1		bit(6)
#define	PCC0CSCIER_P0IREQE0		bit(5)
#define	PCC0CSCIER_P0SCE		bit(4)
#define	PCC0CSCIER_P0CDE		bit(3)
#define	PCC0CSCIER_P0RE			bit(2)
#define	PCC0CSCIER_P0BWE		bit(1)
#define	PCC0CSCIER_P0BDE		bit(0)

/* PCC0SCR bits */
#define	PCC0SCR_P0VCC1			bit(1)
#define	PCC0SCR_P0SWP			bit(0)

/* PCC1ISR bits */
#define	PCC1ISR_P1READY			bit(7)
#define	PCC1ISR_P1MWP			bit(6)
#define	PCC1ISR_P1VS2			bit(5)
#define	PCC1ISR_P1VS1			bit(4)
#define	PCC1ISR_P1CD2			bit(3)
#define	PCC1ISR_P1CD1			bit(2)
#define	PCC1ISR_P1BVD2			bit(1)
#define	PCC1ISR_P1BVD1			bit(0)

/* PCC1GCR bits */
#define	PCC1GCR_P1DRVE			bit(7)
#define	PCC1GCR_P1PCCR			bit(6)
#define	PCC1GCR_P1VCC0			bit(4)
#define	PCC1GCR_P1MMOD			bit(3)
#define	PCC1GCR_P1PA25		`	bit(2)
#define	PCC1GCR_P1PA24			bit(1)
#define	PCC1GCR_P1REG			bit(0)

/* PCC1CSCR bits */
#define	PCC1CSCR_P1SCDI			bit(7)
#define	PCC1CSCR_P1CDC			bit(3)
#define	PCC1CSCR_P1RC			bit(2)
#define	PCC1CSCR_P1BW			bit(1)
#define	PCC1CSCR_P1BD			bit(0)

/* PCC1CSCIER bits */
#define	PCC1CSCIER_P1CRE		bit(7)
#define	PCC1CSCIER_P1CDE		bit(3)
#define	PCC1CSCIER_P1RE			bit(2)
#define	PCC1CSCIER_P1BWE		bit(1)
#define	PCC1CSCIER_P1BDE		bit(0)

/* PCC1SCR bits */
#define	PCC1SCR_P1VCC1			bit(1)
#define	PCC1SCR_P1SWP			bit(0)

/* P0OCR bits */
#define	P0OCR_P0DEPLUP			bit(7)
#define	P0OCR_P0AEPLUP			bit(4)

/* P1OCR bits */
#define	P1OCR_P1RST8MA			bit(3)
#define	P1OCR_P1RST4MA			bit(2)
#define	P1OCR_P1RAS8MA			bit(1)
#define	P1OCR_P1RAS4MA			bit(0)

/* PGCR bits */
#define	PGCR_PSSDIR			bit(1)
#define	PGCR_PSSRDWR			bit(0)

#endif /* HD64461_PCC_H */
