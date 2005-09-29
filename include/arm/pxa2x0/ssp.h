/*
 * $Id$
 *
 * XScale PXA26x/PXA255/PXA250/PXA210 SSP/NSSP/ASSP Registers
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

#ifndef	PXA2X0_SSP_H
#define	PXA2X0_SSP_H

#include <openwince.h>

#if LANGUAGE == C
#include <stdint.h>
#endif

#if defined(PXA2X0_NOPXA250) && !defined(PXA2X0_NOPXA255)
#define PXA2X0_NOPXA255
#endif

#if defined(PXA2X0_NOPXA255) && !defined(PXA2X0_NOPXA260)
#define PXA2X0_NOPXA260
#endif

/* SSP Registers */

#define	SSP_BASE	0x41000000
#if !defined(PXA2X0_NOPXA255)
#define	NSSP_BASE	0x41400000
#endif /* PXA255 and above only */
#if !defined(PXA2X0_NOPXA260)
#define	ASSP_BASE	0x41500000
#endif /* PXA260 and above only */

#if LANGUAGE == C
/* see Table 8-7 in [1], Table 8-7 in [2], Table 16-10 in [2], Table 16-11 in [2], Table 8-7 in [3], Table 16-10 in [3] */
typedef volatile struct SSP_registers {
	uint32_t sscr0;
	uint32_t sscr1;
	uint32_t sssr;
#if defined(PXA2X0_NOPXA255)
	uint32_t __reserved;
#else /* PXA255 and above only */
	uint32_t xssitr;		/* only for NSSP/ASSP */
#endif /* PXA255 and above only */
	uint32_t ssdr;
#if !defined(PXA2X0_NOPXA255)
	uint32_t __reserved[5];
	uint32_t xssto;			/* only for NSSP/ASSP */
	uint32_t xsspsp;		/* only for NSSP/ASSP */
#endif /* PXA255 and above only */
} SSP_registers_t;

#ifdef PXA2X0_UNMAPPED
#define	SSP_pointer	((SSP_registers_t*) SSP_BASE)
#if !defined(PXA2X0_NOPXA255)
#define	NSSP_pointer	((SSP_registers_t*) NSSP_BASE)
#endif /* PXA255 and above only */
#if !defined(PXA2X0_NOPXA260)
#define	ASSP_pointer	((SSP_registers_t*) ASSP_BASE)
#endif /* PXA260 and above only */
#endif

#define	SSCR0		SSP_pointer->sscr0
#define	SSCR1		SSP_pointer->sscr1
#define	SSSR		SSP_pointer->sssr
#define	SSDR		SSP_pointer->ssdr

#if !defined(PXA2X0_NOPXA255)
#define	NSSCR0		NSSP_pointer->sscr0
#define	NSSCR1		NSSP_pointer->sscr1
#define	NSSSR		NSSP_pointer->sssr
#define	NSSITR		NSSP_pointer->xssitr
#define	NSSDR		NSSP_pointer->ssdr
#define	NSSPTO		NSSP_pointer->xsspto
#define	NSSPSP		NSSP_pointer->xsspsp
#endif /* PXA255 and above only */

#if !defined(PXA2X0_NOPXA260)
#define	ASSCR0		ASSP_pointer->sscr0
#define	ASSCR1		ASSP_pointer->sscr1
#define	ASSSR		ASSP_pointer->sssr
#define	ASSITR		ASSP_pointer->xssitr
#define	ASSDR		ASSP_pointer->ssdr
#define	ASSPTO		ASSP_pointer->xsspto
#define	ASSPSP		ASSP_pointer->xsspsp
#endif /* PXA260 and above only */

#if !defined(PXA2X0_NOPXA255)
/* common for NSSP/ASSP */

#define	XSSCR0		XSSP_pointer->sscr0
#define	XSSCR1		XSSP_pointer->sscr1
#define	XSSSR		XSSP_pointer->sssr
#define	XSSITR		XSSP_pointer->xssitr
#define	XSSDR		XSSP_pointer->ssdr
#define	XSSPTO		XSSP_pointer->xsspto
#define	XSSPSP		XSSP_pointer->xsspsp
#endif /* PXA255 and above only */
#endif /* LANGUAGE == C */

#define	SSCR0_OFFSET	0x00
#define	SSCR1_OFFSET	0x04
#define	SSSR_OFFSET	0x08
#define	SSDR_OFFSET	0x10

#if !defined(PXA2X0_NOPXA255)
#define	NSSCR0_OFFSET	0x00
#define	NSSCR1_OFFSET	0x04
#define	NSSSR_OFFSET	0x08
#define	NSSITR_OFFSET	0x0C
#define	NSSDR_OFFSET	0x10
#define	NSSTO_OFFSET	0x28
#define	NSSPSP_OFFSET	0x2C
#endif /* PXA255 and above only */

#if !defined(PXA2X0_NOPXA260)
#define	ASSCR0_OFFSET	0x00
#define	ASSCR1_OFFSET	0x04
#define	ASSSR_OFFSET	0x08
#define	ASSITR_OFFSET	0x0C
#define	ASSDR_OFFSET	0x10
#define	ASSTO_OFFSET	0x28
#define	ASSPSP_OFFSET	0x2C
#endif /* PXA260 and above only */

#if !defined(PXA2X0_NOPXA255)
/* common for NSSP/ASSP */

#define	XSSCR0_OFFSET	0x00
#define	XSSCR1_OFFSET	0x04
#define	XSSSR_OFFSET	0x08
#define	XSSITR_OFFSET	0x0C
#define	XSSDR_OFFSET	0x10
#define	XSSTO_OFFSET	0x28
#define	XSSPSP_OFFSET	0x2C
#endif /* PXA255 and above only */

/* SSCR0 bits - see Table 8-2 in [1], Table 8-2 in [2], Table 8-2 in [3] */

#define	SSCR0_SCR_MASK		bits(15,8)
#define	SSCR0_SCR(x)		bits_val(15,8,x)
#define	get_SSCR0_SCR(x)	bits_get(15,8,x)
#define	SSCR0_SSE		bit(7)
#define	SSCR0_ECS		bit(6)
#define	SSCR0_FRF_MASK		bits(5,4)
#define	SSCR0_FRF(x)		bits_val(5,4,x)
#define	get_SSCR0_FRF(x)	bits_get(5,4,x)
#define	SSCR0_DSS_MASK		bits(3,0)
#define	SSCR0_DSS(x)		bits_val(3,0,x)
#define	get_SSCR0_DSS(x)	bits_get(3,0,x)

/* SSCR1 bits - see Table 8-3 in [1], Table 8-3 in [2], Table 8-3 in [3] */

#define	SSCR1_RFT_MASK		bits(13,10)
#define	SSCR1_RFT(x)		bits_val(13,10,x)
#define	get_SSCR1_RFT(x)	bits_get(13,10,x)
#define	SSCR1_TFT_MASK		bits(9,6)
#define	SSCR1_TFT(x)		bits_val(9,6,x)
#define	get_SSCR1_TFT(x)	bits_get(9,6,x)
#define	SSCR1_MWDS		bit(5)
#define	SSCR1_SPH		bit(4)
#define	SSCR1_SPO		bit(3)
#define	SSCR1_LBM		bit(2)
#define	SSCR1_TIE		bit(1)
#define	SSCR1_RIE		bit(0)

/* SSSR bits - see Table 8-6 in [1], Table 8-6 in [2], Table 8-6 in [3] */

#define	SSSR_RFL_MASK		bits(15,12)
#define	SSSR_RFL(x)		bits_val(15,12,x)
#define	get_SSSR_RFL(x)		bits_get(15,12,x)
#define	SSSR_TFL_MASK		bits(11,8)
#define	SSSR_TFL(x)		bits_val(11,8,x)
#define	get_SSSR_TFL(x)		bits_get(11,8,x)
#define	SSSR_ROR		bit(7)
#define	SSSR_RFS		bit(6)
#define	SSSR_TFS		bit(5)
#define	SSSR_BSY		bit(4)
#define	SSSR_RNE		bit(3)
#define	SSSR_TNF		bit(2)

#if !defined(PXA2X0_NOPXA255)
/* NSSCR0 bits - see Table 16-3 in [2], Table 16-3 in [3] */

#define	NSSCR0_EDSS		bit(20)
#define	NSSCR0_SCR_MASK		bits(19,8)
#define	NSSCR0_SCR(x)		bits_val(19,8,x)
#define	get_NSSCR0_SCR(x)	bits_get(19,8,x)
#define	NSSCR0_SSE		bit(7)
#define	NSSCR0_FRF_MASK		bits(5,4)
#define	NSSCR0_FRF(x)		bits_val(5,4,x)
#define	get_NSSCR0_FRF(x)	bits_get(5,4,x)
#define	NSSCR0_DSS_MASK		bits(3,0)
#define	NSSCR0_DSS(x)		bits_val(3,0,x)
#define	get_NSSCR0_DSS(x)	bits_get(3,0,x)
#endif /* PXA255 and above only */

#if !defined(PXA2X0_NOPXA260)
/* ASSCR0 bits - see Table 16-3 in [2] */

#define	ASSCR0_EDSS		bit(20)
#define	ASSCR0_SCR_MASK		bits(19,8)
#define	ASSCR0_SCR(x)		bits_val(19,8,x)
#define	get_ASSCR0_SCR(x)	bits_get(19,8,x)
#define	ASSCR0_SSE		bit(7)
#define	ASSCR0_FRF_MASK		bits(5,4)
#define	ASSCR0_FRF(x)		bits_val(5,4,x)
#define	get_ASSCR0_FRF(x)	bits_get(5,4,x)
#define	ASSCR0_DSS_MASK		bits(3,0)
#define	ASSCR0_DSS(x)		bits_val(3,0,x)
#define	get_ASSCR0_DSS(x)	bits_get(3,0,x)
#endif /* PXA260 and above only */

#if !defined(PXA2X0_NOPXA255)
/* NSSCR0/ASSCR0 bits - see Table 16-3 in [2], Table 16-3 in [3] */

#define	XSSCR0_EDSS		bit(20)
#define	XSSCR0_SCR_MASK		bits(19,8)
#define	XSSCR0_SCR(x)		bits_val(19,8,x)
#define	get_XSSCR0_SCR(x)	bits_get(19,8,x)
#define	XSSCR0_SSE		bit(7)
#define	XSSCR0_FRF_MASK		bits(5,4)
#define	XSSCR0_FRF(x)		bits_val(5,4,x)
#define	get_XSSCR0_FRF(x)	bits_get(5,4,x)
#define	XSSCR0_DSS_MASK		bits(3,0)
#define	XSSCR0_DSS(x)		bits_val(3,0,x)
#define	get_XSSCR0_DSS(x)	bits_get(3,0,x)

/* NSSCR1 bits - see Table 16-4 in [2], Table 16-4 in [3] */

#define	NSSCR1_TTELP		bit(31)
#define	NSSCR1_TTE		bit(30)
#define	NSSCR1_EBCEI		bit(29)
#define	NSSCR1_SCFR		bit(28)
#define	NSSCR1_SCLKDIR		bit(25)
#define	NSSCR1_SFRMDIR		bit(24)
#define	NSSCR1_RWOT		bit(23)
#define	NSSCR1_TSRE		bit(21)
#define	NSSCR1_RSRE		bit(20)
#define	NSSCR1_TINTE		bit(19)
#define	NSSCR1_STRF		bit(15)
#define	NSSCR1_EFWR		bit(14)
#define	NSSCR1_RFT_MASK		bits(13,10)
#define	NSSCR1_RFT(x)		bits_val(13,10,x)
#define	get_NSSCR1_RFT(x)	bits_get(13,10,x)
#define	NSSCR1_TFT_MASK		bits(9,6)
#define	NSSCR1_TFT(x)		bits_val(9,6,x)
#define	get_NSSCR1_TFT(x)	bits_get(9,6,x)
#define	NSSCR1_MWDS		bit(5)
#define	NSSCR1_SPH		bit(4)
#define	NSSCR1_SPO		bit(3)
#define	NSSCR1_LBM		bit(2)
#define	NSSCR1_TIE		bit(1)
#define	NSSCR1_RIE		bit(0)
#endif /* PXA255 and above only */

#if !defined(PXA2X0_NOPXA260)
/* ASSCR1 bits - see Table 16-4 in [2] */

#define	ASSCR1_TTELP		bit(31)
#define	ASSCR1_TTE		bit(30)
#define	ASSCR1_EBCEI		bit(29)
#define	ASSCR1_SCFR		bit(28)
#define	ASSCR1_SCLKDIR		bit(25)
#define	ASSCR1_SFRMDIR		bit(24)
#define	ASSCR1_RWOT		bit(23)
#define	ASSCR1_TSRE		bit(21)
#define	ASSCR1_RSRE		bit(20)
#define	ASSCR1_TINTE		bit(19)
#define	ASSCR1_STRF		bit(15)
#define	ASSCR1_EFWR		bit(14)
#define	ASSCR1_RFT_MASK		bits(13,10)
#define	ASSCR1_RFT(x)		bits_val(13,10,x)
#define	get_ASSCR1_RFT(x)	bits_get(13,10,x)
#define	ASSCR1_TFT_MASK		bits(9,6)
#define	ASSCR1_TFT(x)		bits_val(9,6,x)
#define	get_ASSCR1_TFT(x)	bits_get(9,6,x)
#define	ASSCR1_MWDS		bit(5)
#define	ASSCR1_SPH		bit(4)
#define	ASSCR1_SPO		bit(3)
#define	ASSCR1_LBM		bit(2)
#define	ASSCR1_TIE		bit(1)
#define	ASSCR1_RIE		bit(0)
#endif /* PXA260 and above only */

#if !defined(PXA2X0_NOPXA255)
/* NSSCR1/ASSCR1 bits - see Table 16-4 in [2], Table 16-4 in [3] */

#define	XSSCR1_TTELP		bit(31)
#define	XSSCR1_TTE		bit(30)
#define	XSSCR1_EBCEI		bit(29)
#define	XSSCR1_SCFR		bit(28)
#define	XSSCR1_SCLKDIR		bit(25)
#define	XSSCR1_SFRMDIR		bit(24)
#define	XSSCR1_RWOT		bit(23)
#define	XSSCR1_TSRE		bit(21)
#define	XSSCR1_RSRE		bit(20)
#define	XSSCR1_TINTE		bit(19)
#define	XSSCR1_STRF		bit(15)
#define	XSSCR1_EFWR		bit(14)
#define	XSSCR1_RFT_MASK		bits(13,10)
#define	XSSCR1_RFT(x)		bits_val(13,10,x)
#define	get_XSSCR1_RFT(x)	bits_get(13,10,x)
#define	XSSCR1_TFT_MASK		bits(9,6)
#define	XSSCR1_TFT(x)		bits_val(9,6,x)
#define	get_XSSCR1_TFT(x)	bits_get(9,6,x)
#define	XSSCR1_MWDS		bit(5)
#define	XSSCR1_SPH		bit(4)
#define	XSSCR1_SPO		bit(3)
#define	XSSCR1_LBM		bit(2)
#define	XSSCR1_TIE		bit(1)
#define	XSSCR1_RIE		bit(0)

/* NSSITR bits - see Table 16-7 in [2], Table 16-7 in [3] */

#define	NSSITR_TROR		bit(7)
#define	NSSITR_TRFS		bit(6)
#define	NSSITR_TTFS		bit(5)
#endif /* PXA255 and above only */

#if !defined(PXA2X0_NOPXA260)
/* ASSITR bits - see Table 16-7 in [2] */

#define	ASSITR_TROR		bit(7)
#define	ASSITR_TRFS		bit(6)
#define	ASSITR_TTFS		bit(5)
#endif /* PXA260 and above only */

#if !defined(PXA2X0_NOPXA255)
/* NSSITR/ASSITR bits - see Table 16-7 in [2], Table 16-7 in [3] */

#define	XSSITR_TROR		bit(7)
#define	XSSITR_TRFS		bit(6)
#define	XSSITR_TTFS		bit(5)

/* NSSSR bits - see Table 16-8 in [2], Table 16-8 in [3] */

#define	NSSSR_BCE		bit(23)
#define	NSSSR_CSS		bit(22)
#define	NSSSR_TUR		bit(21)
#define	NSSSR_TINT		bit(19)
#define	NSSSR_RFL_MASK		bits(15,12)
#define	NSSSR_RFL(x)		bits_val(15,12,x)
#define	get_NSSSR_RFL(x)	bits_get(15,12,x)
#define	NSSSR_TFL_MASK		bits(11,8)
#define	NSSSR_TFL(x)		bits_val(11,8,x)
#define	get_NSSSR_TFL(x)	bits_get(11,8,x)
#define	NSSSR_ROR		bit(7)
#define	NSSSR_RFS		bit(6)
#define	NSSSR_TFS		bit(5)
#define	NSSSR_BSY		bit(4)
#define	NSSSR_RNE		bit(3)
#define	NSSSR_TNF		bit(2)
#endif /* PXA255 and above only */

#if !defined(PXA2X0_NOPXA260)
/* ASSSR bits - see Table 16-8 in [2] */

#define	ASSSR_BCE		bit(23)
#define	ASSSR_CSS		bit(22)
#define	ASSSR_TUR		bit(21)
#define	ASSSR_TINT		bit(19)
#define	ASSSR_RFL_MASK		bits(15,12)
#define	ASSSR_RFL(x)		bits_val(15,12,x)
#define	get_ASSSR_RFL(x)	bits_get(15,12,x)
#define	ASSSR_TFL_MASK		bits(11,8)
#define	ASSSR_TFL(x)		bits_val(11,8,x)
#define	get_ASSSR_TFL(x)	bits_get(11,8,x)
#define	ASSSR_ROR		bit(7)
#define	ASSSR_RFS		bit(6)
#define	ASSSR_TFS		bit(5)
#define	ASSSR_BSY		bit(4)
#define	ASSSR_RNE		bit(3)
#define	ASSSR_TNF		bit(2)
#endif /* PXA260 and above only */

#if !defined(PXA2X0_NOPXA255)
/* NSSSR/ASSSR bits - see Table 16-8 in [2], Table 16-8 in [3] */

#define	XSSSR_BCE		bit(23)
#define	XSSSR_CSS		bit(22)
#define	XSSSR_TUR		bit(21)
#define	XSSSR_TINT		bit(19)
#define	XSSSR_RFL_MASK		bits(15,12)
#define	XSSSR_RFL(x)		bits_val(15,12,x)
#define	get_XSSSR_RFL(x)	bits_get(15,12,x)
#define	XSSSR_TFL_MASK		bits(11,8)
#define	XSSSR_TFL(x)		bits_val(11,8,x)
#define	get_XSSSR_TFL(x)	bits_get(11,8,x)
#define	XSSSR_ROR		bit(7)
#define	XSSSR_RFS		bit(6)
#define	XSSSR_TFS		bit(5)
#define	XSSSR_BSY		bit(4)
#define	XSSSR_RNE		bit(3)
#define	XSSSR_TNF		bit(2)

/* NSSTO bits - see Table 16-6 in [2], Table 16-6 in [3] */

#define	NSSTO_TIMEOUT_MASK	bits(23,0)
#define	NSSTO_TIMEOUT(x)	bits_val(23,0,x)
#define	get_NSSTO_TIMEOUT(x)	bits_get(23,0,x)
#endif /* PXA255 and above only */

#if !defined(PXA2X0_NOPXA260)
/* ASSTO bits - see Table 16-6 in [2] */

#define	ASSTO_TIMEOUT_MASK	bits(23,0)
#define	ASSTO_TIMEOUT(x)	bits_val(23,0,x)
#define	get_ASSTO_TIMEOUT(x)	bits_get(23,0,x)
#endif /* PXA260 and above only */

#if !defined(PXA2X0_NOPXA255)
/* NSSTO/ASSTO bits - see Table 16-6 in [2], Table 16-6 in [3] */

#define	XSSTO_TIMEOUT_MASK	bits(23,0)
#define	XSSTO_TIMEOUT(x)	bits_val(23,0,x)
#define	get_XSSTO_TIMEOUT(x)	bits_get(23,0,x)

/* NSSPSP bits - see Table 16-5 in [2], Table 16-5 in [3] */

#define	NSSPSP_DMYSTOP_MASK	bits(24,23)
#define	NSSPSP_DMYSTOP(x)	bits_val(24,23,x)
#define	get_NSSPSP_DMYSTOP(x)	bits_get(24,23,x)
#define	NSSPSP_SFRMWDTH_MASK	bits(21,16)
#define	NSSPSP_SFRMWDTH(x)	bits_val(21,16,x)
#define	get_NSSPSP_SFRMWDTH(x)	bits_get(21,16,x)
#define	NSSPSP_SFRMDLY_MASK	bits(15,9)
#define	NSSPSP_SFRMDLY(x)	bits_val(15,9,x)
#define	get_NSSPSP_SFRMDLY(x)	bits_get(15,9,x)
#define	NSSPSP_DMYSTRT_MASK	bits(8,7)
#define	NSSPSP_DMYSTRT(x)	bits_val(8,7,x)
#define	get_NSSPSP_DMYSTRT(x)	bits_get(8,7,x)
#define	NSSPSP_STRTDLY_MASK	bits(6,4)
#define	NSSPSP_STRTDLY(x)	bits_val(6,4,x)
#define	get_NSSPSP_STRTDLY(x)	bits_get(6,4,x)
#define	NSSPSP_ETDS		bit(3)
#define	NSSPSP_SFRMP		bit(2)
#define	NSSPSP_SCMODE_MASK	bits(1,0)
#define	NSSPSP_SCMODE(x)	bits_val(1,0,x)
#define	get_NSSPSP_SCMODE(x)	bits_get(1,0,x)
#endif /* PXA255 and above only */

#if !defined(PXA2X0_NOPXA260)
/* ASSPSP bits - see Table 16-5 in [2] */

#define	ASSPSP_DMYSTOP_MASK	bits(24,23)
#define	ASSPSP_DMYSTOP(x)	bits_val(24,23,x)
#define	get_ASSPSP_DMYSTOP(x)	bits_get(24,23,x)
#define	ASSPSP_SFRMWDTH_MASK	bits(21,16)
#define	ASSPSP_SFRMWDTH(x)	bits_val(21,16,x)
#define	get_ASSPSP_SFRMWDTH(x)	bits_get(21,16,x)
#define	ASSPSP_SFRMDLY_MASK	bits(15,9)
#define	ASSPSP_SFRMDLY(x)	bits_val(15,9,x)
#define	get_ASSPSP_SFRMDLY(x)	bits_get(15,9,x)
#define	ASSPSP_DMYSTRT_MASK	bits(8,7)
#define	ASSPSP_DMYSTRT(x)	bits_val(8,7,x)
#define	get_ASSPSP_DMYSTRT(x)	bits_get(8,7,x)
#define	ASSPSP_STRTDLY_MASK	bits(6,4)
#define	ASSPSP_STRTDLY(x)	bits_val(6,4,x)
#define	get_ASSPSP_STRTDLY(x)	bits_get(6,4,x)
#define	ASSPSP_ETDS		bit(3)
#define	ASSPSP_SFRMP		bit(2)
#define	ASSPSP_SCMODE_MASK	bits(1,0)
#define	ASSPSP_SCMODE(x)	bits_val(1,0,x)
#define	get_ASSPSP_SCMODE(x)	bits_get(1,0,x)
#endif /* PXA260 and above only */

#if !defined(PXA2X0_NOPXA255)
/* NSSPSP/ASSPSP bits - see Table 16-5 in [2], Table 16-5 in [3] */

#define	XSSPSP_DMYSTOP_MASK	bits(24,23)
#define	XSSPSP_DMYSTOP(x)	bits_val(24,23,x)
#define	get_XSSPSP_DMYSTOP(x)	bits_get(24,23,x)
#define	XSSPSP_SFRMWDTH_MASK	bits(21,16)
#define	XSSPSP_SFRMWDTH(x)	bits_val(21,16,x)
#define	get_XSSPSP_SFRMWDTH(x)	bits_get(21,16,x)
#define	XSSPSP_SFRMDLY_MASK	bits(15,9)
#define	XSSPSP_SFRMDLY(x)	bits_val(15,9,x)
#define	get_XSSPSP_SFRMDLY(x)	bits_get(15,9,x)
#define	XSSPSP_DMYSTRT_MASK	bits(8,7)
#define	XSSPSP_DMYSTRT(x)	bits_val(8,7,x)
#define	get_XSSPSP_DMYSTRT(x)	bits_get(8,7,x)
#define	XSSPSP_STRTDLY_MASK	bits(6,4)
#define	XSSPSP_STRTDLY(x)	bits_val(6,4,x)
#define	get_XSSPSP_STRTDLY(x)	bits_get(6,4,x)
#define	XSSPSP_ETDS		bit(3)
#define	XSSPSP_SFRMP		bit(2)
#define	XSSPSP_SCMODE_MASK	bits(1,0)
#define	XSSPSP_SCMODE(x)	bits_val(1,0,x)
#define	get_XSSPSP_SCMODE(x)	bits_get(1,0,x)
#endif /* PXA255 and above only */

#endif /* PXA2X0_SSP_H */
