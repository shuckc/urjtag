/*
 * $Id$
 *
 * XScale PXA250/PXA210 RTC Registers
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

#ifndef	PXA2X0_RTC_H
#define	PXA2X0_RTC_H

#ifndef uint32_t
typedef	unsigned int	uint32_t;
#endif

/* RTC Registers */

#define	RTC_BASE	0x40900000

typedef volatile struct RTC_registers {
	uint32_t rcnr;
	uint32_t rtar;
	uint32_t rtsr;
	uint32_t rttr;
} RTC_registers;

#ifndef RTC_pointer
#define	RTC_pointer	((RTC_registers*) RTC_BASE)
#endif

#define	RCNR		RTC_pointer->rcnr
#define	RTAR		RTC_pointer->rtar
#define	RTSR		RTC_pointer->rtsr
#define	RTTR		RTC_pointer->rttr

#endif	/* PXA2X0_RTC_H */
