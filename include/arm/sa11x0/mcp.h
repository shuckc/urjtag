/*
 * $Id$
 *
 * StrongARM SA-1110 MCP Registers
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

#ifndef	SA11X0_MCP_H
#define	SA11X0_MCP_H

#include <common.h>

#if LANGUAGE == C
#include <stdint.h>
#endif

/* MCP Registers (Serial Port 4) */

#define	MCP_BASE	0x80060000

#if LANGUAGE == C
typedef volatile struct MCP_registers {
	uint32_t mccr0;
	uint32_t __reserved1;
	uint32_t mcdr0;
	uint32_t mcdr1;
	uint32_t mcdr2;
	uint32_t __reserved2;
	uint32_t mcsr;
} MCP_registers_t;

#ifdef SA11X0_UNMAPPED
#define	MCP_pointer	((MCP_registers_t*) MCP_BASE)
#endif

#define	MCCR0		MCP_pointer->mccr0
#define	MCDR0		MCP_pointer->mcdr0
#define	MCDR1		MCP_pointer->mcdr1
#define	MCDR2		MCP_pointer->mcdr2
#define	MCSR		MCP_pointer->mcsr
#endif /* LANGUAGE == C */

#define	MCCR0_OFFSET	0x00
#define	MCDR0_OFFSET	0x08
#define	MCDR1_OFFSET	0x0C
#define	MCDR2_OFFSET	0x10
#define	MCSR_OFFSET	0x18

#endif /* SA11X0_MCP_H */
