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

#include <common.h>

/* ICP - HSSP Registers (Serial Port 2) */

#define	HSSP_BASE	0x80040060

#if LANGUAGE == C
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
#endif /* LANGUAGE == C */

#define	HSCR0_OFFSET	0x00
#define	HSCR1_OFFSET	0x04
#define	HSDR_OFFSET	0x0C
#define	HSSR0_OFFSET	0x14
#define	HSSR1_OFFSET	0x18

/* HSCR0 bits */

#define	HSCR0_AME	bit(7)
#define	HSCR0_TIE	bit(6)
#define	HSCR0_RIE	bit(5)
#define	HSCR0_RXE	bit(4)
#define	HSCR0_TXE	bit(3)
#define	HSCR0_TUS	bit(2)
#define	HSCR0_LBM	bit(1)
#define	HSCR0_ITR	bit(0)

/* HSSR0 bits */

#define	HSSR0_FRE	bit(5)
#define	HSSR0_RFS	bit(4)
#define	HSSR0_TFS	bit(3)
#define	HSSR0_RAB	bit(2)
#define	HSSR0_TUR	bit(1)
#define	HSSR0_EIF	bit(0)

/* HSSR1 bits */

#define	HSSR1_ROR	bit(6)
#define	HSSR1_CRE	bit(5)
#define	HSSR1_EOF	bit(4)
#define	HSSR1_TNF	bit(3)
#define	HSSR1_RNE	bit(2)
#define	HSSR1_TBY	bit(1)
#define	HSSR1_RSY	bit(0)

#endif /* SA11X0_HSSP_H */
