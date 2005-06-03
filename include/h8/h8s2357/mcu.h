/*
 * $Id$
 *
 * H8S/2357 MCU Registers
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
 * [1] Renesas Technology Corp., "Hitachi 16-Bit Single-chip Microcomputer
 *     H8S/2357 Series, H8S/2357F-ZTAT, H8S/2398F-ZTAT Hardware Manual",
 *     Rev. 5.0, 11/22/02, Order Number: ADE-602-146D
 *
 */

#ifndef H8S2357_MCU_H
#define H8S2357_MCU_H

#include <common.h>

#if LANGUAGE == C
#include <stdint.h>
#endif

/* MCU registers */

#define MCU_BASE	0xffffff38

#if LANGUAGE == C
typedef volatile struct MCU_registers {
	uint8_t sbycr;
	uint8_t syscr;
	uint8_t sckcr;
	uint8_t mdcr;
	uint8_t mstpcrh;
	uint8_t mstpcrl;
	uint8_t __reserved[4];
	uint8_t syscr2;
} MCU_registers_t;

#define MCU_pointer	((MCU_registers_t*) MCU_BASE)

#define SBYCR		MCU_pointer->sbycr
#define SYSCR		MCU_pointer->syscr
#define SCKCR		MCU_pointer->sckcr
#define MDCR		MCU_pointer->mdcr
#define MSTPCRH		MCU_pointer->mstpcrh
#define MSTPCRL		MCU_pointer->mstpcrl
#define SYSCR2		MCU_pointer->syscr2
#endif /* LANGUAGE == C */

#define SBYCR_OFFSET	0x00
#define SYSCR_OFFSET	0x01
#define SCKCR_OFFSET	0x02
#define MDCR_OFFSET	0x03
#define MSTPCRH_OFFSET	0x04
#define MSTPCRL_OFFSET	0x05
#define SYSCR2_OFFSET	0x0a

/* SBYCR bits */
#define SBYCR_SSBY		bit(7)
#define SBYCR_STS_MASK		bits(6,4)
#define SBYCR_STS(x)		bits_val(6,4,x)
#define get_SBYCR_STS(x)	bits_get(6,4,x)
#define SBYCR_OPE		bit(3)

/* SYSCR bits */
#define SYSCR_INTM_MASK		bits(5,4)
#define SYSCR_INTM(x)		bits_val(5,4,x)
#define get_SYSCR_INTM(x)	bits_get(5,4,x)
#define SYSCR_NMIEG		bit(3)
#define SYSCR_RAME		bit(0)

/* SCKCR bits */
#define SCKCR_PSTOP		bit(7)
#define SCKCR_SCK_MASK		bits(2,0)
#define SCKCR_SCK(x)		bits_val(2,0,x)
#define get_SYSCR_SCK(x)	bits_get(2,0,x)

/* MDCR bits */
#define MDCR_MDS_MASK		bits(2,0)
#define MDCR_MDS(x)		bits_val(2,0,x)
#define get_MDCR_MDS(x)		bits_get(2,0,x)

/* MSTPCRH bits */
#define MSTPCRH_MSTP15		bit(7)
#define MSTPCRH_MSTP14		bit(6)
#define MSTPCRH_MSTP13		bit(5)
#define MSTPCRH_MSTP12		bit(4)
#define MSTPCRH_MSTP11		bit(3)
#define MSTPCRH_MSTP10		bit(2)
#define MSTPCRH_MSTP9		bit(1)
#define MSTPCRH_MSTP8		bit(0)

/* MSTPCRL bits */
#define MSTPCRL_MSTP7		bit(7)
#define MSTPCRL_MSTP6		bit(6)
#define MSTPCRL_MSTP5		bit(5)
#define MSTPCRL_MSTP4		bit(4)
#define MSTPCRL_MSTP3		bit(3)
#define MSTPCRL_MSTP2		bit(2)
#define MSTPCRL_MSTP1		bit(1)
#define MSTPCRL_MSTP0		bit(0)

#endif /* H8S2357_MCU_H */
