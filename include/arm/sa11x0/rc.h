/*
 * $Id$
 *
 * StrongARM SA-1110 Reset Controller Registers
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

#ifndef	SA11X0_RC_H
#define	SA11X0_RC_H

#ifndef uint32_t
typedef	unsigned int	uint32_t;
#endif

/* Reset Controller Registers */

#define	RC_BASE		0x90030000

typedef volatile struct RC_registers {
	uint32_t rsrr;
	uint32_t rcsr;
	uint32_t tucr;
} RC_registers;

#ifndef RC_pointer
#define	RC_pointer	((RC_registers*) RC_BASE)
#endif

#define	RSRR		RC_pointer->rsrr
#define	RCSR		RC_pointer->rcsr
#define	TUCR		RC_pointer->tucr

#endif	/* SA11X0_RC_H */
