/*
 * $Id$
 *
 * XScale PXA250/PXA210 ICP Registers
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
 * [2] Intel Corporation, "Intel PXA250 and PXA210 Application Processors
 *     Specification Update", April 2002, Order Number: 278534-004
 *
 */

#ifndef	PXA2X0_ICP_H
#define	PXA2X0_ICP_H

#ifndef uint32_t
typedef	unsigned int	uint32_t;
#endif

/* ICP Registers */

#define	ICP_BASE	0x40800000

typedef volatile struct ICP_registers {
	uint32_t iccr0;
	uint32_t iccr1;
	uint32_t iccr2;
	uint32_t icdr;
	uint32_t __reserved;
	uint32_t icsr0;
	uint32_t icsr1;
} ICP_registers;

#ifndef ICP_pointer
#define	ICP_pointer	((ICP_registers*) ICP_BASE)
#endif

#define	ICCR0		ICP_pointer->iccr0
#define	ICCR1		ICP_pointer->iccr1
#define	ICCR2		ICP_pointer->iccr2
#define	ICDR		ICP_pointer->icdr
#define	ICSR0		ICP_pointer->icsr0
#define	ICSR1		ICP_pointer->icsr1

#endif	/* PXA2X0_ICP_H */
