/*
 * $Id$
 *
 * H8/3048 Flash Registers
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

#ifndef H83048_FLASH_H
#define H83048_FLASH_H

#include <common.h>

#if LANGUAGE == C
#include <stdint.h>
#endif

/* FLASH registers */

#define FLASH_BASE	0xffff40

#if LANGUAGE == C
typedef volatile struct FLASH_registers {
	uint8_t flmcr;
	uint8_t __reserved1;
	uint8_t ebr1;
	uint8_t ebr2;
	uint8_t __reserved2[4];
	uint8_t ramcr;
} FLASH_registers_t;

#define FLASH_pointer	((FLASH_registers_t*) FLASH_BASE)

#define FLMCR		FLASH_pointer->flmcr
#define EBR1		FLASH_pointer->ebr1
#define EBR2		FLASH_pointer->ebr2
#define RAMCR		FLASH_pointer->ramcr
#endif /* LANGUAGE == C */

#define FLMCR_OFFSET	0x00
#define EBR1_OFFSET	0x02
#define EBR2_OFFSET	0x03
#define RAMCR_OFFSET	0x08

/* FLMCR bits */
#define FLMCR_VPP		bit(7)
#define FLMCR_VPPE		bit(6)
#define FLMCR_EV		bit(3)
#define FLMCR_PV		bit(2)
#define FLMCR_E			bit(1)
#define FLMCR_P			bit(0)

/* EBR1 bits */
#define EBR1_LB7		bit(7)
#define EBR1_LB6		bit(6)
#define EBR1_LB5		bit(5)
#define EBR1_LB4		bit(4)
#define EBR1_LB3		bit(3)
#define EBR1_LB2		bit(2)
#define EBR1_LB1		bit(1)
#define EBR1_LB0		bit(0)

/* EBR2 bits */
#define EBR2_SB7		bit(7)
#define EBR2_SB6		bit(6)
#define EBR2_SB5		bit(5)
#define EBR2_SB4		bit(4)
#define EBR2_SB3		bit(3)
#define EBR2_SB2		bit(2)
#define EBR2_SB1		bit(1)
#define EBR2_SB0		bit(0)

/* RAMCR bits */
#define RAMCR_FLER		bit(7)
#define RAMCR_RAMS		bit(3)
#define RAMCR_RAM_MASK		bits(2,0)
#define RAMCR_RAM(x)		bits_val(2,0,x)
#define get_RAMCR_RAM(x)	bits_get(2,0,x)

#endif /* H83048_FLASH_H */
