/*
 * $Id$
 *
 * Hitachi HD64461 Color LCD Controller Registers
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

#ifndef HD64461_LCDC_H
#define	HD64461_LCDC_H

#include <common.h>

#if LANGUAGE == C
#include <stdint.h>
#endif

/* Color LCD Controller Registers */

#if LANGUAGE == C
typedef volatile struct LCDC_registers {
	uint16_t lcdcbar;
	uint16_t lcdclor;
	uint16_t lcdccr;
	uint16_t __reserved1[5];
	uint16_t ldr1;
	uint16_t ldr2;
	uint16_t ldhncr;
	uint16_t ldhnsr;
	uint16_t ldvntr;
	uint16_t ldvndr;
	uint16_t ldvspr;
	uint16_t ldr3;
	uint16_t crtvtr;
	uint16_t crtvrsr;
	uint16_t vrtvrer;
	uint16_t __reserved2[5];
	uint16_t cptwar;
	uint16_t cptwdr;
	uint16_t cptrar;
	uint16_t cptrdr;
	uint16_t __reserved3[4];
	uint16_t grdor;
	uint16_t grscr;
	uint16_t grcfgr;
	uint16_t lnsarh;
	uint16_t lnsarl;
	uint16_t lnaxlr;
	uint16_t lndgr;
	uint16_t lnaxr;
	uint16_t lnertr;
	uint16_t lnmdr;
	uint16_t bbtssarh;
	uint16_t bbtssarl;
	uint16_t bbtdsarh;
	uint16_t bbtdsarl;
	uint16_t bbtdwr;
	uint16_t bbtdhr;
	uint16_t bbtparh;
	uint16_t bbtparl;
	uint16_t bbtmarh;
	uint16_t bbtmarl;
	uint16_t bbtropr;
	uint16_t bbtmdr;
} LCDC_registers_t;
#endif /* LANGUAGE == C */

#define	LCDCBAR_OFFSET			0x00
#define	LCDCLOR_OFFSET			0x02
#define	LCDCCR_OFFSET			0x04
#define	LDR1_OFFSET			0x10
#define	LDR2_OFFSET			0x12
#define	LDHNCR_OFFSET			0x14
#define	LDHNSR_OFFSET			0x16
#define	LDVNTR_OFFSET			0x18
#define	LDVNDR_OFFSET			0x1A
#define	LDVSPR_OFFSET			0x1C
#define	LDR3_OFFSET			0x1E
#define	CRTVTR_OFFSET			0x20
#define	CRTVRSR_OFFSET			0x22
#define	CRTVRER_OFFSET			0x24
#define	CPTWAR_OFFSET			0x30
#define	CPTWDR_OFFSET			0x32
#define	CPTRAR_OFFSET			0x34
#define	CPTRDR_OFFSET			0x36
#define	GRDOR_OFFSET			0x40
#define	GRSCR_OFFSET			0x42
#define	GRCFGR_OFFSET			0x44
#define	LNSARH_OFFSET			0x46
#define	LNSARL_OFFSET			0x48
#define	LNAXLR_OFFSET			0x4A
#define	LNDGR_OFFSET			0x4C
#define	LNAXR_OFFSET			0x4E
#define	LNERTR_OFFSET			0x50
#define	LNMDR_OFFSET			0x52
#define	BBTSSARH_OFFSET			0x54
#define	BBTSSARL_OFFSET			0x56
#define	BBTDSARH_OFFSET			0x58
#define	BBTDSARL_OFFSET			0x5A
#define	BBTDWR_OFFSET			0x5C
#define	BBTDHR_OFFSET			0x5E
#define	BBTPARH_OFFSET			0x60
#define	BBTPARL_OFFSET			0x62
#define	BBTMARH_OFFSET			0x64
#define	BBTMARL_OFFSET			0x66
#define	BBTROPR_OFFSET			0x68
#define	BBTMDR_OFFSET			0x6A

/* LCDCBAR bits */
#define	LCDCBAR_BAD_MASK		bits(13,0)
#define	LCDCBAR_BAD(x)			bits_val(13,0,x)
#define	get_LCDCBAR_BAD(x)		bits_get(13,0,x)

/* LCDCLOR bits */
#define	LCDCLOR_LO_MASK			bits(10,0)
#define	LCDCLOR_LO(x)			bits_val(10,0,x)
#define	get_LCDCLOR_LO(x)		bits_get(10,0,x)

/* LCDCCR bits */
#define	LCDCCR_STBACK			bit(10)
#define	LCDCCR_STREQ			bit(8)
#define	LCDCCR_MOFF			bit(7)
#define	LCDCCR_REFSEL			bit(6)
#define	LCDCCR_EPON			bit(5)
#define	LCDCCR_SPON			bit(4)
#define	LCDCCR_DSPSEL_MASK		bits(2,0)
#define	LCDCCR_DSPSEL(x)		bits_val(2,0,x)
#define	get_LCDCCR_DSPSEL(x)		bits_get(2,0,x)

/* LDR1 bits */
#define	LDR1_DINV			bit(8)
#define	LDR1_DON			bit(0)

/* LDR2 bits */
#define	LDR2_CC1			bit(7)
#define	LDR2_CC2			bit(6)
#define	LDR2_LM_MASK			bits(2,0)
#define	LDR2_LM(x)			bits_val(2,0,x)
#define	get_LDR2_LM(x)			bits_get(2,0,x)

/* LDHNCR bits */
#define	LDHNCR_NHD_MASK			bits(15,8)
#define	LDHNCR_NHD(x)			bits_val(15,8,x)
#define	get_LDHNCR_NHD(x)		bits_get(15,8,x)
#define	LDHNCR_NHT_MASK			bits(7,0)
#define	LDHNCR_NHT(x)			bits_val(7,0,x)
#define	get_LDHNCR_NHT(x)		bits_get(7,0,x)

/* LDHNSR bits */
#define	LDHNSR_HSW_MASK			bits(11,8)
#define	LDHNSR_HSW(x)			bits_val(11,8,x)
#define	get_LDHNSR_HSW(x)		bits_get(11,8,x)
#define	LDHNSR_HSP_MASK			bits(7,0)
#define	LDHNSR_HSP(x)			bits_val(7,0,x)
#define	get_LDHNSR_HSP(x)		bits_get(7,0,x)

/* LDVNTR bits */
#define	LDVNTR_VTL_MASK			bits(9,0)
#define	LDVNTR_VTL(x)			bits_val(9,0,x)
#define	get_LDVNTR_VTL(x)		bits_get(9,0,x)

/* LDVNDR bits */
#define LDVNDR_VDL_MASK			bits(9,0)
#define	LDVNDR_VDL(x)			bits_val(9,0,x)
#define	get_LDVNDR_VDL(x)		bits_get(9,0,x)

/* LDVSPR bits */
#define	LDVSPR_VSP_MASK			bits(9,0)
#define	LDVSPR_VSP(x)			bits_val(9,0,x)
#define	get_LDVSPR_VSP(x)		bits_get(9,0,x)

/* LDR3 bits */
#define	LDR3_CS_MASK			bits(9,5)
#define	LDR3_CS(x)			bits_val(9,5,x)
#define	get_LDR3_CS(x)			bits_get(9,5,x)
#define	LDR3_CG_MASK			bits(3,0)
#define	LDR3_CG(x)			bits_val(3,0,x)
#define	get_LDR3_CG(x)			bits_get(3,0,x)

/* CRTVTR bits */
#define	CRTVTR_CRTVTR_MASK		bits(9,0)
#define	CRTVTR_CRTVTR(x)		bits_val(9,0,x)
#define	get_CRTVTR_CRTVTR(x)		bits_get(9,0,x)

/* CRTVRSR bits */
#define	CRTVRSR_CRTVRSR_MASK		bits(9,0)
#define	CRTVRSR_CRTVRSR(x)		bits_val(9,0,x)
#define	get_CRTVRSR_CRTVRSR(x)		bits_get(9,0,x)

/* CRTVRER bits */
#define	CRTVRER_CRTVRER_MASK		bits(3,0)
#define	CRTVRER_CRTVRER(x)		bits_val(3,0,x)
#define	get_CRTVRER_CRTVRER(x)		bits_get(3,0,x)

/* CPTWAR bits */
#define	CPTWAR_WRITE_PALETTE_NUM_MASK	bits(15,8)
#define	CPTWAR_WRITE_PALETTE_NUM(x)	bits_val(15,8,x)
#define	get_CPTWAR_WRITE_PALETTE_NUM(x)	bits_get(15,8,x)

/* CPTWDR bits */
#define	CPTWDR_WRITE_PALETTE_D_MASK	bits(5,0)
#define	CPTWDR_WRITE_PALETTE_D(x)	bits_val(5,0,x)
#define	get_CPTWDR_WRITE_PALETTE_D(x)	bits_get(5,0,x)

/* CPTRAR bits */
#define	CPTRAR_READ_PALETTE_NUM_MASK	bits(15,8)
#define	CPTRAR_READ_PALETTE_NUM(x)	bits_val(15,8,x)
#define	get_CPTRAR_READ_PALETTE_NUM(x)	bits_get(15,8,x)

/* CPTRDR bits */
#define	CPTRDR_READ_PALETTE_D_MASK	bits(5,0)
#define	CPTRDR_READ_PALETTE_D(x)	bits_val(5,0,x)
#define	get_CPTRDR_READ_PALETTE_D(x)	bits_get(5,0,x)

/* GRDOR bits */
#define	GRDOR_GRDOR_MASK		bits(10,0)
#define	GRDOR_GRDOR(x)			bits_val(10,0,x)
#define	get_GRDOR_GRDOR(x)		bits_get(10,0,x)

/* GRCFGR bits */
#define	GRCFGR_ACCSTATUS		bit(4)
#define	GRCFGR_ACCRESET			bit(3)
#define	GRCFGR_ACCSTART_MASK		bits(2,1)
#define	GRCFGR_ACCSTART(x)		bits_val(2,1,x)
#define	get_GRCFGR_ACCSTART(x)		bits_get(2,1,x)
#define	GRCFGR_COLORDEPTH		bit(0)

/* LNSARH bits */
#define	LNSARH_LNSARH_MASK		bits(2,0)
#define	LNSARH_LNSARH(x)		bits_val(2,0,x)
#define	get_LNSARH_LNSARH(x)		bits_get(2,0,x)

/* LNAXLR bits */
#define	LNAXLR_LNAXLR_MASK		bits(10,0)
#define	LNAXLR_LNAXLR(x)		bits_val(10,0,x)
#define	get_LNAXLR_LNAXLR(x)		bits_get(10,0,x)

/* LNDGR bits */
#define	LNDGR_LNDGR_SIGN		bit(15)
#define	LNDGR_LNDGR_MASK		bits(10,0)
#define	LNDGR_LNDGR(x)			bits_val(10,0,x)
#define	get_LNDGR_LNDGR(x)		bits_get(10,0,x)

/* LNAXR bits */
#define	LNAXR_LNAXR_MASK		bits(11,0)
#define	LNAXR_LNAXR(x)			bits_val(10,0,x)
#define	get_LNAXR_LNAXR(x)		bits_get(10,0,x)

/* LNERTR bits */
#define	LNERTR_LNERTR_SIGN		bit(15)
#define	LNERTR_LNERTR_MASK		bits(10,0)
#define	LNERTR_LNERTR(x)		bits_val(10,0,x)
#define	get_LNERTR_LNERTR(x)		bits_get(10,0,x)

/* LNMDR bits */
#define	LNMDR_LNMDR_MASK		bits(1,0)
#define	LNMDR_LNMDR(x)			bits_val(1,0,x)
#define	get_LNMDR_LNMDR(x)		bits_get(1,0,x)

/* BBTSSARH bits */
#define	BBTSSARH_BBTSSARH_MASK		bits(2,0)
#define	BBTSSARH_BBTSSARH(x)		bits_val(2,0,x)
#define	get_BBTSSARH_BBTSSARH(x)	bits_get(2,0,x)

/* BBTDSARH bits */
#define	BBTDSARH_BBTDSARH_MASK		bits(2,0)
#define	BBTDSARH_BBTDSARH(x)		bits_val(2,0,x)
#define	get_BBTDSARH_BBTDSARH(x)	bits_get(2,0,x)

/* BBTDWR bits */
#define	BBTDWR_BBTDWR_MASK		bits(10,0)
#define	BBTDWR_BBTDWR(x)		bits_val(10,0,x)
#define	get_BBTDWR_BBTDWR(x)		bits_get(10,0,x)

/* BBTDHR bits */
#define	BBTDHR_BBTDHR_MASK		bits(10,0)
#define	BBTDHR_BBTDHR(x)		bits_val(10,0,x)
#define	get_BBTDHR_BBTDHR(x)		bits_get(10,0,x)

/* BBTPARH bits */
#define	BBTPARH_BBTPARH_MASK		bits(2,0)
#define	BBTPARH_BBTPARH(x)		bits_val(2,0,x)
#define	get_BBTPARH_BBTPARH(x)		bits_get(2,0,x)

/* BBTMARH bits */
#define	BBTMARH_BBTMARH_MASK		bits(2,0)
#define	BBTMARH_BBTMARH(x)		bits_val(2,0,x)
#define	get_BBTMARH_BBTMARH(x)		bits_get(2,0,x)

/* BBTMDR bits */
#define	BBTMDR_MSKENABLE		bit(5)
#define	BBTMDR_PATSELECT		bit(4)
#define	BBTMDR_SCREENSELECT_MASK	bits(3,2)
#define	BBTMDR_SCREENSELECT(x)		bits_val(3,2,x)
#define	get_BBTMDR_SCREENSELECT(x)	bits_get(3,2,x)
#define	BBTMDR_SCANDRCT			bit(0)

#endif /* HD64461_LCDC_H */
