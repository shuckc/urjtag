/*
 * $Id$
 *
 * ARM specific declarations
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
 * [1] ARM Limited, "ARM Architecture Reference Manual", June 2000,
 *     Order Number: ARM DDI 0100E
 *
 */

#ifndef	ARM_H
#define	ARM_H

#include <common.h>

/* PSR bits - see 2.5 in [1] */

#define	PSR_N		bit(31)
#define	PSR_Z		bit(30)
#define	PSR_C		bit(29)
#define	PSR_V		bit(28)
#define	PSR_Q		bit(27)		/* E variants of the ARMV5 and above - see 2.5.1 in [1] */

#define	PSR_I		bit(7)
#define	PSR_F		bit(6)
#define	PSR_T		bit(5)
#define	PSR_MODE_MASK	0x0000001F
#define	PSR_MODE(x)	(x & PSR_MODE_MASK)

#define	PSR_MODE_USR	PSR_MODE(0x10)
#define	PSR_MODE_FIQ	PSR_MODE(0x11)
#define	PSR_MODE_IRQ	PSR_MODE(0x12)
#define	PSR_MODE_SVC	PSR_MODE(0x13)
#define	PSR_MODE_ABT	PSR_MODE(0x17)
#define	PSR_MODE_UND	PSR_MODE(0x1B)
#define	PSR_MODE_SYS	PSR_MODE(0x1F)	/* ARMV4 and above */

#endif /* ARM_H */
