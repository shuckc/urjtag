/*
 * $Id$
 *
 * Hitachi HD64461 AFE Interface Registers
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

#ifndef HD64461_AFE_H
#define	HD64461_AFE_H

#include <common.h>

#if LANGUAGE == C
#include <stdint.h>
#endif

/* AFE Interface Registers */

#if LANGUAGE == C
typedef volatile struct AFE_registers {
	uint16_t actr;
	uint16_t astr;
	uint16_t arxdr;
	uint16_t atxdr;
} AFE_registers_t;
#endif /* LANGUAGE == C */

#define	ACTR_OFFSET			0x00
#define	ASTR_OFFSET			0x02
#define	ARXDR_OFFSET			0x04
#define	ATXDR_OFFSET			0x06

/* ACTR bits */
#define	ACTR_HC				bit(15)
#define	ACTR_DIV_MASK			bits(14,13)
#define	ACTR_DIV(x)			bits_val(14,13,x)
#define	get_ACTR_DIV(x)			bits_get(14,13,x)
#define	ACTR_RLYCNT			bit(12)
#define	ACTR_CNT2			bit(11)
#define	ACTR_CNT1			bit(10)
#define	ACTR_TSW			bit(9)
#define	ACTR_RSW			bit(8)
#define	ACTR_RDETM			bit(7)
#define	ACTR_TEIE			bit(6)
#define	ACTR_REIE			bit(5)
#define	ACTR_TXIE			bit(4)
#define	ACTR_RXIE			bit(3)
#define	ACTR_BUFD			bit(2)
#define	ACTR_TE				bit(1)
#define	ACTR_RE				bit(0)

/* ASTR bits */
#define	ASTR_TAB			bit(15)
#define	ASTR_RAB			bit(14)
#define	ASTR_TERR			bit(3)
#define	ASTR_RERR			bit(2)
#define	ASTR_TDE			bit(1)
#define	ASTR_RDF			bit(0)

#endif /* HD64461_AFE_H */
