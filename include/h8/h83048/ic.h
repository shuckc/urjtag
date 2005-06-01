/*
 * $Id$
 *
 * H8/3048 Interrupt Controller (IC) Registers
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

#ifndef H83048_IC_H
#define H83048_IC_H

#include <common.h>

#if LANGUAGE == C
#include <stdint.h>
#endif

/* IC registers */

#define IC_BASE		0xfffff4

#if LANGUAGE == C
typedef struct IC_registers {
	uint8_t iscr;
	uint8_t ier;
	uint8_t isr;
	uint8_t __reserved;
	uint8_t ipra;
	uint8_t iprb;
} IC_registers_t;

#define IC_pointer	((IC_registers_t*) IC_BASE)

#define ISCR		IC_pointer->iscr
#define IER		IC_pointer->ier
#define ISR		IC_pointer->isr
#define IPRA		IC_pointer->ipra
#define IPRB		IC_pointer->iprb
#endif /* LANGUAGE == C */

#define ISCR_OFFSET	0x00
#define IER_OFFSET	0x01
#define ISR_OFFSET	0x02
#define IPRA_OFFSET	0x04
#define IPRB_OFFSET	0x05

/* ISCR bits */
#define ISCR_IRQ5SC		bit(5)
#define ISCR_IRQ4SC		bit(4)
#define ISCR_IRQ3SC		bit(3)
#define ISCR_IRQ2SC		bit(2)
#define ISCR_IRQ1SC		bit(1)
#define ISCR_IRQ0SC		bit(0)

/* IER bits */
#define IER_IRQ5E		bit(5)
#define IER_IRQ4E		bit(4)
#define IER_IRQ3E		bit(3)
#define IER_IRQ2E		bit(2)
#define IER_IRQ1E		bit(1)
#define IER_IRQ0E		bit(0)

/* ISR bits */
#define ISR_IRQ5F		bit(5)
#define ISR_IRQ4F		bit(4)
#define ISR_IRQ3F		bit(3)
#define ISR_IRQ2F		bit(2)
#define ISR_IRQ1F		bit(1)
#define ISR_IRQ0F		bit(0)

/* IPRA bits */
#define IPRA_IPRA7		bit(7)
#define IPRA_IPRA6		bit(6)
#define IPRA_IPRA5		bit(5)
#define IPRA_IPRA4		bit(4)
#define IPRA_IPRA3		bit(3)
#define IPRA_IPRA2		bit(2)
#define IPRA_IPRA1		bit(1)
#define IPRA_IPRA0		bit(0)

/* IPRB bits */
#define IPRB_IPRB7		bit(7)
#define IPRB_IPRB6		bit(6)
#define IPRB_IPRB5		bit(5)
#define IPRB_IPRB3		bit(3)
#define IPRB_IPRB2		bit(2)
#define IPRB_IPRB1		bit(1)

#endif /* H83048_IC_H */
