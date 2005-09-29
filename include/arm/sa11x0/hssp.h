/*
 * $Id$
 *
 * StrongARM SA-1110 ICP - HSSP Registers
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

#ifndef	SA11X0_HSSP_H
#define	SA11X0_HSSP_H

#include <openwince.h>

#if LANGUAGE == C
#include <stdint.h>
#endif

/* ICP - HSSP Registers (Serial Port 2) */

#define	HSSP_BASE	0x80040060

#if LANGUAGE == C
typedef volatile struct HSSP_registers {
	uint32_t hscr0;
	uint32_t hscr1;
	uint32_t __reserved1;
	uint32_t hsdr;
	uint32_t __reserved2;
	uint32_t hssr0;
	uint32_t hssr1;
} HSSP_registers_t;

#ifdef SA11X0_UNMAPPED
#define	HSSP_pointer	((HSSP_registers_t*) HSSP_BASE)
#endif

#define	HSCR0		HSSP_pointer->hscr0
#define	HSCR1		HSSP_pointer->hscr1
#define	HSDR		HSSP_pointer->hsdr
#define	HSSR0		HSSP_pointer->hssr0
#define	HSSR1		HSSP_pointer->hssr1
#endif /* LANGUAGE == C */

#define	HSCR0_OFFSET	0x00
#define	HSCR1_OFFSET	0x04
#define	HSDR_OFFSET	0x0C
#define	HSSR0_OFFSET	0x14
#define	HSSR1_OFFSET	0x18

/* HSCR0 bits */

#define	HSCR0_AME	bit(7)
#define	HSCR0_TIE	bit(6)
#define	HSCR0_RIE	bit(5)
#define	HSCR0_RXE	bit(4)
#define	HSCR0_TXE	bit(3)
#define	HSCR0_TUS	bit(2)
#define	HSCR0_LBM	bit(1)
#define	HSCR0_ITR	bit(0)

/* HSSR0 bits */

#define	HSSR0_FRE	bit(5)
#define	HSSR0_RFS	bit(4)
#define	HSSR0_TFS	bit(3)
#define	HSSR0_RAB	bit(2)
#define	HSSR0_TUR	bit(1)
#define	HSSR0_EIF	bit(0)

/* HSSR1 bits */

#define	HSSR1_ROR	bit(6)
#define	HSSR1_CRE	bit(5)
#define	HSSR1_EOF	bit(4)
#define	HSSR1_TNF	bit(3)
#define	HSSR1_RNE	bit(2)
#define	HSSR1_TBY	bit(1)
#define	HSSR1_RSY	bit(0)

#endif /* SA11X0_HSSP_H */
