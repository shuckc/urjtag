/*
 * $Id$
 *
 * StrongARM SA-1110 GPCLK Registers
 * Copyright (C) 2002 ETC s.r.o.
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
 * 3. Neither the name of the ETC s.r.o. nor the names of its contributors
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
 * Written by Marcel Telka <marcel@telka.sk>, 2002.
 *
 * Documentation:
 * [1] Intel Corporation, "Intel StrongARM SA-1110 Microprocessor
 *     Developer's Manual", October 2001, Order Number: 278240-004
 *
 */

#ifndef	SA11X0_GPCLK_H
#define	SA11X0_GPCLK_H

#include <common.h>

#if LANGUAGE == C
#include <stdint.h>
#endif

/* GPCLK Registers (Serial Port 1) */

#define	GPCLK_BASE	0x80020060

#if LANGUAGE == C
typedef volatile struct GPCLK_registers {
	uint32_t gpclkr0;
	uint32_t gpclkr1;
	uint32_t __reserved;
	uint32_t gpclkr2;
	uint32_t gpclkr3;
} GPCLK_registers;

#ifdef SA11X0_UNMAPPED
#define	GPCLK_pointer	((GPCLK_registers*) GPCLK_BASE)
#endif

#define	GPCLKR0		GPCLK_pointer->gpclkr0
#define	GPCLKR1		GPCLK_pointer->gpclkr1
#define	GPCLKR2		GPCLK_pointer->gpclkr2
#define	GPCLKR3		GPCLK_pointer->gpclkr3
#endif /* LANGUAGE == C */

#define	GPCLKR0_OFFSET	0x00
#define	GPCLKR1_OFFSET	0x04
#define	GPCLKR2_OFFSET	0x0C
#define	GPCLKR3_OFFSET	0x10

/* GPCLKR0 bits */

#define	GPCLKR0_SCD	bit(5)
#define	GPCLKR0_SCE	bit(4)
#define	GPCLKR0_SUS	bit(0)

/* GPCLKR1 bits */

#define	GPCLKR1_TXE	bit(1)

#endif /* SA11X0_GPCLK_H */
