/*
 * $Id$
 *
 * XScale PXA250/PXA210 Power Manager and Reset Control Registers
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

#ifndef	PXA2X0_PMRC_H
#define	PXA2X0_PMRC_H

#ifndef uint32_t
typedef	unsigned int	uint32_t;
#endif

/* Power Manager and Reset Control Registers */

#define	PMRC_BASE	0x40F00000

typedef volatile struct PMRC_registers {
	uint32_t pmcr;
	uint32_t pssr;
	uint32_t pspr;
	uint32_t pwer;
	uint32_t prer;
	uint32_t pfer;
	uint32_t pedr;
	uint32_t pcfr;
	uint32_t pgsr0;
	uint32_t pgsr1;
	uint32_t pgsr2;
	uint32_t __reserved;
	uint32_t rcsr;
} PMRC_registers;

#ifndef PMRC_pointer
#define	PMRC_pointer	((PMRC_registers*) PMRC_BASE)
#endif

#define	PMCR		PMRC_pointer->pmcr
#define	PSSR		PMRC_pointer->pssr
#define	PSPR		PMRC_pointer->pspr
#define	PWER		PMRC_pointer->pwer
#define	PRER		PMRC_pointer->prer
#define	PFER		PMRC_pointer->pfer
#define	PEDR		PMRC_pointer->pedr
#define	PCFR		PMRC_pointer->pcfr
#define	PGSR0		PMRC_pointer->pgsr0
#define	PGSR1		PMRC_pointer->pgsr1
#define	PGSR2		PMRC_pointer->pgsr2
#define	RCSR		PMRC_pointer->rcsr

#endif	/* PXA2X0_PMRC_H */
