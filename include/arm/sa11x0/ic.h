/*
 * $Id$
 *
 * StrongARM SA-1110 Interrupt Controller Registers
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

#ifndef	SA11X0_IC_H
#define	SA11X0_IC_H

#ifndef uint32_t
typedef	unsigned int	uint32_t;
#endif

/* Interrupt Controller Registers */

#define	IC_BASE		0x90050000

typedef volatile struct IC_registers {
	uint32_t icip;
	uint32_t icmr;
	uint32_t iclr;
	uint32_t iccr;
	uint32_t icfp;
	uint32_t __reserved[3];
	uint32_t icpr;
} IC_registers;

#ifndef IC_pointer
#define	IC_pointer	((IC_registers*) IC_BASE)
#endif

#define	ICIP		IC_pointer->icip
#define	ICMR		IC_pointer->icmr
#define	ICLR		IC_pointer->iclr
#define	ICCR		IC_pointer->iccr
#define	ICFP		IC_pointer->icfp
#define	ICPR		IC_pointer->icpr

#endif	/* SA11X0_IC_H */
