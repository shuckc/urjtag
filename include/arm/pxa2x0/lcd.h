/*
 * $Id$
 *
 * XScale PXA250/PXA210 LCD Controller Registers
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
 * [1] Intel Corporation, "Intel PXA250 and PXA210 Application Processors
 *     Developer's Manual", February 2002, Order Number: 278522-001
 *
 */

#ifndef	PXA2X0_LCD_H
#define	PXA2X0_LCD_H

#ifndef uint32_t
typedef	unsigned int	uint32_t;
#endif

/* LCD Controller Registers */

#define	LCD_BASE	0x44000000

typedef volatile struct LCD_registers {
	uint32_t lccr[4];
	uint32_t __reserved1[4];
	uint32_t fbr0;
	uint32_t fbr1;
	uint32_t __reserved2[4];
	uint32_t lcsr;
	uint32_t liidr;
	uint32_t trgbr;
	uint32_t tcr;
	uint32_t __reserved3[110];
	uint32_t fdadr0;
	uint32_t fsadr0;
	uint32_t fidr0;
	uint32_t ldcmd0;
	uint32_t fdadr1;
	uint32_t fsadr1;
	uint32_t fidr1;
	uint32_t ldcmd1;
} LCD_registers;

#ifndef LCD_pointer
#define	LCD_pointer	((LCD_registers*) LCD_BASE)
#endif

#define	LCCR(i)		LCD_pointer->lccr[i]
#define	LCCR0		LCCR(0)
#define	LCCR1		LCCR(1)
#define	LCCR2		LCCR(2)
#define	LCCR3		LCCR(3)
#define	FDADR0		LCD_pointer->fdadr0
#define	FSADR0		LCD_pointer->fsadr0
#define	FIDR0		LCD_pointer->fidr0
#define	LDCMD0		LCD_pointer->ldcmd0
#define	FDADR1		LCD_pointer->fdadr1
#define	FSADR1		LCD_pointer->fsadr1
#define	FIDR1		LCD_pointer->fidr1
#define	LDCMD1		LCD_pointer->ldcmd1
#define	FBR0		LCD_pointer->fbr0
#define	FBR1		LCD_pointer->fbr1
#define	LCSR		LCD_pointer->lcsr
#define	LIIDR		LCD_pointer->liidr
#define	TRGBR		LCD_pointer->trgbr
#define	TCR		LCD_pointer->tcr

#endif	/* PXA2X0_LCD_H */
