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
 *
 */

#ifndef	PXA2X0_CM_H
#define	PXA2X0_CM_H

#include <common.h>

#if LANGUAGE == C
#include <stdint.h>
#endif

/* Clocks Manager Registers */

#define	CM_BASE		0x41300000

#if LANGUAGE == C
typedef volatile struct CM_registers {
	uint32_t cccr;
	uint32_t cken;
	uint32_t oscc;
} CM_registers;

#ifdef PXA2X0_UNMAPPED
#define	CM_pointer	((CM_registers*) CM_BASE)
#endif

#define	CCCR		CM_pointer->cccr
#define	CKEN		CM_pointer->cken
#define	OSCC		CM_pointer->oscc
#endif /* LANGUAGE == C */

#define	CCCR_OFFSET	0x00
#define	CKEN_OFFSET	0x04
#define	OSCC_OFFSET	0x08

/* CCCR bits - see Table 3-20 in [1] */

#define	CCCR_N_MASK	0x380
#define	CCCR_N(x)	((x << 7) & CCCR_N_MASK)
#define	CCCR_M_MASK	0x060
#define	CCCR_M(x)	((x << 5) & CCCR_M_MASK)
#define	CCCR_L_MASK	0x01F
#define	CCCR_L(x)	(x & CCCR_L_MASK)

#define	CCCR_N_1_0	CCCR_N(0x2)
#define	CCCR_N_1_5	CCCR_N(0x3)
#define	CCCR_N_2_0	CCCR_N(0x4)
#define	CCCR_N_3_0	CCCR_N(0x6)

#define	CCCR_M_1	CCCR_M(0x1)
#define	CCCR_M_2	CCCR_M(0x2)

#define	CCCR_L_27	CCCR_L(0x01)
#define	CCCR_L_32	CCCR_L(0x02)
#define	CCCR_L_36	CCCR_L(0x03)
#define	CCCR_L_40	CCCR_L(0x04)
#define	CCCR_L_45	CCCR_L(0x05)

/* OSCC bits - see Table 3-22 in [1] */

#define	OSCC_OON	bit(1)
#define	OSCC_OOK	bit(0)

#endif /* PXA2X0_CM_H */
