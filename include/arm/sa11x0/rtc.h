/*
 * $Id$
 *
 * StrongARM SA-1110 Real-Time Clock Registers
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

#ifndef	SA11X0_RTC_H
#define	SA11X0_RTC_H

#include <common.h>

#if LANGUAGE == C
#include <stdint.h>
#endif

/* Real-Time Clock Registers */

#define	RTC_BASE	0x90010000

#if LANGUAGE == C
typedef volatile struct RTC_registers {
	uint32_t rtar;
	uint32_t rcnr;
	uint32_t rttr;
	uint32_t __reserved;
	uint32_t rtsr;
} RTC_registers;

#ifdef SA11X0_UNMAPPED
#define	RTC_pointer	((RTC_registers*) RTC_BASE)
#endif

#define	RTAR		RTC_pointer->rtar
#define	RCNR		RTC_pointer->rcnr
#define	RTTR		RTC_pointer->rttr
#define	RTSR		RTC_pointer->rtsr
#endif /* LANGUAGE == C */

#define	RTAR_OFFSET	0x00
#define	RCNR_OFFSET	0x04
#define	RTTR_OFFSET	0x08
#define	RTSR_OFFSET	0x10

/* RTSR bits */

#define	RTSR_HZE	bit(3)
#define	RTSR_ALE	bit(2)
#define	RTSR_HZ		bit(1)
#define	RTSR_AL		bit(0)

#endif /* SA11X0_RTC_H */
