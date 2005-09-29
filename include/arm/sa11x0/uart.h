/*
 * $Id$
 *
 * StrongARM SA-1110 UART Registers (Serial Port 1, 2 and 3)
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

#ifndef	SA11X0_UART_H
#define	SA11X0_UART_H

#include <openwince.h>

#if LANGUAGE == C
#include <stdint.h>
#endif

/* UART Registers (Serial Port 1, 2 and 3) */

#define	UART1_BASE	0x80010000
#define	UART2_BASE	0x80030000
#define	UART3_BASE	0x80050000

#if LANGUAGE == C
typedef volatile struct UART_registers {
	uint32_t utcr0;
	uint32_t utcr1;
	uint32_t utcr2;
	uint32_t utcr3;
	uint32_t utcr4;		/* only for Serial Port 2 */
	uint32_t utdr;
	uint32_t __reserved;
	uint32_t utsr0;
	uint32_t utsr1;
} UART_registers_t;

#ifdef SA11X0_UNMAPPED
#define	UART1_pointer	((UART_registers_t*) UART1_BASE)
#define	UART2_pointer	((UART_registers_t*) UART2_BASE)
#define	UART3_pointer	((UART_registers_t*) UART3_BASE)
#endif

#define	UTCR0		UART_pointer->utcr0
#define	UTCR1		UART_pointer->utcr1
#define	UTCR2		UART_pointer->utcr2
#define	UTCR3		UART_pointer->utcr3
#define	UTCR4		UART_pointer->utcr4		/* only for Serial Port 2 */
#define	UTDR		UART_pointer->utdr
#define	UTSR0		UART_pointer->utsr0
#define	UTSR1		UART_pointer->utsr1

/* Serial Port 1 */

#define	Ser1UTCR0	UART1_pointer->utcr0
#define	Ser1UTCR1	UART1_pointer->utcr1
#define	Ser1UTCR2	UART1_pointer->utcr2
#define	Ser1UTCR3	UART1_pointer->utcr3
#define	Ser1UTDR	UART1_pointer->utdr
#define	Ser1UTSR0	UART1_pointer->utsr0
#define	Ser1UTSR1	UART1_pointer->utsr1

/* Serial Port 2 */

#define	Ser2UTCR0	UART2_pointer->utcr0
#define	Ser2UTCR1	UART2_pointer->utcr1
#define	Ser2UTCR2	UART2_pointer->utcr2
#define	Ser2UTCR3	UART2_pointer->utcr3
#define	Ser2UTCR4	UART2_pointer->utcr4
#define	Ser2UTDR	UART2_pointer->utdr
#define	Ser2UTSR0	UART2_pointer->utsr0
#define	Ser2UTSR1	UART2_pointer->utsr1

/* Serial Port 3 */

#define	Ser3UTCR0	UART3_pointer->utcr0
#define	Ser3UTCR1	UART3_pointer->utcr1
#define	Ser3UTCR2	UART3_pointer->utcr2
#define	Ser3UTCR3	UART3_pointer->utcr3
#define	Ser3UTDR	UART3_pointer->utdr
#define	Ser3UTSR0	UART3_pointer->utsr0
#define	Ser3UTSR1	UART3_pointer->utsr1
#endif /* LANGUAGE == C */

#define	UTCR0_OFFSET	0x00
#define	UTCR1_OFFSET	0x04
#define	UTCR2_OFFSET	0x08
#define	UTCR3_OFFSET	0x0C
#define	UTCR4_OFFSET	0x10		/* only for Serial Port 2 */
#define	UTDR_OFFSET	0x14
#define	UTSR0_OFFSET	0x1C
#define	UTSR1_OFFSET	0x20

/* UTCR0 bits */

#define	UTCR0_TCE	bit(6)
#define	UTCR0_RCE	bit(5)
#define	UTCR0_SCE	bit(4)
#define	UTCR0_DSS	bit(3)
#define	UTCR0_SBS	bit(2)
#define	UTCR0_OES	bit(1)
#define	UTCR0_PE	bit(0)

/* UTCR3 bits */

#define	UTCR3_LBM	bit(5)
#define	UTCR3_TIE	bit(4)
#define	UTCR3_RIE	bit(3)
#define	UTCR3_BRK	bit(2)
#define	UTCR3_TXE	bit(1)
#define	UTCR3_RXE	bit(0)

/* UTCR4 bits */

#define	UTCR4_LPM	bit(1)
#define	UTCR4_HSE	bit(0)

/* UTSR0 bits */

#define	UTSR0_EIF	bit(5)
#define	UTSR0_REB	bit(4)
#define	UTSR0_RBB	bit(3)
#define	UTSR0_RID	bit(2)
#define	UTSR0_RFS	bit(1)
#define	UTSR0_TFS	bit(0)

/* UTSR1 bits */

#define	UTSR1_ROR	bit(5)
#define	UTSR1_FRE	bit(4)
#define	UTSR1_PRE	bit(3)
#define	UTSR1_TNF	bit(2)
#define	UTSR1_RNE	bit(1)
#define	UTSR1_TBY	bit(0)

#endif	/* SA11X0_UART_H */
