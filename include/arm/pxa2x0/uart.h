/*
 * $Id$
 *
 * XScale PXA250/PXA210 UART (FFUART/BTUART/STUART) Declarations
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

#ifndef	PXA2X0_UART_H
#define	PXA2X0_UART_H

#include <common.h>

#if LANGUAGE == C
#include <stdint.h>
#endif

/* Common UART (FFUART/BTUART/STUART) Declarations */

#define	FFUART_BASE	0x40100000
#define	BTUART_BASE	0x40200000
#define	STUART_BASE	0x40700000

#if LANGUAGE == C
typedef volatile struct UART_registers {
	union {
		uint32_t rbr;
		uint32_t thr;
		uint32_t dll;
	};
	union {
		uint32_t ier;
		uint32_t dlh;
	};
	union {
		uint32_t iir;
		uint32_t fcr;
	};
	uint32_t lcr;
	uint32_t mcr;
	uint32_t lsr;
	uint32_t msr;
	uint32_t spr;
	uint32_t isr;
} UART_registers_t;

#ifdef PXA2X0_UNMAPPED
#define	FFUART_pointer	((UART_registers_t*) FFUART_BASE)
#define	BTUART_pointer	((UART_registers_t*) BTUART_BASE)
#define	STUART_pointer	((UART_registers_t*) STUART_BASE)
#endif

#define	RBR		UART_pointer->rbr
#define	THR		UART_pointer->thr
#define	IER		UART_pointer->ier
#define	IIR		UART_pointer->iir
#define	FCR		UART_pointer->fcr
#define	LCR		UART_pointer->lcr
#define	MCR		UART_pointer->mcr
#define	LSR		UART_pointer->lsr
#define	MSR		UART_pointer->msr
#define	SPR		UART_pointer->spr
#define	ISR		UART_pointer->isr
#define	DLL		UART_pointer->dll
#define	DLH		UART_pointer->dlh

/* FFUART */

#define	FFRBR		FFUART_pointer->rbr
#define	FFTHR		FFUART_pointer->thr
#define	FFIER		FFUART_pointer->ier
#define	FFIIR		FFUART_pointer->iir
#define	FFFCR		FFUART_pointer->fcr
#define	FFLCR		FFUART_pointer->lcr
#define	FFMCR		FFUART_pointer->mcr
#define	FFLSR		FFUART_pointer->lsr
#define	FFMSR		FFUART_pointer->msr
#define	FFSPR		FFUART_pointer->spr
#define	FFISR		FFUART_pointer->isr
#define	FFDLL		FFUART_pointer->dll
#define	FFDLH		FFUART_pointer->dlh

/* BTUART */

#define	BTRBR		BTUART_pointer->rbr
#define	BTTHR		BTUART_pointer->thr
#define	BTIER		BTUART_pointer->ier
#define	BTIIR		BTUART_pointer->iir
#define	BTFCR		BTUART_pointer->fcr
#define	BTLCR		BTUART_pointer->lcr
#define	BTMCR		BTUART_pointer->mcr
#define	BTLSR		BTUART_pointer->lsr
#define	BTMSR		BTUART_pointer->msr
#define	BTSPR		BTUART_pointer->spr
#define	BTISR		BTUART_pointer->isr
#define	BTDLL		BTUART_pointer->dll
#define	BTDLH		BTUART_pointer->dlh

/* STUART */

#define	STRBR		STUART_pointer->rbr
#define	STTHR		STUART_pointer->thr
#define	STIER		STUART_pointer->ier
#define	STIIR		STUART_pointer->iir
#define	STFCR		STUART_pointer->fcr
#define	STLCR		STUART_pointer->lcr
#define	STMCR		STUART_pointer->mcr
#define	STLSR		STUART_pointer->lsr
#define	STMSR		STUART_pointer->msr
#define	STSPR		STUART_pointer->spr
#define	STISR		STUART_pointer->isr
#define	STDLL		STUART_pointer->dll
#define	STDLH		STUART_pointer->dlh
#endif /* LANGUAGE == C */

#define	RBR_OFFSET	0x00
#define	THR_OFFSET	0x00
#define	IER_OFFSET	0x04
#define	IIR_OFFSET	0x08
#define	FCR_OFFSET	0x08
#define	LCR_OFFSET	0x0C
#define	MCR_OFFSET	0x10
#define	LSR_OFFSET	0x14
#define	MSR_OFFSET	0x18
#define	SPR_OFFSET	0x1C
#define	ISR_OFFSET	0x20
#define	DLL_OFFSET	0x00
#define	DLH_OFFSET	0x04

/* IER bits */

#define	IER_DMAE	bit(7)
#define IER_UUE		bit(6)
#define	IER_NRZE	bit(5)
#define	IER_RTOIE	bit(4)
#define	IER_MIE		bit(3)
#define	IER_RLSE	bit(2)
#define	IER_TIE		bit(1)
#define	IER_RAVIE	bit(0)

/* IIR bits - see Table 10-9 in [1] */

#define	IIR_FIFOES_MASK	bits(7,6)
#define	IIR_FIFOES(x)	bits_val(7,6,x)
#define	IIR_TOD		bit(3)
#define	IIR_IID_MASK	bits(2,1)
#define	IIR_IID(x)	bits_val(2,1,x)
#define	IIR_IP		bit(0)

/* FCR bits - see Table 10-11 in [1] */

#define	FCR_ITL_MASK	bits(7,6)
#define	FCR_ITL(x)	bits_val(7,6,x)
#define	FCR_RESETTF	bit(2)
#define	FCR_RESETRF	bit(1)
#define	FCR_TRFIFOE	bit(0)

/* LCR bits - see Table 10-12 in [1] */

#define	LCR_DLAB	bit(7)
#define	LCR_SB		bit(6)
#define	LCR_STKYP	bit(5)
#define	LCR_EPS		bit(4)
#define	LCR_PEN		bit(3)
#define	LCR_STB		bit(2)
#define	LCR_WLS_MASK	bits(1,0)
#define	LCR_WLS(x)	bits_val(1,0,x)

/* LSR bits */

#define	LSR_FIFOE	bit(7)
#define	LSR_TEMT	bit(6)
#define	LSR_TDRQ	bit(5)
#define	LSR_BI		bit(4)
#define	LSR_FE		bit(3)
#define	LSR_PE		bit(2)
#define	LSR_OE		bit(1)
#define	LSR_DR		bit(0)

/* MCR bits */

#define	MCR_LOOP	bit(4)
#define	MCR_OUT2	bit(3)
#define	MCR_OUT1	bit(2)
#define	MCR_RTS		bit(1)
#define	MCR_DTR		bit(0)

/* MSR bits */

#define	MSR_DCD		bit(7)
#define	MSR_RI		bit(6)
#define	MSR_DSR		bit(5)
#define	MSR_CTS		bit(4)
#define	MSR_DDCD	bit(3)
#define	MSR_TERI	bit(2)
#define	MSR_DDSR	bit(1)
#define	MSR_DCTS	bit(0)

/* SPR bits - see Table 10-16 in [1] */

#define	SPR_SP_MASK	bits(7,0)
#define	SPR_SP(x)	bits(7,0,x)

/* ISR bits */

#define	ISR_RXPL	bit(4)
#define	ISR_TXPL	bit(3)
#define	ISR_XMODE	bit(2)
#define	ISR_RCVEIR	bit(1)
#define	ISR_XMITIR	bit(0)

#endif /* PXA2X0_UART_H */
