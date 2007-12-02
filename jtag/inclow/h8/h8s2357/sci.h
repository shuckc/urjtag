/*
 * $Id$
 *
 * H8S/2357 SCI Registers
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

#ifndef H8S2357_SCI_H
#define H8S2357_SCI_H

#include <openwince.h>

#if LANGUAGE == C
#include <stdint.h>
#endif

/* SCI registers */

#define SCI0_BASE	0xffffff78
#define SCI1_BASE	0xffffff80
#define SCI2_BASE	0xffffff88

#if LANGUAGE == C
typedef volatile struct SCI_registers {
	uint8_t smr;
	uint8_t brr;
	uint8_t scr;
	uint8_t tdr;
	uint8_t ssr;
	uint8_t rdr;
	uint8_t scmr;
} SCI_registers_t;

#define SCI0_pointer	((SCI_registers_t*) SCI0_BASE)
#define SCI1_pointer	((SCI_registers_t*) SCI1_BASE)
#define SCI2_pointer	((SCI_registers_t*) SCI2_BASE)

#define SMR0		SCI0_pointer->smr
#define BRR0		SCI0_pointer->brr
#define SCR0		SCI0_pointer->scr
#define TDR0		SCI0_pointer->tdr
#define SSR0		SCI0_pointer->ssr
#define RDR0		SCI0_pointer->rdr
#define SCMR0		SCI0_pointer->scmr

#define SMR1		SCI1_pointer->smr
#define BRR1		SCI1_pointer->brr
#define SCR1		SCI1_pointer->scr
#define TDR1		SCI1_pointer->tdr
#define SSR1		SCI1_pointer->ssr
#define RDR1		SCI1_pointer->rdr
#define SCMR1		SCI1_pointer->scmr

#define SMR2		SCI2_pointer->smr
#define BRR2		SCI2_pointer->brr
#define SCR2		SCI2_pointer->scr
#define TDR2		SCI2_pointer->tdr
#define SSR2		SCI2_pointer->ssr
#define RDR2		SCI2_pointer->rdr
#define SCMR2		SCI2_pointer->scmr
#endif /* LANGUAGE == C */

#define SMR_OFFSET	0x00
#define BRR_OFFSET	0x01
#define SCR_OFFSET	0x02
#define TDR_OFFSET	0x03
#define SSR_OFFSET	0x04
#define RDR_OFFSET	0x05
#define SCMR_OFFSET	0x06

/* SMR bits */
#define SMR_CA		bit(7)
#define SMR_GM		bit(7)
#define SMR_CHR		bit(6)
#define SMR_PE		bit(5)
#define SMR_OE		bit(4)
#define SMR_STOP	bit(3)
#define SMR_MP		bit(2)
#define SMR_CKS_MASK	bits(1,0)
#define SMR_CKS(x)	bits_val(1,0,x)
#define get_SMR_CKS(x)	bits_get(1,0,x)

/* SCR bits */
#define SCR_TIE		bit(7)
#define SCR_RIE		bit(6)
#define SCR_TE		bit(5)
#define SCR_RE		bit(4)
#define SCR_MPIE	bit(3)
#define SCR_TEIE	bit(2)
#define SCR_CKE_MASK	bits(1,0)
#define SCR_CKE(x)	bits_val(1,0,x)
#define get_SCR_CKE(x)	bits_get(1,0,x)

/* SSR bits */
#define SSR_TDRE	bit(7)
#define SSR_RDRF	bit(6)
#define SSR_ORER	bit(5)
#define SSR_FER		bit(4)
#define SSR_ERS		bit(4)
#define SSR_PER		bit(3)
#define SSR_TEND	bit(2)
#define SSR_MPB		bit(1)
#define SSR_MPBT	bit(0)

/* SCMR bits */
#define SCMR_SDIR	bit(3)
#define SCMR_SINV	bit(2)
#define SCMR_SMIF	bit(0)

#endif /* H8S2357_SCI_H */
