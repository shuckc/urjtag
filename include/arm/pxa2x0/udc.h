/*
 * $Id$
 *
 * XScale PXA250/PXA210 UDC Registers
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
 * [1] Intel Corporation, "Intel PXA250 and PXA210 Application Processors
 *     Developer's Manual", February 2002, Order Number: 278522-001
 * [2] Intel Corporation, "Intel PXA250 and PXA210 Application Processors
 *     Specification Update", April 2002, Order Number: 278534-004
 *
 */

#ifndef	PXA2X0_UDC_H
#define	PXA2X0_UDC_H

#ifndef uint32_t
typedef	unsigned int	uint32_t;
#endif

/* UDC Registers */

#define	UDC_BASE	0x40600000

typedef volatile struct UDC_registers {
	uint32_t udccr;
	uint32_t __reserved1[3];
	uint32_t udccs[16];
	uint32_t uicr0;
	uint32_t uicr1;
	uint32_t usir0;
	uint32_t usir1;
	uint32_t ufnrh;
	uint32_t ufnrl;
	uint32_t ubcr2;
	uint32_t ubcr4;
	uint32_t ubcr7;
	uint32_t ubcr9;
	uint32_t ubcr12;
	uint32_t ubcr14;
	uint32_t uddr0;
	uint32_t __reserved2[7];
	uint32_t uddr5;
	uint32_t __reserved3[7];
	uint32_t uddr10;
	uint32_t __reserved4[7];
	uint32_t uddr15;
	uint32_t __reserved5[7];
	uint32_t uddr1;
	uint32_t __reserved6[31];
	uint32_t uddr2;
	uint32_t __reserved7[31];
	uint32_t uddr3;
	uint32_t __reserved8[127];
	uint32_t uddr4;
	uint32_t __reserved9[127];
	uint32_t uddr6;
	uint32_t __reserved10[31];
	uint32_t uddr7;
	uint32_t __reserved11[31];
	uint32_t uddr8;
	uint32_t __reserved12[127];
	uint32_t uddr9;
	uint32_t __reserved13[127];
	uint32_t uddr11;
	uint32_t __reserved14[31];
	uint32_t uddr12;
	uint32_t __reserved15[31];
	uint32_t uddr13;
	uint32_t __reserved16[127];
	uint32_t uddr14;
} UDC_registers;

#ifndef UDC_pointer
#define	UDC_pointer	((UDC_registers*) UDC_BASE)
#endif

#define	UDCCR		UDC_pointer->udccr
#define	UDCCS(i)	UDC_pointer->udccs[i]
#define	UFNRH		UDC_pointer->ufnrh
#define	UFNRL		UDC_pointer->ufnrl
#define	UBCR2		UDC_pointer->ubcr2
#define	UBCR4		UDC_pointer->ubcr4
#define	UBCR7		UDC_pointer->ubcr7
#define	UBCR9		UDC_pointer->ubcr9
#define	UBCR12		UDC_pointer->ubcr12
#define	UBCR14		UDC_pointer->ubcr14
#define	UDDR0		UDC_pointer->uddr0
#define	UDDR1		UDC_pointer->uddr1
#define	UDDR2		UDC_pointer->uddr2
#define	UDDR3		UDC_pointer->uddr3
#define	UDDR4		UDC_pointer->uddr4
#define	UDDR5		UDC_pointer->uddr5
#define	UDDR6		UDC_pointer->uddr6
#define	UDDR7		UDC_pointer->uddr7
#define	UDDR8		UDC_pointer->uddr8
#define	UDDR9		UDC_pointer->uddr9
#define	UDDR10		UDC_pointer->uddr10
#define	UDDR11		UDC_pointer->uddr11
#define	UDDR12		UDC_pointer->uddr12
#define	UDDR13		UDC_pointer->uddr13
#define	UDDR14		UDC_pointer->uddr14
#define	UDDR15		UDC_pointer->uddr15
#define	UICR0		UDC_pointer->uicr0
#define	UICR1		UDC_pointer->uicr1
#define	USIR0		UDC_pointer->usir0
#define	USIR1		UDC_pointer->usir1

#endif	/* PXA2X0_UDC_H */
