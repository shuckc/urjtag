/*
 * $Id$
 *
 * XScale PXA26x/PXA250/PXA210 DMA Controller Registers
 * Copyright (C) 2002 ETC s.r.o.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the ETC s.r.o. nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Written by Marcel Telka <marcel@telka.sk>, 2002.
 *
 * Documentation:
 * [1] Intel Corporation, "Intel PXA250 and PXA210 Application Processors
 *     Developer's Manual", February 2002, Order Number: 278522-001
 * [2] Intel Corporation, "Intel PXA26x Processor Family Developer's Manual",
 *     October 2002, Order Number: 278638-001
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
} _DMA_dar_t;

typedef volatile struct DMA_registers {
	uint32_t dcsr[16];
	uint32_t __reserved1[44];
	uint32_t dint;
	uint32_t __reserved2[3];
	uint32_t drcmr[40];
	uint32_t __reserved3[24];
	_DMA_dar_t dar[16];
} DMA_registers_t;

#ifdef PXA2X0_UNMAPPED
#define	DMA_pointer		((DMA_registers_t*) DMA_BASE)
#endif

#define	DCSR(i)			DMA_pointer->dcsr[i]
#define	DINT			DMA_pointer->dint
#define	DRCMR(i)		DMA_pointer->drcmr[i]
#define	DDADR(i)		DMA_pointer->dar[i].ddadr
#define	DSADR(i)		DMA_pointer->dar[i].dsadr
#define	DTADR(i)		DMA_pointer->dar[i].dtadr
#define	DCMD(i)			DMA_pointer->dar[i].dcmd
#endif /* LANGUAGE == C */

#define	DCSR_OFFSET(i)		((i) << 2)
#define	DINT_OFFSET		0xF0
#define	DRCMR_OFFSET(i)		(0x100 + ((i) << 2))
#define	DDADR_OFFSET(i)		(0x200 + ((i) << 4))
#define	DSADR_OFFSET(i)		(0x204 + ((i) << 4))
#define	DTADR_OFFSET(i)		(0x208 + ((i) << 4))
#define	DCMD_OFFSET(i)		(0x20C + ((i) << 4))

/* DCSRx bits - see Table 5-7 in [1], Table 5-7 in [2] */

#define	DCSR_RUN		bit(31)
#define	DCSR_NODESCFETCH	bit(30)
#define	DCSR_STOPIRQEN		bit(29)
#define	DCSR_REQPEND		bit(8)
#define	DCSR_STOPSTATE		bit(3)
#define	DCSR_ENDINTR		bit(2)
#define	DCSR_STARTINTR		bit(1)
#define	DSCR_BUSERRINTR		bit(0)

/* DRCMRx bits - see Table 5-8 in [1], Table 5-8 in [2] */

#define	DRCMR_MAPVLD		bit(7)
#define	DRCMR_CHLNUM_MASK	bits(3,0)
#define	DRCMR_CHLNUM(x)		bits_val(3,0,x)

/* DDADRx bits - see Table 5-9 in [1], Table 5-9 in [2] */

#define	DDADR_STOP		bit(0)

/* DCMDx bits - see Table 5-12 in [1], Table 5-12 in [2] */

#define	DCMD_INCSRCADDR		bit(31)
#define	DCMD_INCTRGADDR		bit(30)
#define	DCMD_FLOWSRC		bit(29)
#define	DCMD_FLOWTRG		bit(28)
#define	DCMD_STARTIRQEN		bit(22)
#define	DCMD_ENDIRQEN		bit(21)
#define	DCMD_ENDIAN		bit(18)
#define	DCMD_SIZE_MASK		bits(17,16)
#define	DCMD_SIZE(x)		bits_val(17,16,x)
#define	DCMD_WIDTH_MASK		bits(15,14)
#define	DCMD_WIDTH(x)		bits_val(15,14,x)
#define	DCMD_LENGTH_MASK	bits(12,0)
#define	DCMD_LENGTH(x)		bits_val(12,0,x)

#endif /* PXA2X0_DMA_H */
