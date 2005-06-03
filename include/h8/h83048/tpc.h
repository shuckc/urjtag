/*
 * $Id$
 *
 * H8/3048 TPC Registers
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
 * [1] Renesas Technology Corp., "Hitachi Single-Chip Microcomputer
 *     H8/3048 Series, H8/3048F-ZTAT Hardware Manual",
 *     Rev. 6.0, 9/3/2002, Order Number: ADE-602-073E
 *
 */

#ifndef H83048_TPC_H
#define H83048_TPC_H

#include <common.h>

#if LANGUAGE == C
#include <stdint.h>
#endif

/* TPC registers */

#define TPC_BASE	0xffffa0

#if LANGUAGE == C
typedef volatile struct TPC_registers {
	uint8_t tpmr;
	uint8_t tpcr;
	uint8_t nderb;
	uint8_t ndera;
	uint8_t ndrb;
	uint8_t ndra;
	uint8_t ndrb_d;
	uint8_t ndra_d;
} TPC_registers_t;

#define TPC_pointer	((TPC_registers_t*) TPC_BASE)

#define TPMR		TPC_pointer->tpmr
#define TPCR		TPC_pointer->tpcr
#define NDERB		TPC_pointer->nderb
#define NDERA		TPC_pointer->ndera
#define NDRB		TPC_pointer->ndrb
#define NDRA		TPC_pointer->ndra
#define NDRB_D		TPC_pointer->ndrb_d
#define NDRA_D		TPC_pointer->ndra_d
#endif /* LANGUAGE == C */

#define TPMR_OFFSET	0x00
#define TPCR_OFFSET	0x01
#define NDERB_OFFSET	0x02
#define NDERA_OFFSET	0x03
#define NDRB_OFFSET	0x04
#define NDRA_OFFSET	0x05
#define NDRB_D_OFFSET	0x06
#define NDRA_D_OFFSET	0x07

/* TPMR bits */
#define TPMR_G3NOV		bit(3)
#define TPMR_G2NOV		bit(2)
#define TPMR_G1NOV		bit(1)
#define TPMR_G0NOV		bit(0)

/* TPCR bits */
#define TPCR_G3CMS_MASK		bits(7,6)
#define TPCR_G3CMS(x)		bits_val(7,6,x)
#define get_TPCR_G3CMS(x)	bits_get(7,6,x)
#define TPCR_G2CMS_MASK		bits(5,4)
#define TPCR_G2CMS(x)		bits_val(5,4,x)
#define get_TPCR_G2CMS(x)	bits_get(5,4,x)
#define TPCR_G1CMS_MASK		bits(3,2)
#define TPCR_G1CMS(x)		bits_val(3,2,x)
#define get_TPCR_G1CMS(x)	bits_get(3,2,x)
#define TPCR_G0CMS_MASK		bits(1,0)
#define TPCR_G0CMS(x)		bits_val(1,0,x)
#define get_TPCR_G0CMS(x)	bits_get(1,0,x)

/* NDERB bits */
#define NDERB_NDER15		bit(7)
#define NDERB_NDER14		bit(6)
#define NDERB_NDER13		bit(5)
#define NDERB_NDER12		bit(4)
#define NDERB_NDER11		bit(3)
#define NDERB_NDER10		bit(2)
#define NDERB_NDER9		bit(1)
#define NDERB_NDER8		bit(0)

/* NDERA bits */
#define NDERA_NDER7		bit(7)
#define NDERA_NDER6		bit(6)
#define NDERA_NDER5		bit(5)
#define NDERA_NDER4		bit(4)
#define NDERA_NDER3		bit(3)
#define NDERA_NDER2		bit(2)
#define NDERA_NDER1		bit(1)
#define NDERA_NDER0		bit(0)

/* NDRB bits */
#define NDRB_NDR15		bit(7)
#define NDRB_NDR14		bit(6)
#define NDRB_NDR13		bit(5)
#define NDRB_NDR12		bit(4)
#define NDRB_NDR11		bit(3)
#define NDRB_NDR10		bit(2)
#define NDRB_NDR9		bit(1)
#define NDRB_NDR8		bit(0)

/* NDRA bits */
#define NDRA_NDR7		bit(7)
#define NDRA_NDR6		bit(6)
#define NDRA_NDR5		bit(5)
#define NDRA_NDR4		bit(4)
#define NDRA_NDR3		bit(3)
#define NDRA_NDR2		bit(2)
#define NDRA_NDR1		bit(1)
#define NDRA_NDR0		bit(0)

#endif /* H83048_TPC_H */
