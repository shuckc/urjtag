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

#if defined(PXA2X0_NOPXA250) && !defined(PXA2X0_NOPXA26X)
#define PXA2X0_NOPXA26X
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

/* DRCMR symbolic names - see Table 5-13 in [1], Table 5-13 in [2] */

#define	DRCMR_DREQ0		DRCMR(0)
#define	DRCMR_DREQ1		DRCMR(1)
#define	DRCMR_I2S_RX		DRCMR(2)
#define	DRCMR_I2S_TX		DRCMR(3)
#define	DRCMR_BTUART_RX		DRCMR(4)
#define	DRCMR_BTUART_TX		DRCMR(5)
#define	DRCMR_FFUART_RX		DRCMR(6)
#define	DRCMR_FFUART_TX		DRCMR(7)
#define	DRCMR_AC97_MIC_RX	DRCMR(8)
#define	DRCMR_AC97_MODEM_RX	DRCMR(9)
#define	DRCMR_AC97_MODEM_TX	DRCMR(10)
#define	DRCMR_AC97_AUDIO_RX	DRCMR(11)
#define	DRCMR_AC97_AUDIO_TX	DRCMR(12)
#define	DRCMR_SSP_RX		DRCMR(13)
#define	DRCMR_SSP_TX		DRCMR(14)
#if !defined(PXA2X0_NOPXA26X)
#define	DRCMR_NSSP_RX		DRCMR(15)
#define	DRCMR_NSSP_TX		DRCMR(16)
#endif /* PXA26x only */
#define	DRCMR_FICP_RX		DRCMR(17)
#define	DRCMR_FICP_TX		DRCMR(18)
#define	DRCMR_STUART_RX		DRCMR(19)
#define	DRCMR_STUART_TX		DRCMR(20)
#define	DRCMR_MMC_RX		DRCMR(21)
#define	DRCMR_MMC_TX		DRCMR(22)
#if !defined(PXA2X0_NOPXA26X)
#define	DRCMR_ASSP_RX		DRCMR(23)
#define	DRCMR_ASSP_TX		DRCMR(24)
#endif /* PXA26x only */
#define	DRCMR_USB_EP1		DRCMR(25)
#define	DRCMR_USB_EP2		DRCMR(26)
#define	DRCMR_USB_EP3		DRCMR(27)
#define	DRCMR_USB_EP4		DRCMR(28)
#if !defined(PXA2X0_NOPXA26X)
#define	DRCMR_HWUART_RX		DRCMR(29)
#endif /* PXA26x only */
#define	DRCMR_USB_EP6		DRCMR(30)
#define	DRCMR_USB_EP7		DRCMR(31)
#define	DRCMR_USB_EP8		DRCMR(32)
#define	DRCMR_USB_EP9		DRCMR(33)
#if !defined(PXA2X0_NOPXA26X)
#define	DRCMR_HWUART_TX		DRCMR(34)
#endif /* PXA26x only */
#define	DRCMR_USB_EP11		DRCMR(35)
#define	DRCMR_USB_EP12		DRCMR(36)
#define	DRCMR_USB_EP13		DRCMR(37)
#define	DRCMR_USB_EP14		DRCMR(38)
#endif /* LANGUAGE == C */

#define	DCSR_OFFSET(i)		((i) << 2)
#define	DINT_OFFSET		0xF0
#define	DRCMR_OFFSET(i)		(0x100 + ((i) << 2))
#define	DDADR_OFFSET(i)		(0x200 + ((i) << 4))
#define	DSADR_OFFSET(i)		(0x204 + ((i) << 4))
#define	DTADR_OFFSET(i)		(0x208 + ((i) << 4))
#define	DCMD_OFFSET(i)		(0x20C + ((i) << 4))

/* DRCMR symbolic names offsets - see Table 5-13 in [1], Table 5-13 in [2] */

#define	DRCMR_DREQ0_OFFSET		DRCMR_OFFSET(0)
#define	DRCMR_DREQ1_OFFSET		DRCMR_OFFSET(1)
#define	DRCMR_I2S_RX_OFFSET		DRCMR_OFFSET(2)
#define	DRCMR_I2S_TX_OFFSET		DRCMR_OFFSET(3)
#define	DRCMR_BTUART_RX_OFFSET		DRCMR_OFFSET(4)
#define	DRCMR_BTUART_TX_OFFSET		DRCMR_OFFSET(5)
#define	DRCMR_FFUART_RX_OFFSET		DRCMR_OFFSET(6)
#define	DRCMR_FFUART_TX_OFFSET		DRCMR_OFFSET(7)
#define	DRCMR_AC97_MIC_RX_OFFSET	DRCMR_OFFSET(8)
#define	DRCMR_AC97_MODEM_RX_OFFSET	DRCMR_OFFSET(9)
#define	DRCMR_AC97_MODEM_TX_OFFSET	DRCMR_OFFSET(10)
#define	DRCMR_AC97_AUDIO_RX_OFFSET	DRCMR_OFFSET(11)
#define	DRCMR_AC97_AUDIO_TX_OFFSET	DRCMR_OFFSET(12)
#define	DRCMR_SSP_RX_OFFSET		DRCMR_OFFSET(13)
#define	DRCMR_SSP_TX_OFFSET		DRCMR_OFFSET(14)
#if !defined(PXA2X0_NOPXA26X)
#define	DRCMR_NSSP_RX_OFFSET		DRCMR_OFFSET(15)
#define	DRCMR_NSSP_TX_OFFSET		DRCMR_OFFSET(16)
#endif /* PXA26x only */
#define	DRCMR_FICP_RX_OFFSET		DRCMR_OFFSET(17)
#define	DRCMR_FICP_TX_OFFSET		DRCMR_OFFSET(18)
#define	DRCMR_STUART_RX_OFFSET		DRCMR_OFFSET(19)
#define	DRCMR_STUART_TX_OFFSET		DRCMR_OFFSET(20)
#define	DRCMR_MMC_RX_OFFSET		DRCMR_OFFSET(21)
#define	DRCMR_MMC_TX_OFFSET		DRCMR_OFFSET(22)
#if !defined(PXA2X0_NOPXA26X)
#define	DRCMR_ASSP_RX_OFFSET		DRCMR_OFFSET(23)
#define	DRCMR_ASSP_TX_OFFSET		DRCMR_OFFSET(24)
#endif /* PXA26x only */
#define	DRCMR_USB_EP1_OFFSET		DRCMR_OFFSET(25)
#define	DRCMR_USB_EP2_OFFSET		DRCMR_OFFSET(26)
#define	DRCMR_USB_EP3_OFFSET		DRCMR_OFFSET(27)
#define	DRCMR_USB_EP4_OFFSET		DRCMR_OFFSET(28)
#if !defined(PXA2X0_NOPXA26X)
#define	DRCMR_HWUART_RX_OFFSET		DRCMR_OFFSET(29)
#endif /* PXA26x only */
#define	DRCMR_USB_EP6_OFFSET		DRCMR_OFFSET(30)
#define	DRCMR_USB_EP7_OFFSET		DRCMR_OFFSET(31)
#define	DRCMR_USB_EP8_OFFSET		DRCMR_OFFSET(32)
#define	DRCMR_USB_EP9_OFFSET		DRCMR_OFFSET(33)
#if !defined(PXA2X0_NOPXA26X)
#define	DRCMR_HWUART_TX_OFFSET		DRCMR_OFFSET(34)
#endif /* PXA26x only */
#define	DRCMR_USB_EP11_OFFSET		DRCMR_OFFSET(35)
#define	DRCMR_USB_EP12_OFFSET		DRCMR_OFFSET(36)
#define	DRCMR_USB_EP13_OFFSET		DRCMR_OFFSET(37)
#define	DRCMR_USB_EP14_OFFSET		DRCMR_OFFSET(38)

/* DCSRx bits - see Table 5-7 in [1], Table 5-7 in [2] */

#define	DCSR_RUN		bit(31)
#define	DCSR_NODESCFETCH	bit(30)
#define	DCSR_STOPIRQEN		bit(29)
#define	DCSR_REQPEND		bit(8)
#define	DCSR_STOPSTATE		bit(3)
#define	DCSR_ENDINTR		bit(2)
#define	DCSR_STARTINTR		bit(1)
#define	DSCR_BUSERRINTR		bit(0)

/* DINT bits - see Table 5-6 in [1], Table 5-6 in [2] */

#define	DINT_CHLINTR(x)		bit(x)
#define	DINT_CHLINTR0		DINT_CHLINTR(0)
#define	DINT_CHLINTR1		DINT_CHLINTR(1)
#define	DINT_CHLINTR2		DINT_CHLINTR(2)
#define	DINT_CHLINTR3		DINT_CHLINTR(3)
#define	DINT_CHLINTR4		DINT_CHLINTR(4)
#define	DINT_CHLINTR5		DINT_CHLINTR(5)
#define	DINT_CHLINTR6		DINT_CHLINTR(6)
#define	DINT_CHLINTR7		DINT_CHLINTR(7)
#define	DINT_CHLINTR8		DINT_CHLINTR(8)
#define	DINT_CHLINTR9		DINT_CHLINTR(9)
#define	DINT_CHLINTR10		DINT_CHLINTR(10)
#define	DINT_CHLINTR11		DINT_CHLINTR(11)
#define	DINT_CHLINTR12		DINT_CHLINTR(12)
#define	DINT_CHLINTR13		DINT_CHLINTR(13)
#define	DINT_CHLINTR14		DINT_CHLINTR(14)
#define	DINT_CHLINTR15		DINT_CHLINTR(15)

/* DRCMRx bits - see Table 5-8 in [1], Table 5-8 in [2] */

#define	DRCMR_MAPVLD		bit(7)
#define	DRCMR_CHLNUM_MASK	bits(3,0)
#define	DRCMR_CHLNUM(x)		bits_val(3,0,x)
#define	get_DCMR_CHLNUM(x)	bits_get(3,0,x)

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
#define	get_DCMD_SIZE(x)	bits_get(17,16,x)
#define	DCMD_WIDTH_MASK		bits(15,14)
#define	DCMD_WIDTH(x)		bits_val(15,14,x)
#define	get_DCMD_WIDTH(x)	bits_get(15,14,x)
#define	DCMD_LENGTH_MASK	bits(12,0)
#define	DCMD_LENGTH(x)		bits_val(12,0,x)
#define	get_DCMD_LENGTH(x)	bits_get(12,0,x)

#endif /* PXA2X0_DMA_H */
