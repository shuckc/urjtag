/*
 * $Id$
 *
 * ARM specific declarations
 * Copyright (C) 2002, 2003 ETC s.r.o.
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
 * Written by Marcel Telka <marcel@telka.sk>, 2002, 2003.
 *
 * Documentation:
 * [1] ARM Limited, "ARM Architecture Reference Manual", June 2000,
 *     Order Number: ARM DDI 0100E
 *
 */

#ifndef	ARM_H
#define	ARM_H

#include <common.h>

/* PSR bits - see A2.5 in [1] */

#define	PSR_N		bit(31)
#define	PSR_Z		bit(30)
#define	PSR_C		bit(29)
#define	PSR_V		bit(28)
#define	PSR_Q		bit(27)		/* E variants of the ARMV5 and above - see A2.5.1 in [1] */

#define	PSR_I		bit(7)
#define	PSR_F		bit(6)
#define	PSR_T		bit(5)
#define	PSR_MODE_MASK	bits(4,0)
#define	PSR_MODE(x)	((x) & PSR_MODE_MASK)

#define	PSR_MODE_USR	PSR_MODE(0x10)
#define	PSR_MODE_FIQ	PSR_MODE(0x11)
#define	PSR_MODE_IRQ	PSR_MODE(0x12)
#define	PSR_MODE_SVC	PSR_MODE(0x13)
#define	PSR_MODE_ABT	PSR_MODE(0x17)
#define	PSR_MODE_UND	PSR_MODE(0x1B)
#define	PSR_MODE_SYS	PSR_MODE(0x1F)	/* ARMV4 and above */

/* System Control Coprocessor (SCC) Register 1: Control Register (CR) bits - see B2.4 in [1] */

#define	SCC_CR_L4	bit(15)
#define	SCC_CR_RR	bit(14)
#define	SCC_CR_V	bit(13)
#define	SCC_CR_I	bit(12)
#define	SCC_CR_Z	bit(11)
#define	SCC_CR_F	bit(10)
#define	SCC_CR_R	bit(9)
#define	SCC_CR_S	bit(8)
#define	SCC_CR_B	bit(7)
#define	SCC_CR_L	bit(6)
#define	SCC_CR_D	bit(5)
#define	SCC_CR_P	bit(4)
#define	SCC_CR_W	bit(3)
#define	SCC_CR_C	bit(2)
#define	SCC_CR_A	bit(1)
#define	SCC_CR_M	bit(0)

#endif /* ARM_H */
