/*
 * $Id$
 *
 * StrongARM SA-1110 Interrupt Controller Registers
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
 * [1] Intel Corporation, "Intel StrongARM SA-1110 Microprocessor
 *     Developer's Manual", October 2001, Order Number: 278240-004
 *
 */

#ifndef	SA11X0_IC_H
#define	SA11X0_IC_H

#include <common.h>

#if LANGUAGE == C
#include <stdint.h>
#endif

/* Interrupt Controller Registers */

#define	IC_BASE		0x90050000

#if LANGUAGE == C
typedef volatile struct IC_registers {
	uint32_t icip;
	uint32_t icmr;
	uint32_t iclr;
	uint32_t iccr;
	uint32_t icfp;
	uint32_t __reserved[3];
	uint32_t icpr;
} IC_registers;

#ifdef SA11X0_UNMAPPED
#define	IC_pointer	((IC_registers*) IC_BASE)
#endif

#define	ICIP		IC_pointer->icip
#define	ICMR		IC_pointer->icmr
#define	ICLR		IC_pointer->iclr
#define	ICCR		IC_pointer->iccr
#define	ICFP		IC_pointer->icfp
#define	ICPR		IC_pointer->icpr
#endif /* LANGUAGE == C */

#define	ICIP_OFFSET	0x00
#define	ICMR_OFFSET	0x04
#define	ICLR_OFFSET	0x08
#define	ICCR_OFFSET	0x0C
#define	ICFP_OFFSET	0x10
#define	ICPR_OFFSET	0x20

/* ICCR bits */

#define	ICCR_DIM	bit(0)

#endif /* SA11X0_IC_H */
