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
 *
 */

#ifndef	PXA2X0_DMA_H
#define	PXA2X0_DMA_H

#include <common.h>

#if LANGUAGE == C
#include <stdint.h>
#endif

/* DMA Controller Registers */

#define	DMA_BASE	0x40000000

#if LANGUAGE == C
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

#ifdef PXA2X0_UNMAPPED
#define	DMA_pointer		((DMA_registers*) DMA_BASE)
#endif

#define	DCSR(i)			DMA_pointer->dcsr[i]
#define	DINT			DMA_pointer->dint
#define	DRCMR(i)		DMA_pointer->drcmr[i]
#define	DDADR(i)		DMA_pointer->dar[i].ddadr
#define	DSADR(i)		DMA_pointer->dar[i].dsadr
#define	DTADR(i)		DMA_pointer->dar[i].dtadr
#define	DCMD(i)			DMA_pointer->dar[i].dcmd
#endif /* LANGUAGE == C */

#define	DCSR_OFFSET(i)		(i << 2)
#define	DINT_OFFSET		0xF0
#define	DRCMR_OFFSET(i)		(0x100 + (i << 2))
#define	DDADR_OFFSET(i)		(0x200 + (i << 4))
#define	DSADR_OFFSET(i)		(0x204 + (i << 4))
#define	DTADR_OFFSET(i)		(0x208 + (i << 4))
#define	DCMD_OFFSET(i)		(0x20C + (i << 4))

/* DCSRx bits - see Table 5-7 in [1] */

#define	DCSR_RUN		bit(31)
#define	DCSR_NODESCFETCH	bit(30)
#define	DCSR_STOPIRQEN		bit(29)
#define	DCSR_REQPEND		bit(8)
#define	DCSR_STOPSTATE		bit(3)
#define	DCSR_ENDINTR		bit(2)
#define	DCSR_STARTINTR		bit(1)
#define	DSCR_BUSERRINTR		bit(0)

/* DRCMRx bits - see Table 5-8 in [1] */

#define	DRCMR_MAPVLD		bit(7)
#define	DRCMR_CHLNUM_MASK	0x0000000F
#define	DRCMR_CHLNUM(x)		(x & DRCMR_CHLNUM_MASK)

/* DDADRx bits - see Table 5-9 in [1] */

#define	DDADR_STOP		bit(0)

/* DCMDx bits - see Table 5-12 in [1] */

#define	DCMD_INCSRCADDR		bit(31)
#define	DCMD_INCTRGADDR		bit(30)
#define	DCMD_FLOWSRC		bit(29)
#define	DCMD_FLOWTRG		bit(28)
#define	DCMD_STARTIRQEN		bit(22)
#define	DCMD_ENDIRQEN		bit(21)
#define	DCMD_ENDIAN		bit(18)
#define	DCMD_SIZE_MASK		0x00030000
#define	DCMD_SIZE(x)		((x << 16) & DCMD_SIZE_MASK)
#define	DCMD_WIDTH_MASK		0x0000C000
#define	DCMD_WIDTH(x)		((x << 14) & DCMD_WIDTH_MASK)
#define	DCMD_LENGTH_MASK	0x00001FFF
#define	DCMD_LENGTH(x)		(x & DCMD_LENGTH_MASK)

#endif /* PXA2X0_DMA_H */
