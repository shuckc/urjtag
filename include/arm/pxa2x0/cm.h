/*
 * $Id$
 *
 * XScale PXA250/PXA210 Clocks Manager Registers
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
 *     Specification Update", May 2002, Order Number: 278534-005
 *
 */

#ifndef	PXA2X0_CM_H
#define	PXA2X0_CM_H

#ifndef uint32_t
typedef	unsigned int	uint32_t;
#endif

/* Clocks Manager Registers */

#define	CM_BASE		0x41300000

typedef volatile struct CM_registers {
	uint32_t cccr;
	uint32_t cken;
	uint32_t oscc;
} CM_registers;

#ifndef CM_pointer
#define	CM_pointer	((CM_registers*) CM_BASE)
#endif

#define	CCCR		CM_pointer->cccr
#define	CKEN		CM_pointer->cken
#define	OSCC		CM_pointer->oscc

#endif	/* PXA2X0_CM_H */
