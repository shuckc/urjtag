/*
 * $Id$
 *
 * XScale PXA250/PXA210 I2C Registers
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
 *
 */

#ifndef	PXA2X0_I2C_H
#define	PXA2X0_I2C_H

#include <common.h>

#if LANGUAGE == C
#include <stdint.h>
#endif

/* I2C Registers */

#define	I2C_BASE	0x40300000

#if LANGUAGE == C
typedef volatile struct I2C_registers {
	uint32_t __reserved1[1440];
	uint32_t ibmr;
	uint32_t __reserved2;
	uint32_t idbr;
	uint32_t __reserved3;
	uint32_t icr;
	uint32_t __reserved4;
	uint32_t isr;
	uint32_t __reserved5;
	uint32_t isar;
} I2C_registers_t;

#ifdef PXA2X0_UNMAPPED
#define	I2C_pointer	((I2C_registers_t*) I2C_BASE)
#endif

#define	IBMR		I2C_pointer->ibmr
#define	IDBR		I2C_pointer->idbr
#define	ICR		I2C_pointer->icr
#define	ISR		I2C_pointer->isr
#define	ISAR		I2C_pointer->isar
#endif /* LANGUAGE == C */

#define	IBMR_OFFSET	0x1680
#define	IDBR_OFFSET	0x1688
#define	ICR_OFFSET	0x1690
#define	ISR_OFFSET	0x1698
#define	ISAR_OFFSET	0x16A0

/* IBMR bits - see Table 9-9 in [1] */

#define	IBMR_SCLS	bit(1)
#define	IBMR_SDAS	bit(0)

/* IDBR bits - see Table 9-10 in [1] */

#define	IDBR_IDB_MASK	bits(7,0)
#define	IDBR_IDB(x)	bits_val(7,0,x)

/* ICR bits - see Table 9-11 in [1] */

#define	ICR_FM		bit(15)
#define	ICR_UR		bit(14)
#define	ICR_SADIE	bit(13)
#define	ICR_ALDIE	bit(12)
#define	ICR_SSDIE	bit(11)
#define	ICR_BEIE	bit(10)
#define	ICR_IRFIE	bit(9)
#define	ICR_ITEIE	bit(8)
#define	ICR_GCD		bit(7)
#define	ICR_IUE		bit(6)
#define	ICR_SCLE	bit(5)
#define	ICR_MA		bit(4)
#define	ICR_TB		bit(3)
#define	ICR_ACKNAK	bit(2)
#define	ICR_STOP	bit(1)
#define	ICR_START	bit(0)

/* ISR bits - see Table 9-12 in [1] */

#define	ISR_BED		bit(10)
#define	ISR_SAD		bit(9)
#define	ISR_GCAD	bit(8)
#define	ISR_IRF		bit(7)
#define	ISR_ITE		bit(6)
#define	ISR_ALD		bit(5)
#define	ISR_SSD		bit(4)
#define	ISR_IBB		bit(3)
#define	ISR_UB		bit(2)
#define	ISR_ACKNAK	bit(1)
#define	ISR_RWM		bit(0)

/* ISAR bits - see Table 9-13 in [1] */

#define	ISAR_ISA_MASK	bits(6,0)
#define	ISAR_ISA(x)	bits_val(6,0,x)

#endif /* PXA2X0_I2C_H */
