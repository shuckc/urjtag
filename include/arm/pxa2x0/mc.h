/*
 * $Id$
 *
 * XScale PXA250/PXA210 Memory Controller Registers
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

#ifndef	PXA2X0_MC_H
#define	PXA2X0_MC_H

#ifndef uint32_t
typedef	unsigned int	uint32_t;
#endif

/* Memory Controller Registers */

#define	MC_BASE		0x48000000

typedef volatile struct MC_registers {
	uint32_t mdcnfg;
	uint32_t mdrefr;
	uint32_t msc0;
	uint32_t msc1;
	uint32_t msc2;
	uint32_t mecr;
	uint32_t __reserved1;
	uint32_t sxcnfg;
	uint32_t __reserved2;
	uint32_t sxmrs;
	uint32_t mcmem0;
	uint32_t mcmem1;
	uint32_t mcatt0;
	uint32_t mcatt1;
	uint32_t mcio0;
	uint32_t mcio1;
	uint32_t mdmrs;
	uint32_t boot_def;
} MC_registers;

#ifndef MC_pointer
#define	MC_pointer	((MC_registers*) MC_BASE)
#endif

#define	MDCNFG		MC_pointer->mdcnfg
#define	MDREFR		MC_pointer->mdrefr
#define	MSC0		MC_pointer->msc0
#define	MSC1		MC_pointer->msc1
#define	MSC2		MC_pointer->msc2
#define	MECR		MC_pointer->mecr
#define	SXCNFG		MC_pointer->sxcnfg
#define	SXMRS		MC_pointer->sxmrs
#define	MCMEM0		MC_pointer->mcmem0
#define	MCMEM1		MC_pointer->mcmem1
#define	MCATT0		MC_pointer->mcatt0
#define	MCATT1		MC_pointer->mcatt1
#define	MCIO0		MC_pointer->mcio0
#define	MCIO1		MC_pointer->mcio1
#define	MDMRS		MC_pointer->mdmrs
#define	BOOT_DEF	MC_pointer->boot_def

#endif	/* PXA2X0_MC_H */
