/*
 * $Id$
 *
 * XScale PXA250/PXA210 SSP Registers
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

#ifndef	PXA2X0_SSP_H
#define	PXA2X0_SSP_H

#ifndef uint32_t
typedef	unsigned int	uint32_t;
#endif

/* SSP Registers */

#define	SSP_BASE	0x41000000

typedef volatile struct SSP_registers {
	uint32_t sscr0;
	uint32_t sscr1;
	uint32_t sssr;
	uint32_t ssitr;
	uint32_t ssdr;
} SSP_registers;

#ifndef SSP_pointer
#define	SSP_pointer	((SSP_registers*) SSP_BASE)
#endif

#define	SSCR0		SSP_pointer->sscr0
#define	SSCR1		SSP_pointer->sscr1
#define	SSSR		SSP_pointer->sssr
#define	SSITR		SSP_pointer->ssitr
#define	SSDR		SSP_pointer->ssdr

#endif	/* PXA2X0_SSP_H */
