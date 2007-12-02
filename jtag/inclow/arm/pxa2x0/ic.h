/*
 * $Id$
 *
 * XScale PXA26x/PXA255/PXA250/PXA210 Interrupt Control Registers
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

#ifndef	PXA2X0_IC_H
#define	PXA2X0_IC_H

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

/* Interrupt Control Registers */

#define	IC_BASE		0x40D00000

#if LANGUAGE == C
typedef volatile struct IC_registers {
	uint32_t icip;
	uint32_t icmr;
	uint32_t iclr;
	uint32_t icfp;
	uint32_t icpr;
	uint32_t iccr;
} IC_registers_t;

#ifdef PXA2X0_UNMAPPED
#define	IC_pointer	((IC_registers_t*) IC_BASE)
#endif

#define	ICIP		IC_pointer->icip
#define	ICMR		IC_pointer->icmr
#define	ICLR		IC_pointer->iclr
#define	ICFP		IC_pointer->icfp
#define	ICPR		IC_pointer->icpr
#define	ICCR		IC_pointer->iccr
#endif /* LANGUAGE == C */

#define	ICIP_OFFSET	0x00
#define	ICMR_OFFSET	0x04
#define	ICLR_OFFSET	0x08
#define	ICFP_OFFSET	0x0C
#define	ICPR_OFFSET	0x10
#define	ICCR_OFFSET	0x14

/* IRQ bits */

#define	IC_IRQ31	bit(31)
#define	IC_IRQ30	bit(30)
#define	IC_IRQ29	bit(29)
#define	IC_IRQ28	bit(28)
#define	IC_IRQ27	bit(27)
#define	IC_IRQ26	bit(26)
#define	IC_IRQ25	bit(25)
#define	IC_IRQ24	bit(24)
#define	IC_IRQ23	bit(23)
#define	IC_IRQ22	bit(22)
#define	IC_IRQ21	bit(21)
#define	IC_IRQ20	bit(20)
#define	IC_IRQ19	bit(19)
#define	IC_IRQ18	bit(18)
#define	IC_IRQ17	bit(17)
#if !defined(PXA2X0_NOPXA255)
#define	IC_IRQ16	bit(16)
#endif /* PXA255 and above only */
#if !defined(PXA2X0_NOPXA260)
#define	IC_IRQ15	bit(15)
#endif /* PXA260 and above only */
#define	IC_IRQ14	bit(14)
#define	IC_IRQ13	bit(13)
#define	IC_IRQ12	bit(12)
#define	IC_IRQ11	bit(11)
#define	IC_IRQ10	bit(10)
#define	IC_IRQ9		bit(9)
#define	IC_IRQ8		bit(8)
#if !defined(PXA2X0_NOPXA255)
#define	IC_IRQ7		bit(7)
#endif /* PXA255 and above only */

/* symbolic names for IRQs - see Table 4-36 in [1], Table 4-36 in [2], Table 4-36 in [3] */

#define	IC_IRQ_RTC_ALARM	IC_IRQ31
#define	IC_IRQ_RTC_HZ		IC_IRQ30
#define	IC_IRQ_OST3		IC_IRQ29
#define	IC_IRQ_OST2		IC_IRQ28
#define	IC_IRQ_OST1		IC_IRQ27
#define	IC_IRQ_OST0		IC_IRQ26
#define	IC_IRQ_DMA		IC_IRQ25
#define	IC_IRQ_SSP		IC_IRQ24
#define	IC_IRQ_MMC		IC_IRQ23
#define	IC_IRQ_FFUART		IC_IRQ22
#define	IC_IRQ_BTUART		IC_IRQ21
#define	IC_IRQ_STUART		IC_IRQ20
#define	IC_IRQ_ICP		IC_IRQ19
#define	IC_IRQ_I2C		IC_IRQ18
#define	IC_IRQ_LCD		IC_IRQ17
#if !defined(PXA2X0_NOPXA255)
#define	IC_IRQ_NSSP		IC_IRQ16
#endif /* PXA255 and above only */
#if !defined(PXA2X0_NOPXA260)
#define	IC_IRQ_ASSP		IC_IRQ15
#endif /* PXA260 and above only */
#define	IC_IRQ_AC97		IC_IRQ14
#define	IC_IRQ_I2S		IC_IRQ13
#define	IC_IRQ_PMU		IC_IRQ12
#define	IC_IRQ_USB		IC_IRQ11
#define	IC_IRQ_GPIO		IC_IRQ10
#define	IC_IRQ_GPIO1		IC_IRQ9
#define	IC_IRQ_GPIO0		IC_IRQ8
#if !defined(PXA2X0_NOPXA255)
#define	IC_IRQ_HWUART		IC_IRQ7
#endif /* PXA255 and above only */

/* ICCR bits - see Table 4-33 in [1], Table 4-33 in [2], Table 4-32 in [3] */

#define	ICCR_DIM	bit(0)

#endif /* PXA2X0_IC_H */
