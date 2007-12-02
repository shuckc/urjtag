/*
 * $Id$
 *
 * H8S/2357 Interrupt Controller (IC) Registers
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

#ifndef H8S2357_IC_H
#define H8S2357_IC_H

#include <openwince.h>

#if LANGUAGE == C
#include <stdint.h>
#endif

/* interrupt registers */

#define IPR_BASE	0xfffffec4
#define IC_BASE		0xffffff2c

#if LANGUAGE == C
typedef volatile struct IPR_registers {
	uint8_t ipra;
	uint8_t iprb;
	uint8_t iprc;
	uint8_t iprd;
	uint8_t ipre;
	uint8_t iprf;
	uint8_t iprg;
	uint8_t iprh;
	uint8_t ipri;
	uint8_t iprj;
	uint8_t iprk;
} IPR_registers_t;

typedef volatile struct IC_registers {
	uint16_t iscr;
	uint8_t ier;
	uint8_t isr;
} IC_registers_t;

#define IPR_pointer	((IPR_registers_t*) IPR_BASE)
#define IC_pointer	((IC_registers_t*) IC_BASE)

#define IPRA		IPR_pointer->ipra
#define IPRB 		IPR_pointer->iprb
#define IPRC 		IPR_pointer->iprc
#define IPRD 		IPR_pointer->iprd
#define IPRE 		IPR_pointer->ipre
#define IPRF 		IPR_pointer->iprf
#define IPRG 		IPR_pointer->iprg
#define IPRH 		IPR_pointer->iprh
#define IPRI 		IPR_pointer->ipri
#define IPRJ 		IPR_pointer->iprj
#define IPRK 		IPR_pointer->iprk

#define ISCR 		IC_pointer->iscr
#define IER 		IC_pointer->ier
#define ISR 		IC_pointer->isr
#endif /* LANGUAGE == C */

#define IPRA_OFFSET	0x00
#define IPRB_OFFSET	0x01
#define IPRC_OFFSET	0x02
#define IPRD_OFFSET	0x03
#define IPRE_OFFSET	0x04
#define IPRF_OFFSET	0x05
#define IPRG_OFFSET	0x06
#define IPRH_OFFSET	0x07
#define IPRI_OFFSET	0x08
#define IPRJ_OFFSET	0x09
#define IPRK_OFFSET	0x0a

#define ISCR_OFFSET	0x00
#define IER_OFFSET	0x02
#define ISR_OFFSET	0x03

/* IPR bits */
#define IPR_IPR6	bit(6)
#define IPR_IPR5	bit(5)
#define IPR_IPR4	bit(4)
#define IPR_IPR2	bit(2)
#define IPR_IPR1	bit(1)
#define IPR_IPR0	bit(0)

/* ISCR bits */
#define ISCR_IRQ7SCB	bit(15)
#define ISCR_IRQ7SCA	bit(14)
#define ISCR_IRQ6SCB	bit(13)
#define ISCR_IRQ6SCA	bit(12)
#define ISCR_IRQ5SCB	bit(11)
#define ISCR_IRQ5SCA	bit(10)
#define ISCR_IRQ4SCB	bit(9)
#define ISCR_IRQ4SCA	bit(8)
#define ISCR_IRQ3SCB	bit(7)
#define ISCR_IRQ3SCA	bit(6)
#define ISCR_IRQ2SCB	bit(5)
#define ISCR_IRQ2SCA	bit(4)
#define ISCR_IRQ1SCB	bit(3)
#define ISCR_IRQ1SCA	bit(2)
#define ISCR_IRQ0SCB	bit(1)
#define ISCR_IRQ0SCA	bit(0)

/* IER bits */
#define IER_IRQ7E	bit(7)
#define IER_IRQ6E	bit(6)
#define IER_IRQ5E	bit(5)
#define IER_IRQ4E	bit(4)
#define IER_IRQ3E	bit(3)
#define IER_IRQ2E	bit(2)
#define IER_IRQ1E	bit(1)
#define IER_IRQ0E	bit(0)

/* ISR bits */
#define ISR_IRQ7F	bit(7)
#define ISR_IRQ6F	bit(6)
#define ISR_IRQ5F	bit(5)
#define ISR_IRQ4F	bit(4)
#define ISR_IRQ3F	bit(3)
#define ISR_IRQ2F	bit(2)
#define ISR_IRQ1F	bit(1)
#define ISR_IRQ0F	bit(0)

#endif /* H8S2357_IC_H */
