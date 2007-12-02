/*
 * $Id$
 *
 * H8/3048 Refresh Controller (RC) Registers
 * Copyright (C) 2005 Elcom s.r.o.
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
 * 3. Neither the name of the copyright holders nor the names of its contributors
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
 * Written by Branislav Petrovsky <brano111@szm.sk>, 2005.
 *
 * Documentation:
 * [1] Renesas Technology Corp., "Hitachi Single-Chip Microcomputer
 *     H8/3048 Series, H8/3048F-ZTAT Hardware Manual",
 *     Rev. 6.0, 9/3/2002, Order Number: ADE-602-073E
 *
 */

#ifndef H83048_RC_H
#define H83048_RC_H

#include <openwince.h>

#ifndef __ASSEMBLY__
#include <stdint.h>
#endif

/* RC registers */

#define RC_BASE		0xffffac

#ifndef __ASSEMBLY__
typedef volatile struct RC_registers {
	uint8_t rfshcr;
	uint8_t rtmcsr;
	uint8_t rtcnt;
	uint8_t rtcor;
} RC_registers_t;

#define RC_pointer	((RC_registers_t*) RC_BASE)

#define RFSHCR		RC_pointer->rfshcr
#define RTMCSR		RC_pointer->rtmcsr
#define RTCNT		RC_pointer->rtcnt
#define RTCOR		RC_pointer->rtcor
#endif /* __ASSEMBLY__ */

#define RFSHCR_OFFSET	0x00
#define RTMCSR_OFFSET	0x01
#define RTCNT_OFFSET	0x02
#define RTCOR_OFFSET	0x03

/* RFSHCR bits */
#define RFSHCR_SRFMD		bit(7)
#define RFSHCR_PSRAME		bit(6)
#define RFSHCR_DRAME		bit(5)
#define RFSHCR_CASWE		bit(4)
#define RFSHCR_M9M8		bit(3)
#define RFSHCR_RFSHE		bit(2)
#define RFSHCR_RCYCE		bit(0)

/* RTMCSR bits */
#define RTMCSR_CMF		bit(7)
#define RTMCSR_CMIE		bit(6)
#define RTMCSR_CKS_MASK		bits(5,3)
#define RTMCSR_CKS(x)		bits_val(5,3,x)
#define get_RTMCSR_CKS(x)	bits_get(5,3,x)

#endif /* H83048_RC_H */
