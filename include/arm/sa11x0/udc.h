/*
 * $Id$
 *
 * StrongARM SA-1110 UDC Registers
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
 *
 */

#ifndef	SA11X0_UDC_H
#define	SA11X0_UDC_H

#ifndef uint32_t
typedef	unsigned int	uint32_t;
#endif

/* UDC Registers (Serial Port 0) */

#define	UDC_BASE	0x80000000

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
} UDC_registers;

#ifndef UDC_pointer
#define	UDC_pointer	((UDC_registers*) UDC_BASE)
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

/* UDCCR bits */

#define	UDCCR_SUSIM	0x40
#define	UDCCR_TIM	0x20
#define	UDCCR_RIM	0x10
#define	UDCCR_EIM	0x08
#define	UDCCR_RESIM	0x04
#define	UDCCR_UDA	0x02
#define	UDCCR_UDD	0x01

/* UDCCS0 bits */

#define	UDCCS0_SSE	0x80
#define	UDCCS0_SO	0x40
#define	UDCCS0_SE	0x20
#define	UDCCS0_DE	0x10
#define	UDCCS0_FST	0x08
#define	UDCCS0_SST	0x04
#define	UDCCS0_IPR	0x02
#define	UDCCS0_OPR	0x01

/* UDCCS1 bits */

#define	UDCCS1_RNE	0x20
#define	UDCCS1_FST	0x10
#define	UDCCS1_SST	0x08
#define	UDCCS1_RPE	0x04
#define	UDCCS1_RPC	0x02
#define	UDCCS1_RFS	0x01

/* UDCCS2 bits */

#define	UDCCS2_FST	0x20
#define	UDCCS2_SST	0x10
#define	UDCCS2_TUR	0x08
#define	UDCCS2_TPE	0x04
#define	UDCCS2_TPC	0x02
#define	UDCCS2_TFS	0x01

/* UDCSR bits */

#define	UDCSR_RSTIR	0x20
#define	UDCSR_RESIR	0x10
#define	UDCSR_SUSIR	0x08
#define	UDCSR_TIR	0x04
#define	UDCSR_RIR	0x02
#define	UDCSR_EIR	0x01

#endif	/* SA11X0_UDC_H */
