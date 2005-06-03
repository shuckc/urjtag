/*
 * $Id$
 *
 * H8/3048 DMAC Registers
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

#ifndef H83048_DMAC_H
#define H83048_DMAC_H

#include <common.h>

#if LANGUAGE == C
#include <stdint.h>
#endif

/* DMAC registers */

#define DMAC_BASE	0xffff20

#if LANGUAGE == C
typedef volatile struct DMAC_registers {
	uint32_t mar0a;
	uint16_t etcr0a;
	uint8_t ioar0a;
	uint8_t dtcr0a;
	uint32_t mar0b;
	uint16_t etcr0b;
	uint8_t ioar0b;
	uint8_t dtcr0b;
	uint32_t mar1a;
	uint16_t etcr1a;
	uint8_t ioar1a;
	uint8_t dtcr1a;
	uint32_t mar1b;
	uint16_t etcr1b;
	uint8_t ioar1b;
	uint8_t dtcr1b;
} DMAC_registers_t;

#define DMAC_pointer	((DMAC_registers_t*) DMAC_BASE)

#define MAR0A		DMAC_pointer->mar0a
#define ETCR0A		DMAC_pointer->etcr0a
#define IOAR0A		DMAC_pointer->ioar0a
#define DTCR0A		DMAC_pointer->dtcr0a
#define MAR0B		DMAC_pointer->mar0b
#define ETCR0B		DMAC_pointer->etcr0b
#define IOAR0B		DMAC_pointer->ioar0b
#define DTCR0B		DMAC_pointer->dtcr0b
#define MAR1A		DMAC_pointer->mar1a
#define ETCR1A		DMAC_pointer->etcr1a
#define IOAR1A		DMAC_pointer->ioar1a
#define DTCR1A		DMAC_pointer->dtcr1a
#define MAR1B		DMAC_pointer->mar1b
#define ETCR1B		DMAC_pointer->etcr1b
#define IOAR1B		DMAC_pointer->ioar1b
#define DTCR1B		DMAC_pointer->dtcr1b
#endif /* LANGUAGE == C */

#define MAR0A_OFFSET	0x00
#define ETCR0A_OFFSET	0x04
#define IOAR0A_OFFSET	0x06
#define DTCR0A_OFFSET	0x07
#define MAR0B_OFFSET	0x08
#define ETCR0B_OFFSET	0x0c
#define IOAR0B_OFFSET	0x0e
#define DTCR0B_OFFSET	0x0f
#define MAR1A_OFFSET	0x10
#define ETCR1A_OFFSET	0x14
#define IOAR1A_OFFSET	0x16
#define DTCR1A_OFFSET	0x17
#define MAR1B_OFFSET	0x18
#define ETCR1B_OFFSET	0x1c
#define IOAR1B_OFFSET	0x1e
#define DTCR1B_OFFSET	0x1f

/* DTCRA bits - short address mode */
#define DTCRA_DTE		bit(7)
#define DTCRA_DTSZ		bit(6)
#define DTCRA_DTID		bit(5)
#define DTCRA_RPE		bit(4)
#define DTCRA_DTIE		bit(3)
#define DTCRA_DTS_MASK		bits(2,0)
#define DTCRA_DTS(x)		bits_val(2,0,x)
#define get_DTCRA_DTS(x)	bits_get(2,0,x)

/* DTCRA bits - full address mode */
#define DTCRA_SAID		bit(5)
#define DTCRA_SAIDE		bit(4)
#define DTCRA_DTSA_MASK		bits(2,0)
#define DTCRA_DTSA(x)		bits_val(2,0,x)
#define get_DTCRA_DTSA(x)	bits_get(2,0,x)

/* DTCRB bits - short address mode */
#define DTCRB_DTE		bit(7)
#define DTCRB_DTSZ		bit(6)
#define DTCRB_DTID		bit(5)
#define DTCRB_RPE		bit(4)
#define DTCRB_DTIE		bit(3)
#define DTCRB_DTS_MASK		bits(2,0)
#define DTCRB_DTS(x)		bits_val(2,0,x)
#define get_DTCRB_DTS(x)	bits_get(2,0,x)

/* DTCRB bits - full address mode */
#define DTCRB_DAID		bit(5)
#define DTCRB_DAIDE		bit(4)
#define DTCRB_TMS		bit(3)
#define DTCRB_DTSB_MASK		bits(2,0)
#define DTCRB_DTSB(x)		bits_val(2,0,x)
#define get_DTCRB_DTSB(x)	bits_get(2,0,x)

#endif /* H83048_DMAC_H */
