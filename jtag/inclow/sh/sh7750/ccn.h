/*
 * $Id$
 *
 * Renesas SH7750 CCN Registers
 * Copyright (C) 2003 Marcel Telka
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
 * Written by Marcel Telka <marcel@telka.sk>, 2003.
 *
 * Documentation:
 * [1] Renesas Technology Corp., "Hitachi SuperH RISC engine SH7750 Series
 *     SH7750, SH7750S, SH7750R Hardware Manual", ADE-602-124E, Rev. 6.0, 7/10/2002
 *
 */

#ifndef	SH7750_CCN_H
#define	SH7750_CCN_H

#include <openwince.h>

#if LANGUAGE == C
#include <stdint.h>
#endif

/* CCN Registers */

#if LANGUAGE == C
/* see Table A.1 in [1] */
typedef volatile struct CCN_registers {
	uint32_t pteh;
	uint32_t ptel;
	uint32_t ttb;
	uint32_t tea;
	uint32_t mmucr;
	union {
		uint32_t _reserved1;
		uint8_t basra;
	} _basra;
	union {
		uint32_t _reserved2;
		uint8_t basrb;
	} _basrb;
	uint32_t ccr;
	uint32_t tra;
	uint32_t expevt;
	uint32_t intevt;
	uint32_t _reserved3[2];
	uint32_t ptea;
	uint32_t qacr0;
	uint32_t qacr1;
} CCN_registers_t;

#define	PTEH			CCN_pointer->pteh
#define	PTEL			CCN_pointer->ptel
#define	TTB			CCN_pointer->ttb
#define	TEA			CCN_pointer->tea
#define	MMUCR			CCN_pointer->mmucr
#define	BASRA			CCN_pointer->_basra.basra
#define	BASRB			CCN_pointer->_basrb.basrb
#define	CCR			CCN_pointer->ccr
#define	TRA			CCN_pointer->tra
#define	EXPEVT			CCN_pointer->expevt
#define	INTEVT			CCN_pointer->intevt
#define	PTEA			CCN_pointer->ptea
#define	QACR0			CCN_pointer->qacr0
#define	QACR1			CCN_pointer->qacr1
#endif /* LANGUAGE == C */

#define	PTEH_OFFSET		0x00
#define	PTEL_OFFSET		0x04
#define	TTB_OFFSET		0x08
#define	TEA_OFFSET		0x0C
#define	MMUCR_OFFSET		0x10
#define	BASRA_OFFSET		0x14
#define	BASRB_OFFSET		0x18
#define	CCR_OFFSET		0x1C
#define	TRA_OFFSET		0x20
#define	EXPEVT_OFFSET		0x24
#define	INTEVT_OFFSET		0x28
#define	PTEA_OFFSET		0x34
#define	QACR0_OFFSET		0x38
#define	QACR1_OFFSET		0x3C

/* PTEH bits - see Figure 3.2 in [1] */

#define	PTEH_VPN_MASK		bits(31,10)
#define	PTEH_VPN(x)		bits_val(31,10,x)
#define	get_PTEH_VPN(x)		bits_get(31,10,x)
#define	PTEH_ASID_MASK		bits(7,0)
#define	PTEH_ASID(x)		bits_val(7,0,x)
#define	get_PTEH_ASID(x)	bits_get(7,0,x)

/* PTEL bits - see Figure 3.2 in [1] */

#define	PTEL_PPN_MASK		bits(28,10)
#define	PTEL_PPN(x)		bits_val(28,10,x)
#define	get_PTEL_PPN(x)		bits_get(28,10,x)
#define	PTEL_V			bit(8)
#define	PTEL_PR_MASK		bits(6,5)
#define	PTEL_PR(x)		bits_val(6,5,x)
#define	get_PTEL_PR(x)		bits_get(6,5,x)
#define	PTEL_C			bit(3)
#define	PTEL_D			bit(2)
#define	PTEL_SH			bit(1)
#define	PTEL_WT			bit(0)

/* MMUCR bits - see Figure 3.2 in [1] */

#define	MMUCR_LRUI_MASK		bits(31,26)
#define	MMUCR_LRUI(x)		bits_val(31,26,x)
#define	get_MMUCR_LRUI(x)	bits_get(31,26,x)
#define	MMUCR_URB_MASK		bits(23,18)
#define	MMUCR_URB(x)		bits_val(23,18,x)
#define	get_MMUCR_URB(x)	bits_get(23,18,x)
#define	MMUCR_URC_MASK		bits(15,10)
#define	MMUCR_URC(x)		bits_val(15,10,x)
#define	get_MMUCR_URC(x)	bits_get(15,10,x)
#define	MMUCR_SQMD		bit(9)
#define	MMUCR_SV		bit(8)
#define	MMUCR_TI		bit(2)
#define	MMUCR_AT		bit(0)

/* CCR bits - see Figure 4.1 in [1] */

#if defined(SH7750R)
#define	CCR_EMODE		bit(31)
#endif /* SH7750R only */
#define	CCR_IIX			bit(15)
#define	CCR_ICI			bit(11)
#define	CCR_ICE			bit(8)
#define	CCR_OIX			bit(7)
#define	CCR_ORA			bit(5)
#define	CCR_OCI			bit(3)
#define	CCR_CB			bit(2)
#define	CCR_WT			bit(1)
#define	CCR_OCE			bit(0)

/* PTEA bits - see Figure 3.2 in [1] */

#define	PTEA_TC			bit(3)
#define	PTEA_SA_MASK		bits(2,0)
#define	PTEA_SA(x)		bits_val(2,0,x)
#define	get_PTEA_SA(x)		bits_get(2,0,x)

/* QACR0 bits - see Figure 4.1 in [1] */

#define	QACR0_AREA_MASK		bits(4,2)
#define	QACR0_AREA(x)		bits_val(4,2,x)
#define	get_QACR0_AREA(x)	bits_get(4,2,x)

/* QACR1 bits - see Figure 4.1 in [1] */

#define	QACR1_AREA_MASK		bits(4,2)
#define	QACR1_AREA(x)		bits_val(4,2,x)
#define	get_QACR1_AREA(x)	bits_get(4,2,x)

#endif /* SH7750_CCN_H */
