/*
 * $Id$
 *
 * StrongARM SA-1110 GPCLK Registers
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
 *
 */

#ifndef	SA11X0_GPCLK_H
#define	SA11X0_GPCLK_H

#include <common.h>

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

#ifndef GPCLK_pointer
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
