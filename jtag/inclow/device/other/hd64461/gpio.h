/*
 * $Id$
 *
 * Hitachi HD64461 Pin Function Controller & I/O Port Registers
 * Copyright (C) 2004 Marcel Telka
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *     * Neither the name of the copyright holders nor the names of their
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Written by Marcel Telka <marcel@telka.sk>, 2004.
 *
 * Documentation:
 * [1] Hitachi, Ltd., "HD64461 Windows(R) CE Intelligent Peripheral Controller",
 *     1st Edition, July 1998, Order Number: ADE-602-076
 *
 */

#ifndef HD64461_GPIO_H
#define	HD64461_GPIO_H

#include <openwince.h>

#if LANGUAGE == C
#include <stdint.h>
#endif

/* Pin Function Controller & I/O Port Registers */

#if LANGUAGE == C
typedef volatile struct GPIO_registers {
	uint16_t gpacr;
	uint16_t gpbcr;
	uint16_t gpccr;
	uint16_t gpdcr;
	uint16_t __reserved1[4];
	uint8_t gpadr;
	uint8_t __reserved2;
	uint8_t gpbdr;
	uint8_t __reserved3;
	uint8_t gpcdr;
	uint8_t __reserved4;
	uint8_t gpddr;
	uint8_t __reserved5[9];
	uint8_t gpaicr;
	uint8_t __reserved6;
	uint8_t gpbicr;
	uint8_t __reserved7;
	uint8_t gpcicr;
	uint8_t __reserved8;
	uint8_t gpdicr;
	uint8_t __reserved9[25];
	uint8_t gpaisr;
	uint8_t __reserved10;
	uint8_t gpbisr;
	uint8_t __reserved11;
	uint8_t gpcisr;
	uint8_t __reserved12;
	uint8_t gpdisr;
} GPIO_registers_t;
#endif /* LANGUAGE == C */

#define	GPACR_OFFSET			0x00
#define	GPBCR_OFFSET			0x02
#define	GPCCR_OFFSET			0x04
#define	GPDCR_OFFSET			0x06
#define	GPADR_OFFSET			0x10
#define	GPBDR_OFFSET			0x12
#define	GPCDR_OFFSET			0x14
#define	GPDDR_OFFSET			0x16
#define	GPAICR_OFFSET			0x20
#define	GPBICR_OFFSET			0x22
#define	GPCICR_OFFSET			0x24
#define	GPDICR_OFFSET			0x26
#define	GPAISR_OFFSET			0x40
#define	GPBISR_OFFSET			0x42
#define	GPCISR_OFFSET			0x44
#define	GPDISR_OFFSET			0x46

#endif /* HD64461_GPIO_H */
