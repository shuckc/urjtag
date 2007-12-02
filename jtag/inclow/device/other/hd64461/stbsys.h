/*
 * $Id$
 *
 * Hitachi HD64461 Standby Mode and System Configuration Registers
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

#ifndef HD64461_STBSYS_H
#define	HD64461_STBSYS_H

#include <openwince.h>

#if LANGUAGE == C
#include <stdint.h>
#endif

/* Standby Mode and System Configuration Registers */

#if LANGUAGE == C
typedef volatile struct STBSYS_registers {
	uint16_t stbcr;
	uint16_t syscr;
	uint16_t scpucr;
} STBSYS_registers_t;
#endif /* LANGUAGE == C */

#define	STBCR_OFFSET			0x00
#define	SYSCR_OFFSET			0x02
#define	SCPUCR_OFFSET			0x04

/* STBCR bits */
#define	STBCR_CKIO_STBY			bit(13)
#define	STBCR_SAFECKE_IST		bit(12)
#define	STBCR_SLCKE_IST			bit(11)
#define	STBCR_SAFECKE_OST		bit(10)
#define	STBCR_SLCKE_OST			bit(9)
#define	STBCR_SMIAST			bit(8)
#define	STBCR_SLCDST			bit(7)
#define	STBCR_SPC0ST			bit(6)
#define	STBCR_SPC1ST			bit(5)
#define	STBCR_SAFEST			bit(4)
#define	STBCR_STM0ST			bit(3)
#define	STBCR_STM1ST			bit(2)
#define	STBCR_SIRST			bit(1)
#define	STBCR_SURTSD			bit(0)

/* SYSCR bits */
#define	SYSCR_SCPU_BUS_IGAT		bit(13)
#define	SYSCR_SPTA_IR			bit(7)
#define	SYSCR_SPTA_TM			bit(6)
#define	SYSCR_SPTB_UR			bit(5)
#define	SYSCR_WAIT_CTL_SEL		bit(4)
#define	SYSCR_SMODE1			bit(1)
#define	SYSCR_SMODE0			bit(0)

/* SCPUCR bits */
#define	SCPUCR_SPDSTOF			bit(15)
#define	SCPUCR_SPDSTIG			bit(14)
#define	SCPUCR_SPCSTOF			bit(13)
#define	SCPUCR_SPCSTIG			bit(12)
#define	SCPUCR_SPBSTOF			bit(11)
#define	SCPUCR_SPBSTIG			bit(10)
#define	SCPUCR_SPASTOF			bit(9)
#define	SCPUCR_SPASTIG			bit(8)
#define	SCPUCR_SLCDSTIG			bit(7)
#define	SCPUCR_SCPU_CS56_EP		bit(6)
#define	SCPUCR_SCPU_CMD_EP		bit(5)
#define	SCPUCR_SCPU_ADDR_EP		bit(4)
#define	SCPUCR_SCPDPU			bit(3)
#define	SCPUCR_SCPU_A2319_EP		bit(0)

#endif /* HD64461_STBSYS_H */
