/*
 * $Id$
 *
 * Hitachi HD64461 Timer Registers
 * Copyright (C) 2004 Marcel Telka
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *     * Neither the name of the copyright holders nor the names of their
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Written by Marcel Telka <marcel@telka.sk>, 2004.
 *
 * Documentation:
 * [1] Hitachi, Ltd., "HD64461 Windows(R) CE Intelligent Peripheral Controller",
 *     1st Edition, July 1998, Order Number: ADE-602-076
 *
 */

#ifndef HD64461_TIMER_H
#define	HD64461_TIMER_H

#include <common.h>

#if LANGUAGE == C
#include <stdint.h>
#endif

/* Timer Registers */

#if LANGUAGE == C
typedef volatile struct TIMER_registers {
	uint16_t tcvr1;
	uint16_t tcvr0;
	uint16_t trvr1;
	uint16_t trvr0;
	uint16_t tcr1;
	uint16_t tcr0;
	uint16_t tirr;
	uint16_t ter;
} TIMER_registers_t;
#endif /* LANGUAGE == C */

#define	TCVR1_OFFSET			0x00
#define	TCVR0_OFFSET			0x02
#define	TRVR1_OFFSET			0x04
#define	TRVR0_OFFSET			0x06
#define	TCR1_OFFSET			0x08
#define	TCR0_OFFSET			0x0A
#define	TIRR_OFFSET			0x0C
#define	TER_OFFSET			0x0E

/* TCR1 bits */
#define	TCR1_ETMO1			bit(3)
#define	TCR1_PST1_MASK			bits(2,1)
#define	TCR1_PST1(x)			bits_val(2,1,x)
#define	get_TCR1_PST1(x)		bits_get(2,1,x)
#define	TCR1_T1STP			bit(0)

/* TCR0 bits */
#define	TCR0_ETMO0			bit(3)
#define	TCR0_PST0_MASK			bits(2,1)
#define	TCR0_PST0(x)			bits_val(2,1,x)
#define	get_TCR0_PST0(x)		bits_get(2,1,x)
#define	TCR0_T0STP			bit(0)

/* TIRR bits */
#define	TIRR_TMU1R			bit(1)
#define	TIRR_TMU0R			bit(0)

/* TER bits */
#define	TER_TMU1E			bit(1)
#define	TER_TMU0E			bit(0)

#endif /* HD64461_TIMER_H */
