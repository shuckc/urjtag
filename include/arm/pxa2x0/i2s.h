/*
 * $Id$
 *
 * XScale PXA250/PXA210 I2S Registers
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
 * [1] Intel Corporation, "Intel PXA250 and PXA210 Application Processors
 *     Developer's Manual", February 2002, Order Number: 278522-001
 *
 */

#ifndef	PXA2X0_I2S_H
#define	PXA2X0_I2S_H

#ifndef uint32_t
typedef	unsigned int	uint32_t;
#endif

/* I2S Registers */

#define	I2S_BASE	0x40400000

typedef volatile struct I2S_registers {
	uint32_t sacr0;
	uint32_t sacr1;
	uint32_t __reserved1;
	uint32_t sasr0;
	uint32_t __reserved2;
	uint32_t saimr;
	uint32_t saicr;
	uint32_t __reserved3[17];
	uint32_t sadiv;
	uint32_t __reserved4[7];
	uint32_t sadr;
} I2S_registers;

#ifndef I2S_pointer
#define	I2S_pointer	((I2S_registers*) I2S_BASE)
#endif

#define	SACR0		I2S_pointer->sacr0
#define	SACR1		I2S_pointer->sacr1
#define	SASR0		I2S_pointer->sasr0
#define	SAIMR		I2S_pointer->saimr
#define	SAICR		I2S_pointer->saicr
#define	SADIV		I2S_pointer->sadiv
#define	SADR		I2S_pointer->sadr

#endif	/* PXA2X0_I2S_H */
