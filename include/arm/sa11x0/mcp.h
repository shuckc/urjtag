/*
 * $Id$
 *
 * StrongARM SA-1110 MCP Registers
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

#ifndef	SA11X0_MCP_H
#define	SA11X0_MCP_H

#include <common.h>

#if LANGUAGE == C
#include <stdint.h>
#endif

/* MCP Registers (Serial Port 4) */

#define	MCP_BASE	0x80060000

#if LANGUAGE == C
typedef volatile struct MCP_registers {
	uint32_t mccr0;
	uint32_t __reserved1;
	uint32_t mcdr0;
	uint32_t mcdr1;
	uint32_t mcdr2;
	uint32_t __reserved2;
	uint32_t mcsr;
} MCP_registers;

#ifdef SA11X0_UNMAPPED
#define	MCP_pointer	((MCP_registers*) MCP_BASE)
#endif

#define	MCCR0		MCP_pointer->mccr0
#define	MCDR0		MCP_pointer->mcdr0
#define	MCDR1		MCP_pointer->mcdr1
#define	MCDR2		MCP_pointer->mcdr2
#define	MCSR		MCP_pointer->mcsr
#endif /* LANGUAGE == C */

#define	MCCR0_OFFSET	0x00
#define	MCDR0_OFFSET	0x08
#define	MCDR1_OFFSET	0x0C
#define	MCDR2_OFFSET	0x10
#define	MCSR_OFFSET	0x18

#endif /* SA11X0_MCP_H */
