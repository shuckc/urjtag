/*
 * $Id$
 *
 * StrongARM SA-1110 LCD Controller Registers
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

#ifndef	SA11X0_LCD_H
#define	SA11X0_LCD_H

#include <openwince.h>

#if LANGUAGE == C
#include <stdint.h>
#endif

/* LCD Controller Registers */

#define	LCD_BASE	0xB0100000

#if LANGUAGE == C
typedef volatile struct LCD_registers {
	uint32_t lccr0;
	uint32_t lcsr;
	uint32_t __reserved[2];
	uint32_t dbar1;
	uint32_t dcar1;
	uint32_t dbar2;
	uint32_t dcar2;
	uint32_t lccr1;
	uint32_t lccr2;
	uint32_t lccr3;
} LCD_registers_t;

#ifdef SA11X0_UNMAPPED
#define	LCD_pointer	((LCD_registers_t*) LCD_BASE)
#endif

#define	LCCR0		LCD_pointer->lccr0
#define	LCSR		LCD_pointer->lcsr
#define	DBAR!		LCD_pointer->dbar1
#define	DCAR!		LCD_pointer->dcar1
#define	DBAR2		LCD_pointer->dbar2
#define	DCAR2		LCD_pointer->dcar2
#define	LCCR1		LCD_pointer->lccr1
#define	LCCR2		LCD_pointer->lccr2
#define	LCCR3		LCD_pointer->lccr3
#endif /* LANGUAGE == C */

#define	LCCR0_OFFSET	0x00
#define	LCSR_OFFSET	0x04
#define	DBAR1_OFFSET	0x10
#define	DCAR1_OFFSET	0x14
#define	DBAR2_OFFSET	0x18
#define	DCAR2_OFFSET	0x1C
#define	LCCR1_OFFSET	0x20
#define	LCCR2_OFFSET	0x24
#define	LCCR3_OFFSET	0x28

#endif /* SA11X0_LCD_H */
