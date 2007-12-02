/*
 * $Id$
 *
 * StrongARM SA-1110 UDC Registers
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

#ifndef	SA11X0_UDC_H
#define	SA11X0_UDC_H

#include <openwince.h>

#if LANGUAGE == C
#include <stdint.h>
#endif

/* UDC Registers (Serial Port 0) */

#define	UDC_BASE	0x80000000

#if LANGUAGE == C
typedef volatile struct UDC_registers {
	uint32_t udccr;
	uint32_t udcar;
	uint32_t udcomp;
	uint32_t udcimp;
	uint32_t udccs0;
	uint32_t udccs1;
	uint32_t udccs2;
	uint32_t udcd0;
	uint32_t udcwc;
	uint32_t __reserved1;
	uint32_t udcdr;
	uint32_t __reserved2;
	uint32_t udcsr;
} UDC_registers_t;

#ifdef SA11X0_UNMAPPED
#define	UDC_pointer	((UDC_registers_t*) UDC_BASE)
#endif

#define	UDCCR		UDC_pointer->udccr
#define	UDCAR		UDC_pointer->udcar
#define	UDCOMP		UDC_pointer->udcomp
#define	UDCIMP		UDC_pointer->udcimp
#define	UDCCS0		UDC_pointer->udccs0
#define	UDCCS1		UDC_pointer->udccs1
#define	UDCCS2		UDC_pointer->udccs2
#define	UDCD0		UDC_pointer->udcd0
#define	UDCWC		UDC_pointer->udcwc
#define	UDCDR		UDC_pointer->udcdr
#define	UDCSR		UDC_pointer->udcsr
#endif /* LANGUAGE == C */

#define	UDCCR_OFFSET	0x00
#define	UDCAR_OFFSET	0x04
#define	UDCOMP_OFFSET	0x08
#define	UDCIMP_OFFSET	0x0C
#define	UDCCS0_OFFSET	0x10
#define	UDCCS1_OFFSET	0x14
#define	UDCCS2_OFFSET	0x18
#define	UDCD0_OFFSET	0x1C
#define	UDCWC_OFFSET	0x20
#define	UDCDR_OFFSET	0x28
#define	UDCSR_OFFSET	0x30

/* UDCCR bits */

#define	UDCCR_SUSIM	bit(6)
#define	UDCCR_TIM	bit(5)
#define	UDCCR_RIM	bit(4)
#define	UDCCR_EIM	bit(3)
#define	UDCCR_RESIM	bit(2)
#define	UDCCR_UDA	bit(1)
#define	UDCCR_UDD	bit(0)

/* UDCCS0 bits */

#define	UDCCS0_SSE	bit(7)
#define	UDCCS0_SO	bit(6)
#define	UDCCS0_SE	bit(5)
#define	UDCCS0_DE	bit(4)
#define	UDCCS0_FST	bit(3)
#define	UDCCS0_SST	bit(2)
#define	UDCCS0_IPR	bit(1)
#define	UDCCS0_OPR	bit(0)

/* UDCCS1 bits */

#define	UDCCS1_RNE	bit(5)
#define	UDCCS1_FST	bit(4)
#define	UDCCS1_SST	bit(3)
#define	UDCCS1_RPE	bit(2)
#define	UDCCS1_RPC	bit(1)
#define	UDCCS1_RFS	bit(0)

/* UDCCS2 bits */

#define	UDCCS2_FST	bit(5)
#define	UDCCS2_SST	bit(4)
#define	UDCCS2_TUR	bit(3)
#define	UDCCS2_TPE	bit(2)
#define	UDCCS2_TPC	bit(1)
#define	UDCCS2_TFS	bit(0)

/* UDCSR bits */

#define	UDCSR_RSTIR	bit(5)
#define	UDCSR_RESIR	bit(4)
#define	UDCSR_SUSIR	bit(3)
#define	UDCSR_TIR	bit(2)
#define	UDCSR_RIR	bit(1)
#define	UDCSR_EIR	bit(0)

#endif /* SA11X0_UDC_H */
