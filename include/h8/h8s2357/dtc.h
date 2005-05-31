/*
 * $Id$
 *
 * H8S/2357 DTC Registers
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

#ifndef H8S2357_DTC_H
#define H8S2357_DTC_H

#include <common.h>

#if LANGUAGE == C
#include <stdint.h>
#endif

/* DTC registers */

#define DTCR_BASE	0xffffff30

#if LANGUAGE == C
typedef struct DTCR_registers {
	uint8_t dtcera;
	uint8_t dtcerb;
	uint8_t dtcerc;
	uint8_t dtcerd;
	uint8_t dtcere;
	uint8_t dtcerf;
	uint8_t __reserved;
	uint8_t dtvecr;
} DTCR_registers_t;

#define DTCR_pointer	((DTCR_registers_t*) DTCR_BASE)

#define DTCERA		DTCR_pointer->dtcera
#define DTCERB		DTCR_pointer->dtcerb
#define DTCERC		DTCR_pointer->dtcerc
#define DTCERD		DTCR_pointer->dtcerd
#define DTCERE		DTCR_pointer->dtcere
#define DTCERF		DTCR_pointer->dtcerf
#define DTVECR		DTCR_pointer->dtvecr
#endif /* LANGUAGE == C */

#define DTCERA_OFFSET	0x00
#define DTCERB_OFFSET	0x01
#define DTCERC_OFFSET	0x02
#define DTCERD_OFFSET	0x03
#define DTCERE_OFFSET	0x04
#define DTCERF_OFFSET	0x05
#define DTVECR_OFFSET	0x07

/* DTCERA bits */
#define DTCERA_DTCEA7	bit(7)
#define DTCERA_DTCEA6	bit(6)
#define DTCERA_DTCEA5	bit(5)
#define DTCERA_DTCEA4	bit(4)
#define DTCERA_DTCEA3	bit(3)
#define DTCERA_DTCEA2	bit(2)
#define DTCERA_DTCEA1	bit(1)
#define DTCERA_DTCEA0	bit(0)

/* DTCERB bits */
#define DTCERB_DTCEB7	bit(7)
#define DTCERB_DTCEB6	bit(6)
#define DTCERB_DTCEB5	bit(5)
#define DTCERB_DTCEB4	bit(4)
#define DTCERB_DTCEB3	bit(3)
#define DTCERB_DTCEB2	bit(2)
#define DTCERB_DTCEB1	bit(1)
#define DTCERB_DTCEB0	bit(0)

/* DTCERC bits */
#define DTCERC_DTCEC7	bit(7)
#define DTCERC_DTCEC6	bit(6)
#define DTCERC_DTCEC5	bit(5)
#define DTCERC_DTCEC4	bit(4)
#define DTCERC_DTCEC3	bit(3)
#define DTCERC_DTCEC2	bit(2)
#define DTCERC_DTCEC1	bit(1)
#define DTCERC_DTCEC0	bit(0)

/* DTCERD bits */
#define DTCERD_DTCED7	bit(7)
#define DTCERD_DTCED6	bit(6)
#define DTCERD_DTCED5	bit(5)
#define DTCERD_DTCED4	bit(4)
#define DTCERD_DTCED3	bit(3)
#define DTCERD_DTCED2	bit(2)
#define DTCERD_DTCED1	bit(1)
#define DTCERD_DTCED0	bit(0)

/* DTCERE bits */
#define DTCERE_DTCEE7	bit(7)
#define DTCERE_DTCEE6	bit(6)
#define DTCERE_DTCEE5	bit(5)
#define DTCERE_DTCEE4	bit(4)
#define DTCERE_DTCEE3	bit(3)
#define DTCERE_DTCEE2	bit(2)
#define DTCERE_DTCEE1	bit(1)
#define DTCERE_DTCEE0	bit(0)

/* DTCERF bits */
#define DTCERF_DTCEF7	bit(7)
#define DTCERF_DTCEF6	bit(6)
#define DTCERF_DTCEF5	bit(5)
#define DTCERF_DTCEF4	bit(4)
#define DTCERF_DTCEF3	bit(3)
#define DTCERF_DTCEF2	bit(2)
#define DTCERF_DTCEF1	bit(1)
#define DTCERF_DTCEF0	bit(0)

/* DTVECR bits */
#define DTVECR_SWDTE		bit(7)
#define DTVECR_DTVEC_MASK	bits(6,0)
#define DTVECR_DTVEC(x)		bits_val(6,0,x)
#define get_DTVECR_DTVEC(x)	bits_get(6,0,x)

#endif /* H8S2357_DTC_H */
