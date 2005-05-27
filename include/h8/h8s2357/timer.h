/*
 * $Id$
 *
 * H8S/2357 8 bit timer Registers
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

#ifndef H8S2357_TIMER_H
#define H8S2357_TIMER_H

#include <common.h>

#if LANGUAGE == C
#include <stdint.h>
#endif

/* TIMER registers */

#define TIMER_BASE	0xffffffb0

#if LANGUAGE == C
typedef struct TIMER_registers {
	uint8_t tcr0;
	uint8_t tcr1;
	uint8_t tcsr0;
	uint8_t tcsr1;
	uint8_t tcora0;
	uint8_t tcora1;
	uint8_t tcorb0;
	uint8_t tcorb1;
	uint8_t tcnt0;
	uint8_t tcnt1;
} TIMER_registers_t;

#define TIMER_pointer	((TIMER_registers_t*) TIMER_BASE)

#define TCR0		TIMER_pointer->tcr0
#define TCR1		TIMER_pointer->tcr1
#define TCSR0		TIMER_pointer->tcsr0
#define TCSR1		TIMER_pointer->tcsr1
#define TCORA0		TIMER_pointer->tcora0
#define TCORA1		TIMER_pointer->tcora1
#define TCORB0		TIMER_pointer->tcorb0
#define TCORB1		TIMER_pointer->tcorb1
#define TCNT0		TIMER_pointer->tcnt0
#define TCNT1		TIMER_pointer->tcnt1
#endif /* LANGUAGE == C */

#define TCR0_OFFSET	0x00
#define TCR1_OFFSET	0x01
#define TCSR0_OFFSET	0x02
#define TCSR1_OFFSET	0x03
#define TCORA0_OFFSET	0x04
#define TCORA1_OFFSET	0x05
#define TCORB0_OFFSET	0x06
#define TCORB1_OFFSET	0x07
#define TCNT0_OFFSET	0x08
#define TCNT1_OFFSET	0x09

/* TCR bits */
#define TCR_CMIEB	bit(7)
#define TCR_CMIEA	bit(6)
#define TCR_OVIE	bit(5)
#define TCR_CCLR_MASK	bits(4,3)
#define TCR_CCLR(x)	bits_val(4,3,x)
#define get_TCR_CCLR(x)	bits_get(4,3,x)
#define TCR_CKS_MASK	bits(2,0)
#define TCR_CKS(x)	bits_val(2,0,x)
#define get_TCR_CKS(x)	bits_get(2,0,x)

/* TCSR bits */
#define TCSR_CMFB	bit(7)
#define TCSR_CMFA	bit(6)
#define TCSR_OVF	bit(5)
#define TCSR_ADTE	bit(4) /* only for TCSR0 */
#define TCSR_OSB_MASK	bits(3,2)
#define TCSR_OSB(x)	bits_val(3,2,x)
#define get_TCSR_OSB(x)	bits_get(3,2,x)
#define TCSR_OSA_MASK	bits(1,0)
#define TCSR_OSA(x)	bits_val(1,0,x)
#define get_TCSR_OSA(x)	bits_get(1,0,x)

#endif /* H8S2357_TIMER_H */
