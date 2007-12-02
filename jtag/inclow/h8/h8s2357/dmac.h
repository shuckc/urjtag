/*
 * $Id$
 *
 * H8S/2357 DMAC Registers
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

#ifndef H8S2357_DMAC_H
#define H8S2357_DMAC_H

#include <openwince.h>

#if LANGUAGE == C
#include <stdint.h>
#endif

/* DMAC registers */

#define DMAC_BASE	0xfffffee0

#if LANGUAGE == C
typedef volatile struct DMAC_registers {
	uint32_t mar0a;
	uint16_t ioar0a;
	uint16_t etcr0a;
	uint32_t mar0b;
	uint16_t ioar0b;
	uint16_t etcr0b;
	uint32_t mar1a;
	uint16_t ioar1a;
	uint16_t etcr1a;
	uint32_t mar1b;
	uint16_t ioar1b;
	uint16_t etcr1b;
	uint8_t dmawer;
	uint8_t dmatcr;
	uint8_t dmacr0a;
	uint8_t dmacr0b;
	uint8_t dmacr1a;
	uint8_t dmacr1b;
	uint8_t dmabcr;
} DMAC_registers_t;

#define DMAC_pointer	((DMAC_registers_t*) DMAC_BASE)

#define MAR0A		DMAC_pointer->mar0a
#define IOAR0A		DMAC_pointer->ioar0a
#define ETCR0A		DMAC_pointer->etcr0a
#define MAR0B		DMAC_pointer->mar0b
#define IOAR0B		DMAC_pointer->ioar0b
#define ETCR0B		DMAC_pointer->etcr0b
#define MAR1A		DMAC_pointer->mar1a
#define IOAR1A		DMAC_pointer->ioar1a
#define ETCR1A		DMAC_pointer->etcr1a
#define MAR1B		DMAC_pointer->mar1b
#define IOAR1B		DMAC_pointer->ioar1b
#define ETCR1B		DMAC_pointer->etcr1b
#define DMAWER		DMAC_pointer->dmawer
#define DMATCR		DMAC_pointer->dmatcr
#define DMACR0A		DMAC_pointer->dmacr0a
#define DMACR0B		DMAC_pointer->dmacr0b
#define DMACR1A		DMAC_pointer->dmacr1a
#define DMACR1B		DMAC_pointer->dmacr1b
#define DMABCR		DMAC_pointer->dmabcr
#endif /* LANGUAGE == C */

#define MAR0A_OFFSET	0x00
#define IOAR0A_OFFSET	0x04
#define ETCR0A_OFFSET	0x06
#define MAR0B_OFFSET	0x08
#define IOAR0B_OFFSET	0x0c
#define ETCR0B_OFFSET	0x0e
#define MAR1A_OFFSET	0x10
#define IOAR1A_OFFSET	0x14
#define ETCR1A_OFFSET	0x16
#define MAR1B_OFFSET	0x18
#define IOAR1B_OFFSET	0x1c
#define ETCR1B_OFFSET	0x1e
#define DMAWER_OFFSET	0x20
#define DMATCR_OFFSET	0x21
#define DMACR0A_OFFSET	0x22
#define DMACR0B_OFFSET	0x23
#define DMACR1A_OFFSET	0x24
#define DMACR1B_OFFSET	0x25
#define DMABCR_OFFSET	0x26

/* DMAWER bits */
#define DMAWER_WE1B	bit(3)
#define DMAWER_WE1A	bit(2)
#define DMAWER_WE0B	bit(1)
#define DMAWER_WE0A	bit(0)

/* DMATCR bits */
#define DMATCR_TEE1	bit(5)
#define DMATCR_TEE0	bit(4)

/* DMACR bits - short address mode */
#define DMACR_DTSZ		bit(7)
#define DMACR_DTID		bit(6)
#define DMACR_RPE		bit(5)
#define DMACR_DTDIR		bit(4)
#define DMACR_DTF_MASK		bits(3,0)
#define DMACR_DTF(x)		bits_val(3,0,x)
#define get_DMACR_DTF(x)	bits_get(3,0,x)

/* DMACRA bits - full address mode */
#define DMACRA_DTSZ		bit(7)
#define DMACRA_SAID		bit(6)
#define DMACRA_SAIDE		bit(5)
#define DMACRA_BLKDIR		bit(4)
#define DMACRA_BLKE		bit(3)

/* DMACRB bits - full address mode */
#define DMACRB_DAID		bit(6)
#define DMACRB_DAIDE		bit(5)
#define DMACRB_DTF_MASK		bits(3,0)
#define DMACRB_DTF(x)		bits_val(3,0,x)
#define get_DMACRB_DTF(x)	bits_get(3,0,x)

/* DMABCR bits - short address mode */
#define DMABCR_FAE1		bit(15)
#define DMABCR_FAE0		bit(14)
#define DMABCR_SAE1		bit(13)
#define DMABCR_SAE0		bit(12)
#define DMABCR_DTA1B		bit(11)
#define DMABCR_DTA1A		bit(10)
#define DMABCR_DTA0B		bit(9)
#define DMABCR_DTA0A		bit(8)
#define DMABCR_DTE1B		bit(7)
#define DMABCR_DTE1A		bit(6)
#define DMABCR_DTE0B		bit(5)
#define DMABCR_DTE0A		bit(4)
#define DMABCR_DTIE1B		bit(3)
#define DMABCR_DTIE1A		bit(2)
#define DMABCR_DTIE0B		bit(1)
#define DMABCR_DTIE0A		bit(0)

/* DMABCR bits - full address mode */
#define DMABCR_DTA1		bit(11)
#define DMABCR_DTA0		bit(9)
#define DMABCR_DTME1		bit(7)
#define DMABCR_DTE1		bit(6)
#define DMABCR_DTME0		bit(5)
#define DMABCR_DTE0		bit(4)

#endif /* H8S2357_DMAC_H */
