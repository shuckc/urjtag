/*
 * $Id$
 *
 * StrongARM SA-1110 UART Registers (Serial Port 1, 2 and 3)
 * Copyright (C) 2002 ETC s.r.o.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 *
 * Written by Marcel Telka <marcel@telka.sk>, 2002.
 *
 * Documentation:
 * [1] Intel Corporation, "Intel StrongARM SA-1110 Microprocessor
 *     Developer's Manual", October 2001, Order Number: 278240-004
 * [2] Intel Corporation, "Intel StrongARM SA-1110 Microprocessor
 *     Specification Update", December 2001, Order Number: 278259-023
 *
 */

#ifndef	SA11X0_UART_H
#define	SA11X0_UART_H

#include <common.h>

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
} UART_registers;

#ifndef UART1_pointer
#define	UART1_pointer	((UART_registers*) UART1_BASE)
#endif
#ifndef UART2_pointer
#define	UART2_pointer	((UART_registers*) UART2_BASE)
#endif
#ifndef UART3_pointer
#define	UART3_pointer	((UART_registers*) UART3_BASE)
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
