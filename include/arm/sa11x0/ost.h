/*
 * $Id$
 *
 * StrongARM SA-1110 OS Timer Registers
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

#ifndef	SA11X0_OST_H
#define	SA11X0_OST_H

#include <common.h>

#if LANGUAGE == C
#include <stdint.h>
#endif

/* OS Timer Registers */

#define	OST_BASE	0x90000000

#if LANGUAGE == C
typedef volatile struct OST_registers {
	uint32_t osmr[4];
	uint32_t oscr;
	uint32_t ossr;
	uint32_t ower;
	uint32_t oier;
} OST_registers;

#ifdef SA11X0_UNMAPPED
#define	OST_pointer	((OST_registers*) OST_BASE)
#endif

#define	OSMR(i)		OST_pointer->osmr[i]
#define	OSCR		OST_pointer->oscr
#define	OSSR		OST_pointer->ossr
#define	OWER		OST_pointer->ower
#define	OIER		OST_pointer->oier
#endif /* LANGUAGE == C */

#define	OSMR0_OFFSET	0x00
#define	OSMR1_OFFSET	0x04
#define	OSMR2_OFFSET	0x08
#define	OSMR3_OFFSET	0x0C
#define	OSCR_OFFSET	0x10
#define	OSSR_OFFSET	0x14
#define	OWER_OFFSET	0x18
#define	OIER_OFFSET	0x1C

#endif /* SA11X0_OST_H */
