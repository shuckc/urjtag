/*
 * $Id$
 *
 * H8S/2357 PPG Registers
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

#ifndef H8S2357_PPG_H
#define H8S2357_PPG_H

#include <openwince.h>

#if LANGUAGE == C
#include <stdint.h>
#endif

/* PPG registers */

#define PPG_BASE	0xffffff46

#if LANGUAGE == C
typedef volatile struct PPG_registers {
	uint8_t pcr;
	uint8_t pmr;
	uint8_t nderh;
	uint8_t nedrl;
	uint8_t podrh;
	uint8_t podrl;
	uint8_t ndrh;
	uint8_t ndrl;
	uint8_t ndrh_d;
	uint8_t ndrl_d;
} PPG_registers_t;

#define PPG_pointer	((PPG_registers_t*) PPG_BASE)

#define PCR		PPG_pointer->pcr
#define PMR		PPG_pointer->pmr
#define NDERH		PPG_pointer->nderh
#define NEDRL		PPG_pointer->nedrl
#define PODRH		PPG_pointer->podrh
#define PODRL		PPG_pointer->podrl
#define NDRH		PPG_pointer->ndrh
#define NDRL		PPG_pointer->ndrl
#define NDRH_d		PPG_pointer->ndrh_d
#define NDRL_d		PPG_pointer->ndrl_d
#endif /* LANGUAGE == C */

#define PCR_OFFSET	0x00
#define PMR_OFFSET	0x01
#define NDERH_OFFSET	0x02
#define NEDRL_OFFSET	0x03
#define PODRH_OFFSET	0x04
#define PODRL_OFFSET	0x05
#define NDRH_OFFSET	0x06
#define NDRL_OFFSET	0x07
#define NDRH_OFFSET_d	0x08
#define NDRL_OFFSET_d	0x09

/* PCR bits */
#define PCR_G3CMS_MASK		bits(7,6)
#define PCR_G3CMS(x)		bits_val(7,6,x)
#define get_PCR_G3CMS(x)	bits_get(7,6,x)
#define PCR_G2CMS_MASK		bits(5,4)
#define PCR_G2CMS(x)		bits_val(5,4,x)
#define get_PCR_G2CMS(x)	bits_get(5,4,x)
#define PCR_G1CMS_MASK		bits(3,2)
#define PCR_G1CMS(x)		bits_val(3,2,x)
#define get_PCR_G1CMS(x)	bits_get(3,2,x)
#define PCR_G0CMS_MASK		bits(1,0)
#define PCR_G0CMS(x)		bits_val(1,0,x)
#define get_PCR_G0CMS(x)	bits_get(1,0,x)

/* PMR bits */
#define PMR_G3INV		bit(7)
#define PMR_G2INV		bit(6)
#define PMR_G1INV		bit(5)
#define PMR_G0INV		bit(4)
#define PMR_G3NOV		bit(3)
#define PMR_G2NOV		bit(2)
#define PMR_G1NOV		bit(1)
#define PMR_G0NOV		bit(0)

/* NDERH bits */
#define NDERH_NDER15		bit(7)
#define NDERH_NDER14		bit(6)
#define NDERH_NDER13		bit(5)
#define NDERH_NDER12		bit(4)
#define NDERH_NDER11		bit(3)
#define NDERH_NDER10		bit(2)
#define NDERH_NDER9		bit(1)
#define NDERH_NDER8		bit(0)

/* NDERL bits */
#define NDERL_NDER7		bit(7)
#define NDERL_NDER6		bit(6)
#define NDERL_NDER5		bit(5)
#define NDERL_NDER4		bit(4)
#define NDERL_NDER3		bit(3)
#define NDERL_NDER2		bit(2)
#define NDERL_NDER1		bit(1)
#define NDERL_NDER0		bit(0)

/* PODRH bits */
#define PODRH_POD15		bit(7)
#define PODRH_POD14		bit(6)
#define PODRH_POD13		bit(5)
#define PODRH_POD12		bit(4)
#define PODRH_POD11		bit(3)
#define PODRH_POD10		bit(2)
#define PODRH_POD9		bit(1)
#define PODRH_POD8		bit(0)

/* PODRL bits */
#define PODRL_POD7		bit(7)
#define PODRL_POD6		bit(6)
#define PODRL_POD5		bit(5)
#define PODRL_POD4		bit(4)
#define PODRL_POD3		bit(3)
#define PODRL_POD2		bit(2)
#define PODRL_POD1		bit(1)
#define PODRL_POD0		bit(0)

/* NDRH bits */
#define NDRH_NDR15		bit(7)
#define NDRH_NDR14		bit(6)
#define NDRH_NDR13		bit(5)
#define NDRH_NDR12		bit(4)
#define NDRH_NDR11		bit(3)
#define NDRH_NDR10		bit(2)
#define NDRH_NDR9		bit(1)
#define NDRH_NDR8		bit(0)

/* NDRL bits */
#define NDRL_NDR7		bit(7)
#define NDRL_NDR6		bit(6)
#define NDRL_NDR5		bit(5)
#define NDRL_NDR4		bit(4)
#define NDRL_NDR3		bit(3)
#define NDRL_NDR2		bit(2)
#define NDRL_NDR1		bit(1)
#define NDRL_NDR0		bit(0)

#endif /* H8S2357_PPG_H */
