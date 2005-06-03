/*
 * $Id$
 *
 * H8/3048 ADC Registers
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

#ifndef H83048_ADC_H
#define H83048_ADC_H

#include <common.h>

#if LANGUAGE == C
#include <stdint.h>
#endif

/* ADC registers */

#define ADC_BASE	0xffffe0

#if LANGUAGE == C
typedef volatile struct ADC_registers {
	uint8_t addrah;
	uint8_t addral;
	uint8_t addrbh;
	uint8_t addrbl;
	uint8_t addrch;
	uint8_t addrcl;
	uint8_t addrdh;
	uint8_t addrdl;
	uint8_t adcsr;
	uint8_t adcr;
} ADC_registers_t;

#define ADC_pointer	((ADC_registers_t*) ADC_BASE)

#define ADDRAH		ADC_pointer->addrah
#define ADDRAL		ADC_pointer->addral
#define ADDRBH		ADC_pointer->addrbh
#define ADDRBL		ADC_pointer->addrbl
#define ADDRCH		ADC_pointer->addrch
#define ADDRCL		ADC_pointer->addrcl
#define ADDRDH		ADC_pointer->addrdh
#define ADDRDL		ADC_pointer->addrdl
#define ADCSR		ADC_pointer->adcsr
#define ADCR		ADC_pointer->adcr
#endif /* LANGUAGE == C */

#define ADDRAH_OFFSET	0x00
#define ADDRAL_OFFSET	0x01
#define ADDRBH_OFFSET	0x02
#define ADDRBL_OFFSET	0x03
#define ADDRCH_OFFSET	0x04
#define ADDRCL_OFFSET	0x05
#define ADDRDH_OFFSET	0x06
#define ADDRDL_OFFSET	0x07
#define ADCSR_OFFSET	0x08
#define ADCR_OFFSET	0x09

/* ADDR bits */
#define ADDR_AD9	bit(7)
#define ADDR_AD8	bit(6)
#define ADDR_AD7	bit(5)
#define ADDR_AD6	bit(4)
#define ADDR_AD5	bit(3)
#define ADDR_AD4	bit(2)
#define ADDR_AD3	bit(1)
#define ADDR_AD2	bit(0)
#define ADDR_AD1	bit(7)
#define ADDR_AD0	bit(6)

/* ADCSR bits */
#define ADCSR_ADF	bit(7)
#define ADCSR_ADIE	bit(6)
#define ADCSR_ADST	bit(5)
#define ADCSR_SCAN	bit(4)
#define ADCSR_CKS	bit(3)
#define ADCSR_CH_MASK	bits(2,0)
#define ADCSR_CH(x)	bits_val(2,0,x)
#define get_ADCSR_CH(x)	bits_get(2,0,x)

/* ADCR bits */
#define ADCR_TRGE	bit(7)

#endif /* H83048_ADC_H */
