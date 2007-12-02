/*
 * $Id$
 *
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
 * [1] Intel Corporation, "3 Volt Synchronous Intel Strata Flash Memory 28F640K3, 28F640K18,
 *     28F128K3, 28F128K18, 28F256K3, 28F256K18 (x16)", June 2002, Order Number: 290737-005
 *
 */

#ifndef	FLASH_28FXXXK_H
#define	FLASH_28FXXXK_H

#include <openwince.h>

/* RCR bits - see Table 4. in [1] */

#define	RCR_RM		bit(15)
#define	RCR_LC_MASK	bits(14,11)
#define	RCR_LC(x)	bits_val(14,11,x)
#define	RCR_WP		bit(10)
#define	RCR_DH		bit(9)
#define	RCR_WD		bit(8)
#define	RCR_BS		bit(7)
#define	RCR_CE		bit(6)
#define	RCR_BL_MASK	bits(2,0)
#define	RCR_BL(x)	bits_val(2,0,x)

/* SR bits - see Table 7. in [1] */

#define	SR_RDY		bit(7)
#define	SR_ES		bit(6)
#define	SR_EE		bit(5)
#define	SR_PE		bit(4)
#define	SR_VE		bit(3)
#define	SR_PS		bit(2)
#define	SR_LE		bit(1)
#define	SR_PS		bit(0)

#endif /* FLASH_28FXXXK_H */
