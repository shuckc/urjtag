/*
 * $Id$
 *
 * StrongARM SA-1110 SSP Registers
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

#ifndef	SA11X0_SSP_H
#define	SA11X0_SSP_H

#include <common.h>

#if LANGUAGE == C
#include <stdint.h>
#endif

/* SSP Registers (Serial Port 4) */

#define	SSP_BASE	0x80070060

#if LANGUAGE == C
typedef volatile struct SSP_registers {
	uint32_t sscr0;
	uint32_t sscr1;
	uint32_t __reserved1;
	uint32_t ssdr;
	uint32_t __reserved2;
	uint32_t sssr;
} SSP_registers;

#ifdef SA11X0_UNMAPPED
#define	SSP_pointer	((SSP_registers*) SSP_BASE)
#endif

#define	SSCR0		SSP_pointer->sscr0
#define	SSCR1		SSP_pointer->sscr1
#define	SSDR		SSP_pointer->ssdr
#define	SSSR		SSP_pointer->sssr
#endif /* LANGUAGE == C */

#define	SSCR0_OFFSET	0x00
#define	SSCR1_OFFSET	0x04
#define	SSDR_OFFSET	0x0C
#define	SSSR_OFFSET	0x14

/* SSCR0 bits */

#define	SSCR0_SCR_MASK	0xFF00
#define	SSCR0_SCR(x)	((x << 8) & SSCR0_SCR_MASK)
#define	SSCR0_SSE	bit(7)
#define	SSCR0_FRF_MASK	0x0030
#define	SSCR0_FRF(x)	((x << 4) & SSCR0_FRF_MASK)
#define	SSCR0_DSS_MASK	0x000F
#define	SSCR0_DSS(x)	(x & SSCR0_DSS_MASK)

/* SSCR1 bits */

#define	SSCR1_ECS	bit(5)
#define	SSCR1_SPH	bit(4)
#define	SSCR1_SPO	bit(3)
#define	SSCR1_LBM	bit(2)
#define	SSCR1_TIE	bit(1)
#define	SSCR1_RIE	bit(0)

/* SSSR bits */

#define	SSSR_ROR	bit(6)
#define	SSSR_RFS	bit(5)
#define	SSSR_TFS	bit(4)
#define	SSSR_BSY	bit(3)
#define	SSSR_RNE	bit(2)
#define	SSSR_TNF	bit(1)

#endif /* SA11X0_SSP_H */
