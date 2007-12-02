/*
 * $Id$
 *
 * StrongARM SA-1110 GPIO Registers
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

#ifndef	SA11X0_GPIO_H
#define	SA11X0_GPIO_H

#include <openwince.h>

#if LANGUAGE == C
#include <stdint.h>
#endif

/* GPIO Registers */

#define	GPIO_BASE	0x90040000

#if LANGUAGE == C
typedef volatile struct GPIO_registers {
	uint32_t gplr;
	uint32_t gpdr;
	uint32_t gpsr;
	uint32_t gpcr;
	uint32_t grer;
	uint32_t gfer;
	uint32_t gedr;
	uint32_t gafr;
} GPIO_registers_t;

#ifdef SA11X0_UNMAPPED
#define	GPIO_pointer	((GPIO_registers_t*) GPIO_BASE)
#endif

#define	GPLR		GPIO_pointer->gplr
#define	GPDR		GPIO_pointer->gpdr
#define	GPSR		GPIO_pointer->gpsr
#define	GPCR		GPIO_pointer->gpcr
#define	GRER		GPIO_pointer->grer
#define	GFER		GPIO_pointer->gfer
#define	GEDR		GPIO_pointer->gedr
#define	GAFR		GPIO_pointer->gafr
#endif /* LANGUAGE == C */

#define	GPLR_OFFSET	0x00
#define	GPDR_OFFSET	0x04
#define	GPSR_OFFSET	0x08
#define	GPCR_OFFSET	0x0C
#define	GRER_OFFSET	0x10
#define	GFER_OFFSET	0x14
#define	GEDR_OFFSET	0x18
#define	GAFR_OFFSET	0x1C

#endif /* SA11X0_GPIO_H */
