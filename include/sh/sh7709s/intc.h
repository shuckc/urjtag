/*
 * $Id$
 *
 * Renesas SH7709S Interrupt Controller Registers
 * Copyright (C) 2005 Marcel Telka
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
 * Written by Marcel Telka <marcel@telka.sk>, 2005.
 *
 * Documentation:
 * [1] Renesas Technology, "SH7709S Group Hardware Manual",
 *     Rev.5.00, 2003.9.18, REJ09B0081-0500O
 *
 */

#ifndef SH7709S_INTC_H
#define	SH7709S_INTC_H

#include <openwince.h>

#if LANGUAGE == C
#include <stdint.h>
#endif

/* Interrupt Controller Registers */

#if LANGUAGE == C
typedef volatile struct INTC_registers {
	uint16_t nirr;
	uint16_t nimr;
} INTC_registers_t;
#endif /* LANGUAGE == C */

#define	NIRR_OFFSET			0x00
#define	NIMR_OFFSET			0x02

/* NIRR bits */
#define	NIRR_PCC0R			bit(14)
#define	NIRR_PCC1R			bit(13)
#define	NIRR_AFER			bit(12)
#define	NIRR_GPIOR			bit(11)
#define	NIRR_TMU0R			bit(10)
#define	NIRR_TMU1R			bit(9)
#define	NIRR_IRDAR			bit(6)
#define	NIRR_UARTR			bit(5)

/* NIMR bits */
#define	NIMR_PCC0M			bit(14)
#define	NIMR_PCC1M			bit(13)
#define	NIMR_AFEM			bit(12)
#define	NIMR_GPIOM			bit(11)
#define	NIMR_TMU0M			bit(10)
#define	NIMR_TMU1M			bit(9)
#define	NIMR_IRDAM			bit(6)
#define	NIMR_UARTM			bit(5)

#endif /* SH7709S_INTC_H */
