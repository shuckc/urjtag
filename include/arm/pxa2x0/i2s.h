/*
 * $Id$
 *
 * XScale PXA250/PXA210 I2S Registers
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

#ifndef	PXA2X0_I2S_H
#define	PXA2X0_I2S_H

#include <common.h>

#if LANGUAGE == C
#include <stdint.h>
#endif

/* I2S Registers */

#define	I2S_BASE	0x40400000

#if LANGUAGE == C
typedef volatile struct I2S_registers {
	uint32_t sacr0;
	uint32_t sacr1;
	uint32_t __reserved1;
	uint32_t sasr0;
	uint32_t __reserved2;
	uint32_t saimr;
	uint32_t saicr;
	uint32_t __reserved3[17];
	uint32_t sadiv;
	uint32_t __reserved4[7];
	uint32_t sadr;
} I2S_registers;

#ifdef PXA2X0_UNMAPPED
#define	I2S_pointer		((I2S_registers*) I2S_BASE)
#endif

#define	SACR0			I2S_pointer->sacr0
#define	SACR1			I2S_pointer->sacr1
#define	SASR0			I2S_pointer->sasr0
#define	SAIMR			I2S_pointer->saimr
#define	SAICR			I2S_pointer->saicr
#define	SADIV			I2S_pointer->sadiv
#define	SADR			I2S_pointer->sadr
#endif /* LANGUAGE == C */

#define	SACR0_OFFSET		0x00
#define	SACR1_OFFSET		0x04
#define	SASR0_OFFSET		0x0C
#define	SAIMR_OFFSET		0x14
#define	SAICR_OFFSET		0x18
#define	SADIV_OFFSET		0x60
#define	SADR_OFFSET		0x80

/* SACR0 bits - see Table 14-3 in [1] */

#define	SACR0_RFTH_MASK		0xF000
#define	SACR0_RFTH(x)		((x << 12) & SACR0_RFTH_MASK)
#define	SACR0_TFTH_MASK		0x0F00
#define	SACR0_TFTH(x)		((x << 8) & SACR0_TFTH_MASK)
#define	SACR0_STRF		bit(5)
#define	SACR0_EFWR		bit(4)
#define	SACR0_RST		bit(3)
#define	SACR0_BCKD		bit(2)
#define	SACR0_ENB		bit(0)

/* SACR1 bits - see Table 14-6 in [1] */

#define	SACR1_ENLBF		bit(5)
#define	SACR1_DRPL		bit(4)
#define	SACR1_DREC		bit(3)
#define	SACR1_AMSL		bit(0)

/* SASR0 bits - see Table 14-7 in [1] */

#define	SASR0_RFL_MASK		0xF000
#define	SASR0_RFL(x)		((x << 12) & SASR0_RFL_MASK)
#define	SASR0_TFL_MASK		0x0F00
#define	SASR0_TFL(x)		((x << 8) & SASR0_TFL_MASK)
#define	SASR0_ROR		bit(6)
#define	SASR0_TUR		bit(5)
#define	SASR0_RFS		bit(4)
#define	SASR0_TFS		bit(3)
#define	SASR0_BSY		bit(2)
#define	SASR0_RNE		bit(1)
#define	SASR0_TNF		bit(0)

/* SAIMR bits - see Table 14-10 in [1] */

#define	SAIMR_ROR		bit(6)
#define	SAIMR_TUR		bit(5)
#define	SAIMR_RFS		bit(4)
#define	SAIMR_TFS		bit(3)

/* SAICR bits - see Table 14-9 in [1] */

#define	SAICR_ROR		bit(6)
#define	SAICR_TUR		bit(5)

/* SADIV bits - see Table 14-8 in [1] */

#define	SADIV_SADIV_MASK	0x7F
#define	SADIV_SADIV(x)		(x & SADIV_SADIV_MASK)

/* SADR bits - see Table 14-11 in [1] */

#define	SADR_DTH_MASK		0xFFFF0000
#define	SADR_DTH(x)		((x << 16) & SADR_DTH_MASK)
#define	SADR_DTL_MASK		0x0000FFFF
#define	SADR_DTL(x)		(x & SADR_DTL_MASK)

#endif /* PXA2X0_I2S_H */
