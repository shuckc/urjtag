/*
 * $Id$
 *
 * XScale PXA250/PXA210 OS Timer Registers
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

#ifndef	PXA2X0_OST_H
#define	PXA2X0_OST_H

#ifndef uint32_t
typedef	unsigned int	uint32_t;
#endif

/* OS Timer Registers */

#define	OST_BASE	0x40A00000

typedef volatile struct OST_registers {
	uint32_t osmr[4];
	uint32_t oscr;
	uint32_t ossr;
	uint32_t ower;
	uint32_t oier;
} OST_registers;

#ifndef OST_pointer
#define	OST_pointer	((OST_registers*) OST_BASE)
#endif

#define	OSMR(i)		OST_pointer->osmr[i]
#define	OSCR		OST_pointer->oscr
#define	OSSR		OST_pointer->ossr
#define	OWER		OST_pointer->ower
#define	OIER		OST_pointer->oier

#endif	/* PXA2X0_OST_H */
