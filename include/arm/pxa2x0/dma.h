/*
 * $Id$
 *
 * XScale PXA250/PXA210 DMA Controller Registers
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
 *     Specification Update", April 2002, Order Number: 278534-004
 *
 */

#ifndef	PXA2X0_DMA_H
#define	PXA2X0_DMA_H

#ifndef uint32_t
typedef	unsigned int	uint32_t;
#endif

/* DMA Controller Registers */

#define	DMA_BASE	0x40000000

typedef struct _DMA_dar {
	uint32_t ddadr;
	uint32_t dsadr;
	uint32_t dtadr;
	uint32_t dcmd;
} _DMA_dar;

typedef volatile struct DMA_registers {
	uint32_t dcsr[16];
	uint32_t __reserved1[44];
	uint32_t dint;
	uint32_t __reserved2[3];
	uint32_t drcmr[40];
	uint32_t __reserved3[24];
	_DMA_dar dar[16];
} DMA_registers;

#ifndef DMA_pointer
#define	DMA_pointer	((DMA_registers*) DMA_BASE)
#endif

#define	DCSR(i)		DMA_pointer->dcsr[i]
#define	DINT		DMA_pointer->dint
#define	DRCMR(i)	DMA_pointer->drcmr[i]
#define	DDADR(i)	DMA_pointer->dar[i].ddadr
#define	DSADR(i)	DMA_pointer->dar[i].dsadr
#define	DTADR(i)	DMA_pointer->dar[i].dtadr
#define	DCMD(i)		DMA_pointer->dar[i].dcmd

#endif	/* PXA2X0_DMA_H */
