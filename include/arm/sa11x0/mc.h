/*
 * $Id$
 *
 * StrongARM SA-1110 Memory Controller Registers
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

#ifndef	SA11X0_MC_H
#define	SA11X0_MC_H

#ifndef uint32_t
typedef	unsigned int	uint32_t;
#endif

/* Memory Controller Registers */

#define	MC_BASE		0xA0000000

typedef volatile struct MC_registers {
	uint32_t mdcnfg;
	uint32_t mdcas00;
	uint32_t mdcas01;
	uint32_t mdcas02;
	uint32_t msc0;
	uint32_t msc1;
	uint32_t mecr;
	uint32_t mdrefr;
	uint32_t mdcas20;
	uint32_t mdcas21;
	uint32_t mdcas22;
	uint32_t msc2;
	uint32_t smcnfg;
} MC_registers;

#ifndef MC_pointer
#define	MC_pointer	((MC_registers*) MC_BASE)
#endif

#define	MDCNFG		MC_pointer->mdcnfg
#define	MDCAS00		MC_pointer->mdcas00
#define	MDCAS01		MC_pointer->mdcas01
#define	MDCAS02		MC_pointer->mdcas02
#define	MSC0		MC_pointer->msc0
#define	MSC1		MC_pointer->msc1
#define	MECR		MC_pointer->mecr
#define	MDREFR		MC_pointer->mdrefr
#define	MDCAS20		MC_pointer->mdcas20
#define	MDCAS21		MC_pointer->mdcas21
#define	MDCAS22		MC_pointer->mdcas22
#define	MSC2		MC_pointer->msc2
#define	SMCNFG		MC_pointer->smcnfg

#endif	/* SA11X0_MC_H */
