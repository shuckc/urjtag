/*
 * $Id$
 *
 * StrongARM SA-1110 PPC Registers
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

#ifndef	SA11X0_PPC_H
#define	SA11X0_PPC_H

#ifndef uint32_t
typedef	unsigned int	uint32_t;
#endif

/* PPC Registers */

#define	PPC_BASE	0x90060000

typedef volatile struct PPC_registers {
	uint32_t ppdr;
	uint32_t ppsr;
	uint32_t ppar;
	uint32_t psdr;
	uint32_t ppfr;
	uint32_t __reserved1[5];
	uint32_t hscr2;
	uint32_t __reserved2;
	uint32_t mccr1;
} PPC_registers;

#ifndef PPC_pointer
#define	PPC_pointer	((PPC_registers*) PPC_BASE)
#endif

#define	PPDR		PPC_pointer->ppdr
#define	PPSR		PPC_pointer->ppsr
#define	PPAR		PPC_pointer->ppar
#define	PSDR		PPC_pointer->psdr
#define	PPFR		PPC_pointer->ppfr
#define	HSCR2		PPC_pointer->hscr2
#define	MCCR1		PPC_pointer->mccr1

#endif	/* SA11X0_PPC_H */
