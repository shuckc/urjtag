/*
 * $Id$
 *
 * StrongARM SA-1110 LCD Controller Registers
 * Copyright (C) 2002 ETC s.r.o.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 *
 * Written by Marcel Telka <marcel@telka.sk>, 2002.
 *
 * Documentation:
 * [1] Intel Corporation, "Intel StrongARM SA-1110 Microprocessor
 *     Developer's Manual", October 2001, Order Number: 278240-004
 * [2] Intel Corporation, "Intel StrongARM SA-1110 Microprocessor
 *     Specification Update", December 2001, Order Number: 278259-023
 *
 */

#ifndef	SA11X0_LCD_H
#define	SA11X0_LCD_H

#include <common.h>

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
} LCD_registers;

#ifdef SA11X0_UNMAPPED
#define	LCD_pointer	((LCD_registers*) LCD_BASE)
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
