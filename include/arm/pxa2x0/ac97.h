/*
 * $Id$
 *
 * XScale PXA250/PXA210 AC97 Registers
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
 * [2] Intel Corporation, "Intel PXA250 and PXA210 Application Processors
 *     Specification Update", May 2002, Order Number: 278534-005
 *
 */

#ifndef	PXA2X0_AC97_H
#define	PXA2X0_AC97_H

#ifndef uint32_t
typedef	unsigned int	uint32_t;
#endif

/* AC97 Registers */

#define	AC97_BASE	0x40500000

typedef volatile struct AC97_registers {
	uint32_t pocr;
	uint32_t picr;
	uint32_t mccr;
	uint32_t gcr;
	uint32_t posr;
	uint32_t pisr;
	uint32_t mcsr;
	uint32_t gsr;
	uint32_t car;
	uint32_t __reserved1[7];
	uint32_t pcdr;
	uint32_t __reserved2[7];
	uint32_t mcdr;
	uint32_t __reserved3[39];
	uint32_t mocr;
	uint32_t __reserved4;
	uint32_t micr;
	uint32_t __reserved5;
	uint32_t mosr;
	uint32_t __reserved5;
	uint32_t misr;
	uint32_t __reserved6[9];
	uint32_t modr;
	uint32_t __reserved7[47];
	uint32_t __pacr[64];		/* Primary Audio codec Registers */
	uint32_t __sacr[64];		/* Secondary Audio codec Registers */
	uint32_t __pmcr[64];		/* Primary Modem codec Registers */
	uint32_t __smcr[64];		/* Secondary Modem codec Registers */
} AC97_registers;

#ifndef AC97_pointer
#define	AC97_pointer	((AC97_registers*) AC97_BASE)
#endif

#define	POCR		AC97_pointer->pocr
#define	PICR		AC97_pointer->picr
#define	MCCR		AC97_pointer->mccr
#define	GCR		AC97_pointer->gcr
#define	POSR		AC97_pointer->posr
#define	PISR		AC97_pointer->pisr
#define	MCSR		AC97_pointer->mcsr
#define	GSR		AC97_pointer->gsr
#define	CAR		AC97_pointer->car
#define	PCDR		AC97_pointer->pcdr
#define	MCDR		AC97_pointer->mcdr
#define	MOCR		AC97_pointer->mocr
#define	MICR		AC97_pointer->micr
#define	MOSR		AC97_pointer->mosr
#define	MISR		AC97_pointer->misr
#define	MODR		AC97_pointer->modr

#endif	/* PXA2X0_AC97_H */
