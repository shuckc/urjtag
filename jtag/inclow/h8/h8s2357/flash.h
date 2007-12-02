/*
 * $Id$
 *
 * H8S/2357 FLASH Registers
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

#ifndef H8S2357_FLASH_H
#define H8S2357_FLASH_H

#include <openwince.h>

#if LANGUAGE == C
#include <stdint.h>
#endif

/* FLASH registers */

#define FLASH_BASE	0xffffffc8

#if LANGUAGE == C
typedef volatile struct FLASH_registers {
	uint8_t flmcr1;
	uint8_t flmcr2;
	uint8_t ebr1;
	uint8_t ebr2;
} FLASH_registers_t;

#define FLASH_pointer	((FLASH_registers_t*) FLASH_BASE)

#define FLMCR1		FLASH_pointer->flmcr1
#define FLMCR2		FLASH_pointer->flmcr2
#define EBR1		FLASH_pointer->ebr1
#define EBR2		FLASH_pointer->ebr2
#endif /* LANGUAGE == C */

#define FLMCR1_OFFSET	0x00
#define FLMCR2_OFFSET	0x01
#define EBR1_OFFSET	0x02
#define EBR2_OFFSET	0x03

/* FLMCR1 bits */
#define FLMCR1_FWE	bit(7)
#define FLMCR1_SWE	bit(6)
#define FLMCR1_ESU	bit(5) /* 2398F-ZTAT */
#define FLMCR1_PSU	bit(4) /* 2398F-ZTAT */
#define FLMCR1_EV	bit(3)
#define FLMCR1_PV	bit(2)
#define FLMCR1_E	bit(1)
#define FLMCR1_P	bit(0)

/* FLMCR2 bits */
#define FLMCR2_FLER	bit(7)
#define FLMCR2_ESU	bit(1) /* 2357F-ZTAT */
#define FLMCR2_PSU	bit(0) /* 2357F-ZTAT */

/* EBR1 bits */
#define EBR1_EB9	bit(1) /* 2357F-ZTAT */
#define EBR1_EB8	bit(0) /* 2357F-ZTAT */
#define EBR1_EB7	bit(7) /* 2398F-ZTAT */
#define EBR1_EB6	bit(6) /* 2398F-ZTAT */
#define EBR1_EB5	bit(5) /* 2398F-ZTAT */
#define EBR1_EB4	bit(4) /* 2398F-ZTAT */
#define EBR1_EB3	bit(3) /* 2398F-ZTAT */
#define EBR1_EB2	bit(2) /* 2398F-ZTAT */
#define EBR1_EB1	bit(1) /* 2398F-ZTAT */
#define EBR1_EB0	bit(0) /* 2398F-ZTAT */

/* EBR2 bits */
#define EBR2_EB11	bit(3) /* 2398F-ZTAT */
#define EBR2_EB10	bit(2) /* 2398F-ZTAT */
#define EBR2_EB9	bit(1) /* 2398F-ZTAT */
#define EBR2_EB8	bit(0) /* 2398F-ZTAT */
#define EBR2_EB7	bit(7) /* 2357F-ZTAT */
#define EBR2_EB6	bit(6) /* 2357F-ZTAT */
#define EBR2_EB5	bit(5) /* 2357F-ZTAT */
#define EBR2_EB4	bit(4) /* 2357F-ZTAT */
#define EBR2_EB3	bit(3) /* 2357F-ZTAT */
#define EBR2_EB2	bit(2) /* 2357F-ZTAT */
#define EBR2_EB1	bit(1) /* 2357F-ZTAT */
#define EBR2_EB0	bit(0) /* 2357F-ZTAT */

#endif /* H8S2357_FLASH_H */
