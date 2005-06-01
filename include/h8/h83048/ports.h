/*
 * $Id$
 *
 * H8/3048 PORTS Registers
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

#ifndef H83048_PORTS_H
#define H83048_PORTS_H

#include <common.h>

#if LANGUAGE == C
#include <stdint.h>
#endif

/* PORTS registers */

#define PORT_BASE	0xffffc0

#if LANGUAGE == C
typedef struct PORT_registers {
	uint8_t p1ddr;
	uint8_t p2ddr;
	uint8_t p1dr;
	uint8_t p2dr;
	uint8_t p3ddr;
	uint8_t p4ddr;
	uint8_t p3dr;
	uint8_t p4dr;
	uint8_t p5ddr;
	uint8_t p6ddr;
	uint8_t p5dr;
	uint8_t p6dr;
	uint8_t __reserved1;
	uint8_t p8ddr;
	uint8_t p7dr;
	uint8_t p8dr;
	uint8_t p9ddr;
	uint8_t paddr;
	uint8_t p9dr;
	uint8_t padr;
	uint8_t pbddr;
	uint8_t __reserved2;
	uint8_t pbdr;
	uint8_t __reserved3;
	uint8_t p2pcr;
	uint8_t __reserved4;
	uint8_t p4pcr;
	uint8_t p5pcr;
} PORT_registers_t;

#define PORT_pointer	((PORT_registers_t*) PORT_BASE)

#define P1DDR		PORT_pointer->p1ddr
#define P2DDR		PORT_pointer->p2ddr
#define P1DR		PORT_pointer->p1dr
#define P2DR		PORT_pointer->p2dr
#define P3DDR		PORT_pointer->p3ddr
#define P4DDR		PORT_pointer->p4ddr
#define P3DR		PORT_pointer->p3dr
#define P4DR		PORT_pointer->p4dr
#define P5DDR		PORT_pointer->p5ddr
#define P6DDR		PORT_pointer->p6ddr
#define P5DR		PORT_pointer->p5dr
#define P6DR		PORT_pointer->p6dr
#define P8DDR		PORT_pointer->p8ddr
#define P7DR		PORT_pointer->p7dr
#define P8DR		PORT_pointer->p8dr
#define P9DDR		PORT_pointer->p9ddr
#define PADDR		PORT_pointer->paddr
#define P9DR		PORT_pointer->p9dr
#define PADR		PORT_pointer->padr
#define PBDDR		PORT_pointer->pbddr
#define PBDR		PORT_pointer->pbdr
#define P2PCR		PORT_pointer->p2pcr
#define P4PCR		PORT_pointer->p4pcr
#define P5PCR		PORT_pointer->p5pcr
#endif /* LANGUAGE == C */

#define P1DDR_OFFSET	0x00
#define P2DDR_OFFSET	0x01
#define P1DR_OFFSET	0x02
#define P2DR_OFFSET	0x03
#define P3DDR_OFFSET	0x04
#define P4DDR_OFFSET	0x05
#define P3DR_OFFSET	0x06
#define P4DR_OFFSET	0x07
#define P5DDR_OFFSET	0x08
#define P6DDR_OFFSET	0x09
#define P5DR_OFFSET	0x0a
#define P6DR_OFFSET	0x0b
#define P8DDR_OFFSET	0x0d
#define P7DR_OFFSET	0x0e
#define P8DR_OFFSET	0x0f
#define P9DDR_OFFSET	0x10
#define PADDR_OFFSET	0x11
#define P9DR_OFFSET	0x12
#define PADR_OFFSET	0x13
#define PBDDR_OFFSET	0x14
#define PBDR_OFFSET	0x16
#define P2PCR_OFFSET	0x18
#define P4PCR_OFFSET	0x1a
#define P5PCR_OFFSET	0x1b

/* P1DDR bits */
#define P1DDR_P17DDR	bit(7)
#define P1DDR_P16DDR	bit(6)
#define P1DDR_P15DDR	bit(5)
#define P1DDR_P14DDR	bit(4)
#define P1DDR_P13DDR	bit(3)
#define P1DDR_P12DDR	bit(2)
#define P1DDR_P11DDR	bit(1)
#define P1DDR_P10DDR	bit(0)

/* P1DR bits */
#define P1DR_P17	bit(7)
#define P1DR_P16	bit(6)
#define P1DR_P15	bit(5)
#define P1DR_P14	bit(4)
#define P1DR_P13	bit(3)
#define P1DR_P12	bit(2)
#define P1DR_P11	bit(1)
#define P1DR_P10	bit(0)

/* P2DDR bits */
#define P2DDR_P27DDR	bit(7)
#define P2DDR_P26DDR	bit(6)
#define P2DDR_P25DDR	bit(5)
#define P2DDR_P24DDR	bit(4)
#define P2DDR_P23DDR	bit(3)
#define P2DDR_P22DDR	bit(2)
#define P2DDR_P21DDR	bit(1)
#define P2DDR_P20DDR	bit(0)

/* P2DR bits */
#define P2DR_P27	bit(7)
#define P2DR_P26	bit(6)
#define P2DR_P25	bit(5)
#define P2DR_P24	bit(4)
#define P2DR_P23	bit(3)
#define P2DR_P22	bit(2)
#define P2DR_P21	bit(1)
#define P2DR_P20	bit(0)

/* P3DDR bits */
#define P3DDR_P37DDR	bit(7)
#define P3DDR_P36DDR	bit(6)
#define P3DDR_P35DDR	bit(5)
#define P3DDR_P34DDR	bit(4)
#define P3DDR_P33DDR	bit(3)
#define P3DDR_P32DDR	bit(2)
#define P3DDR_P31DDR	bit(1)
#define P3DDR_P30DDR	bit(0)

/* P3DR bits */
#define P3DR_P37	bit(7)
#define P3DR_P36	bit(6)
#define P3DR_P35	bit(5)
#define P3DR_P34	bit(4)
#define P3DR_P33	bit(3)
#define P3DR_P32	bit(2)
#define P3DR_P31	bit(1)
#define P3DR_P30	bit(0)

/* P4DDR bits */
#define P4DDR_P47DDR	bit(7)
#define P4DDR_P46DDR	bit(6)
#define P4DDR_P45DDR	bit(5)
#define P4DDR_P44DDR	bit(4)
#define P4DDR_P43DDR	bit(3)
#define P4DDR_P42DDR	bit(2)
#define P4DDR_P41DDR	bit(1)
#define P4DDR_P40DDR	bit(0)

/* P4DR bits */
#define P4DR_P47	bit(7)
#define P4DR_P46	bit(6)
#define P4DR_P45	bit(5)
#define P4DR_P44	bit(4)
#define P4DR_P43	bit(3)
#define P4DR_P42	bit(2)
#define P4DR_P41	bit(1)
#define P4DR_P40	bit(0)

/* P5DDR bits */
#define P5DDR_P53DDR	bit(3)
#define P5DDR_P52DDR	bit(2)
#define P5DDR_P51DDR	bit(1)
#define P5DDR_P50DDR	bit(0)

/* P5DR bits */
#define P5DR_P53	bit(3)
#define P5DR_P52	bit(2)
#define P5DR_P51	bit(1)
#define P5DR_P50	bit(0)

/* P6DDR bits */
#define P6DDR_P66DDR	bit(6)
#define P6DDR_P65DDR	bit(5)
#define P6DDR_P64DDR	bit(4)
#define P6DDR_P63DDR	bit(3)
#define P6DDR_P62DDR	bit(2)
#define P6DDR_P61DDR	bit(1)
#define P6DDR_P60DDR	bit(0)

/* P6DR bits */
#define P6DR_P66	bit(6)
#define P6DR_P65	bit(5)
#define P6DR_P64	bit(4)
#define P6DR_P63	bit(3)
#define P6DR_P62	bit(2)
#define P6DR_P61	bit(1)
#define P6DR_P60	bit(0)

/* P7DR bits */
#define P7DR_P77	bit(7)
#define P7DR_P76	bit(6)
#define P7DR_P75	bit(5)
#define P7DR_P74	bit(4)
#define P7DR_P73	bit(3)
#define P7DR_P72	bit(2)
#define P7DR_P71	bit(1)
#define P7DR_P70	bit(0)

/* P8DDR bits */
#define P8DDR_P84DDR	bit(4)
#define P8DDR_P83DDR	bit(3)
#define P8DDR_P82DDR	bit(2)
#define P8DDR_P81DDR	bit(1)
#define P8DDR_P80DDR	bit(0)

/* P8DR bits */
#define P8DR_P84	bit(4)
#define P8DR_P83	bit(3)
#define P8DR_P82	bit(2)
#define P8DR_P81	bit(1)
#define P8DR_P80	bit(0)

/* P9DDR bits */
#define P9DDR_P95DDR	bit(5)
#define P9DDR_P94DDR	bit(4)
#define P9DDR_P93DDR	bit(3)
#define P9DDR_P92DDR	bit(2)
#define P9DDR_P91DDR	bit(1)
#define P9DDR_P90DDR	bit(0)

/* P9DR bits */
#define P9DR_P95	bit(5)
#define P9DR_P94	bit(4)
#define P9DR_P93	bit(3)
#define P9DR_P92	bit(2)
#define P9DR_P91	bit(1)
#define P9DR_P90	bit(0)

/* PADDR bits */
#define PADDR_PA7DDR	bit(7)
#define PADDR_PA6DDR	bit(6)
#define PADDR_PA5DDR	bit(5)
#define PADDR_PA4DDR	bit(4)
#define PADDR_PA3DDR	bit(3)
#define PADDR_PA2DDR	bit(2)
#define PADDR_PA1DDR	bit(1)
#define PADDR_PA0DDR	bit(0)

/* PADR bits */
#define PADR_PA7	bit(7)
#define PADR_PA6	bit(6)
#define PADR_PA5	bit(5)
#define PADR_PA4	bit(4)
#define PADR_PA3	bit(3)
#define PADR_PA2	bit(2)
#define PADR_PA1	bit(1)
#define PADR_PA0	bit(0)

/* PBDDR bits */
#define PBDDR_PB7DDR	bit(7)
#define PBDDR_PB6DDR	bit(6)
#define PBDDR_PB5DDR	bit(5)
#define PBDDR_PB4DDR	bit(4)
#define PBDDR_PB3DDR	bit(3)
#define PBDDR_PB2DDR	bit(2)
#define PBDDR_PB1DDR	bit(1)
#define PBDDR_PB0DDR	bit(0)

/* PBDR bits */
#define PBDR_PB7	bit(7)
#define PBDR_PB6	bit(6)
#define PBDR_PB5	bit(5)
#define PBDR_PB4	bit(4)
#define PBDR_PB3	bit(3)
#define PBDR_PB2	bit(2)
#define PBDR_PB1	bit(1)
#define PBDR_PB0	bit(0)

/* P2PCR bits */
#define P2PCR_P27PCR	bit(7)
#define P2PCR_P26PCR	bit(6)
#define P2PCR_P25PCR	bit(5)
#define P2PCR_P24PCR	bit(4)
#define P2PCR_P23PCR	bit(3)
#define P2PCR_P22PCR	bit(2)
#define P2PCR_P21PCR	bit(1)
#define P2PCR_P20PCR	bit(0)

/* P4PCR bits */
#define P4PCR_P47PCR	bit(7)
#define P4PCR_P46PCR	bit(6)
#define P4PCR_P45PCR	bit(5)
#define P4PCR_P44PCR	bit(4)
#define P4PCR_P43PCR	bit(3)
#define P4PCR_P42PCR	bit(2)
#define P4PCR_P41PCR	bit(1)
#define P4PCR_P40PCR	bit(0)

/* P5PCR bits */
#define P5PCR_P53PCR	bit(3)
#define P5PCR_P52PCR	bit(2)
#define P5PCR_P51PCR	bit(1)
#define P5PCR_P50PCR	bit(0)

#endif /* H83048_PORTS_H */
