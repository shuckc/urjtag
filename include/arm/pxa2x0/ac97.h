/*
 * $Id$
 *
 * XScale PXA26x/PXA255/PXA250/PXA210 AC97 Registers
 * Copyright (C) 2002, 2003 ETC s.r.o.
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
 * Written by Marcel Telka <marcel@telka.sk>, 2002, 2003.
 *
 * Documentation:
 * [1] Intel Corporation, "Intel PXA250 and PXA210 Application Processors
 *     Developer's Manual", February 2002, Order Number: 278522-001
 * [2] Intel Corporation, "Intel PXA26x Processor Family Developer's Manual",
 *     March 2003, Order Number: 278638-002
 * [3] Intel Corporation, "Intel PXA255 Processor Developer's Manual"
 *     March 2003, Order Number: 278693-001
 *
 */

#ifndef	PXA2X0_AC97_H
#define	PXA2X0_AC97_H

#include <common.h>

#if LANGUAGE == C
#include <stdint.h>
#endif

#if defined(PXA2X0_NOPXA250) && !defined(PXA2X0_NOPXA255)
#define PXA2X0_NOPXA255
#endif

#if defined(PXA2X0_NOPXA255) && !defined(PXA2X0_NOPXA260)
#define PXA2X0_NOPXA260
#endif

/* AC97 Registers */

#define	AC97_BASE	0x40500000

#if LANGUAGE == C
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
	uint32_t __reserved6;
	uint32_t misr;
	uint32_t __reserved7[9];
	uint32_t modr;
	uint32_t __reserved8[47];
	uint32_t __pacr[64];		/* Primary Audio codec Registers */
	uint32_t __sacr[64];		/* Secondary Audio codec Registers */
	uint32_t __pmcr[64];		/* Primary Modem codec Registers */
	uint32_t __smcr[64];		/* Secondary Modem codec Registers */
} AC97_registers_t;

#ifdef PXA2X0_UNMAPPED
#define	AC97_pointer		((AC97_registers_t*) AC97_BASE)
#endif

#define	POCR			AC97_pointer->pocr
#define	PICR			AC97_pointer->picr
#define	MCCR			AC97_pointer->mccr
#define	GCR			AC97_pointer->gcr
#define	POSR			AC97_pointer->posr
#define	PISR			AC97_pointer->pisr
#define	MCSR			AC97_pointer->mcsr
#define	GSR			AC97_pointer->gsr
#define	CAR			AC97_pointer->car
#define	PCDR			AC97_pointer->pcdr
#define	MCDR			AC97_pointer->mcdr
#define	MOCR			AC97_pointer->mocr
#define	MICR			AC97_pointer->micr
#define	MOSR			AC97_pointer->mosr
#define	MISR			AC97_pointer->misr
#define	MODR			AC97_pointer->modr
#define	__PACR(r)		AC97_pointer->__pacr[(r) >> 1]
#define	__SACR(r)		AC97_pointer->__sacr[(r) >> 1]
#define	__PMCR(r)		AC97_pointer->__pmcr[(r) >> 1]
#define	__SMCR(r)		AC97_pointer->__smcr[(r) >> 1]
#endif /* LANGUAGE == C */

#define	POCR_OFFSET		0x000
#define	PICR_OFFSET		0x004
#define	MCCR_OFFSET		0x008
#define	GCR_OFFSET		0x00C
#define	POSR_OFFSET		0x010
#define	PISR_OFFSET		0x014
#define	MCSR_OFFSET		0x018
#define	GSR_OFFSET		0x01C
#define	CAR_OFFSET		0x020
#define	PCDR_OFFSET		0x040
#define	MCDR_OFFSET		0x060
#define	MOCR_OFFSET		0x100
#define	MICR_OFFSET		0x108
#define	MOSR_OFFSET		0x110
#define	MISR_OFFSET		0x118
#define	MODR_OFFSET		0x140

/* POCR bits - see Table 13-50 in [1], Table 13-10 in [2], Table 13-9 in [3] */

#define	POCR_FEIE		bit(3)

/* PICR bits - see Table 13-51 in [1], Table 13-11 in [2], Table 13-10 in [3] */

#define	PICR_FEIE		bit(3)

/* MCCR bits - see Table 13-56 in [1], Table 13-16 in [2], Table 13-15 in [3] */

#define	MCCR_FEIE		bit(3)

/* GCR bits - see Table 13-48 in [1], Table 13-8 in [2], Table 13-7 in [3] */

#define	GCR_CDONE_IE		bit(19)
#define	GCR_SDONE_IE		bit(18)
#define	GCR_SECRDY_IEN		bit(9)
#define	GCR_PRIRDY_IEN		bit(8)
#define	GCR_SECRES_IEN		bit(5)
#define	GCR_PRIRES_IEN		bit(4)
#define	GCR_ACLINK_OFF		bit(3)
#define	GCR_WARM_RST		bit(2)
#define	GCR_COLD_RST		bit(1)
#define	GCR_GIE			bit(0)

/* POSR bits - see Table 13-52 in [1], Table 13-12 in [2], Table 13-11 in [3] */

#define	POSR_FIFOE		bit(4)

/* PISR bits - see Table 13-53 in [1], Table 13-13 in [2], Table 13-12 in [3] */

#define	PISR_FIFOE		bit(4)

/* MCSR bits - see Table 13-57 in [1], Table 13-17 in [2], Table 13-16 in [3] */

#define	MCSR_FIFOE		bit(4)

/* GSR bits - see Table 13-49 in [1], Table 13-9 in [2], Table 13-8 in [3] */

#define	GSR_CDONE		bit(19)
#define	GSR_SDONE		bit(18)
#define	GSR_RDCS		bit(15)
#define	GSR_BIT3SLT12		bit(14)
#define	GSR_BIT2SLT12		bit(13)
#define	GSR_BIT1SLT12		bit(12)
#define	GSR_SECRES		bit(11)
#define	GSR_PRIRES		bit(10)
#define	GSR_SCR			bit(9)
#define	GSR_PCR			bit(8)
#define	GSR_MINT		bit(7)
#define	GSR_POINT		bit(6)
#define	GSR_PIINT		bit(5)
#define	GSR_MOINT		bit(2)
#define	GSR_MIINT		bit(1)
#define	GSR_GSCI		bit(0)

/* CAR bits - see Table 13-54 in [1], Table 13-14 in [2], Table 13-13 in [3] */

#define	CAR_CAIP		bit(0)

/* PCDR bits - see Table 13-55 in [1], Table 13-15 in [2], Table 13-14 in [3] */

#define	PCDR_PCM_RDATA_MASK	bits(31,16)
#define	PCDR_PCM_RDATA(x)	bits_val(31,16,x)
#define	get_PCDR_PCM_RDATA(x)	bits_get(31,16,x)
#define	PCDR_PCM_LDATA_MASK	bits(15,0)
#define	PCDR_PCM_LDATA(x)	bits_val(15,0,x)
#define	get_PCDR_PCM_LDATA(x)	bits_get(15,0,x)

/* MCDR bits - see Table 13-58 in [1], Table 13-18 in [2], Table 13-17 in [3] */

#define	MCDR_MIC_IN_DAT_MASK	bits(15,0)
#define	MCDR_MIC_IN_DAT(x)	bits_val(15,0,x)
#define	get_MCDR_MIC_IN_DAT(x)	bits_get(15,0,x)

/* MOCR bits - see Table 13-59 in [1], Table 13-19 in [2], Table 13-18 in [3] */

#define	MOCR_FEIE		bit(3)

/* MICR bits - see Table 13-60 in [1], Table 13-20 in [2], Table 13-19 in [3] */

#define	MICR_FEIE		bit(3)

/* MOSR bits - see Table 13-61 in [1], Table 13-21 in [2], Table 13-20 in [3] */

#define	MOSR_FIFOE		bit(4)

/* MISR bits - see Table 13-62 in [1], Table 13-22 in [2], Table 13-21 in [3] */

#define	MISR_FIFOE		bit(4)

/* MODR bits - see Table 13-63 in [1], Table 13-23 in [2], Table 13-22 in [3] */

#define	MODR_MODEM_DAT_MASK	bits(15,0)
#define	MODR_MODEM_DAT(x)	bits_val(15,0,x)
#define	get_MODR_MODEM_DAT(x)	bits_get(15,0,x)

#endif /* PXA2X0_AC97_H */
