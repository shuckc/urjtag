/*
 * $Id$
 *
 * StrongARM SA-1110 ICP - HSSP Registers
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

#ifndef	SA11X0_HSSP_H
#define	SA11X0_HSSP_H

#ifndef uint32_t
typedef	unsigned int	uint32_t;
#endif

/* ICP - HSSP Registers (Serial Port 2) */

#define	HSSP_BASE	0x80040060

typedef volatile struct HSSP_registers {
	uint32_t hscr0;
	uint32_t hscr1;
	uint32_t __reserved1;
	uint32_t hsdr;
	uint32_t __reserved2;
	uint32_t hssr0;
	uint32_t hssr1;
} HSSP_registers;

#ifndef HSSP_pointer
#define	HSSP_pointer	((HSSP_registers*) HSSP_BASE)
#endif

#define	HSCR0		HSSP_pointer->hscr0
#define	HSCR1		HSSP_pointer->hscr1
#define	HSDR		HSSP_pointer->hsdr
#define	HSSR0		HSSP_pointer->hssr0
#define	HSSR1		HSSP_pointer->hssr1

#endif	/* SA11X0_HSSP_H */
