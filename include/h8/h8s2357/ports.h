/*
 * $Id$
 *
 * H8S/2357 PORTS Registers
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

#ifndef H8S2357_PORTS_H
#define H8S2357_PORTS_H

#include <common.h>

#if LANGUAGE == C
#include <stdint.h>
#endif

/* PORTS registers */

#define PORT_BASE	0xffffff50
#define DDR_BASE	0xfffffeb0

#if LANGUAGE == C
typedef struct PORT_registers {
	uint8_t port1;
	uint8_t port2;
	uint8_t port3;
	uint8_t port4;
	uint8_t port5;
	uint8_t port6;
	uint8_t __reserved1[3];
	uint8_t porta;
	uint8_t portb;
	uint8_t portc;
	uint8_t portd;
	uint8_t porte;
	uint8_t portf;
	uint8_t portg;
	uint8_t p1dr;
	uint8_t p2dr;
	uint8_t p3dr;
	uint8_t __reserved2;
	uint8_t p5dr;
	uint8_t p6dr;
	uint8_t __reserved3[3];
	uint8_t padr;
	uint8_t pbdr;
	uint8_t pcdr;
	uint8_t pddr;
	uint8_t pedr;
	uint8_t pfdr;
	uint8_t pgdr;
	uint8_t papcr;
	uint8_t pbpcr;
	uint8_t pcpcr;
	uint8_t pdpcr;
	uint8_t pepcr;
	uint8_t __reserved4;
	uint8_t p3odr;
	uint8_t paodr;
} PORT_registers_t;

typedef struct DDR_registers {
	uint8_t p1ddr;
	uint8_t p2ddr;
	uint8_t p3ddr;
	uint8_t __reserved1;
	uint8_t p5ddr;
	uint8_t p6ddr;
	uint8_t __reserved2[3];
	uint8_t paddr;
	uint8_t pbddr;
	uint8_t pcddr;
	uint8_t pdddr;
	uint8_t peddr;
	uint8_t pfddr;
	uint8_t pgddr;
} DDR_registers_t;


#define PORT_pointer	((PORT_registers_t*) PORT_BASE)
#define DDR_pointer	((DDR_registers_t*) DDR_BASE)

#define PORT1		PORT_pointer->port1
#define PORT2		PORT_pointer->port2
#define PORT3		PORT_pointer->port3
#define PORT4		PORT_pointer->port4
#define PORT5		PORT_pointer->port5
#define PORT6		PORT_pointer->port6
#define ORTA		PORT_pointer->orta
#define PORTB		PORT_pointer->portb
#define PORTC		PORT_pointer->portc
#define PORTD		PORT_pointer->portd
#define PORTE		PORT_pointer->porte
#define PORTF		PORT_pointer->portf
#define PORTG		PORT_pointer->portg
#define P1DR		PORT_pointer->p1dr
#define P2DR		PORT_pointer->p2dr
#define P3DR		PORT_pointer->p3dr
#define P5DR		PORT_pointer->p5dr
#define P6DR		PORT_pointer->p6dr
#define PADR		PORT_pointer->padr
#define PBDR		PORT_pointer->pbdr
#define PCDR		PORT_pointer->pcdr
#define PDDR		PORT_pointer->pddr
#define PEDR		PORT_pointer->pedr
#define PFDR		PORT_pointer->pfdr
#define PGDR		PORT_pointer->pgdr
#define PAPCR		PORT_pointer->papcr
#define PBPCR		PORT_pointer->pbpcr
#define PCPCR		PORT_pointer->pcpcr
#define PDPCR		PORT_pointer->pdpcr
#define PEPCR		PORT_pointer->pepcr
#define P3ODR		PORT_pointer->p3odr
#define PAODR		PORT_pointer->paodr

#define P1DDR		DDR_pointer->p1ddr
#define P2DDR		DDR_pointer->p2ddr
#define P3DDR		DDR_pointer->p3ddr
#define P5DDR		DDR_pointer->p5ddr
#define P6DDR		DDR_pointer->p6ddr
#define PADDR		DDR_pointer->paddr
#define PBDDR		DDR_pointer->pbddr
#define PCDDR		DDR_pointer->pcddr
#define PDDDR		DDR_pointer->pdddr
#define PEDDR		DDR_pointer->peddr
#define PFDDR		DDR_pointer->pfddr
#define PGDDR		DDR_pointer->pgddr
#endif /* LANGUAGE == C */

#define PORT1_OFFSET	0x00
#define PORT2_OFFSET	0x01
#define PORT3_OFFSET	0x02
#define PORT4_OFFSET	0x03
#define PORT5_OFFSET	0x04
#define PORT6_OFFSET	0x05
#define ORTA_OFFSET	0x09
#define PORTB_OFFSET	0x0a
#define PORTC_OFFSET	0x0b
#define PORTD_OFFSET	0x0c
#define PORTE_OFFSET	0x0d
#define PORTF_OFFSET	0x0e
#define PORTG_OFFSET	0x0f
#define P1DR_OFFSET	0x10
#define P2DR_OFFSET	0x11
#define P3DR_OFFSET	0x12
#define P5DR_OFFSET	0x14
#define P6DR_OFFSET	0x15
#define PADR_OFFSET	0x19
#define PBDR_OFFSET	0x1a
#define PCDR_OFFSET	0x1b
#define PDDR_OFFSET	0x1c
#define PEDR_OFFSET	0x1d
#define PFDR_OFFSET	0x1e
#define PGDR_OFFSET	0x1f
#define PAPCR_OFFSET	0x20
#define PBPCR_OFFSET	0x21
#define PCPCR_OFFSET	0x22
#define PDPCR_OFFSET	0x23
#define PEPCR_OFFSET	0x24
#define P3ODR_OFFSET	0x26
#define PAODR_OFFSET	0x27

#define P1DDR_OFFSET	0x00
#define P2DDR_OFFSET	0x01
#define P3DDR_OFFSET	0x02
#define P5DDR_OFFSET	0x04
#define P6DDR_OFFSET	0x05
#define PADDR_OFFSET	0x09
#define PBDDR_OFFSET	0x0a
#define PCDDR_OFFSET	0x0b
#define PDDDR_OFFSET	0x0c
#define PEDDR_OFFSET	0x0d
#define PFDDR_OFFSET	0x0e
#define PGDDR_OFFSET	0x0f

/* PORT1 bits */
#define PORT1_P17	bit(7)
#define PORT1_P16	bit(6)
#define PORT1_P15	bit(5)
#define PORT1_P14	bit(4)
#define PORT1_P13	bit(3)
#define PORT1_P12	bit(2)
#define PORT1_P11	bit(1)
#define PORT1_P10	bit(0)

/* PORT2 bits */
#define PORT2_P27	bit(7)
#define PORT2_P26	bit(6)
#define PORT2_P25	bit(5)
#define PORT2_P24	bit(4)
#define PORT2_P23	bit(3)
#define PORT2_P22	bit(2)
#define PORT2_P21	bit(1)
#define PORT2_P20	bit(0)

/* PORT3 bits */
#define PORT3_P35	bit(5)
#define PORT3_P34	bit(4)
#define PORT3_P33	bit(3)
#define PORT3_P32	bit(2)
#define PORT3_P31	bit(1)
#define PORT3_P30	bit(0)

/* PORT4 bits */
#define PORT4_P47	bit(7)
#define PORT4_P46	bit(6)
#define PORT4_P45	bit(5)
#define PORT4_P44	bit(4)
#define PORT4_P43	bit(3)
#define PORT4_P42	bit(2)
#define PORT4_P41	bit(1)
#define PORT4_P40	bit(0)

/* PORT5 bits */
#define PORT5_P53	bit(3)
#define PORT5_P52	bit(2)
#define PORT5_P51	bit(1)
#define PORT5_P50	bit(0)

/* PORT6 bits */
#define PORT6_P67	bit(7)
#define PORT6_P66	bit(6)
#define PORT6_P65	bit(5)
#define PORT6_P64	bit(4)
#define PORT6_P63	bit(3)
#define PORT6_P62	bit(2)
#define PORT6_P61	bit(1)
#define PORT6_P60	bit(0)

/* PORTA bits */
#define PORTA_PA7	bit(7)
#define PORTA_PA6	bit(6)
#define PORTA_PA5	bit(5)
#define PORTA_PA4	bit(4)
#define PORTA_PA3	bit(3)
#define PORTA_PA2	bit(2)
#define PORTA_PA1	bit(1)
#define PORTA_PA0	bit(0)

/* PORTB bits */
#define PORTB_PB7	bit(7)
#define PORTB_PB6	bit(6)
#define PORTB_PB5	bit(5)
#define PORTB_PB4	bit(4)
#define PORTB_PB3	bit(3)
#define PORTB_PB2	bit(2)
#define PORTB_PB1	bit(1)
#define PORTB_PB0	bit(0)

/* PORTC bits */
#define PORTC_PC7	bit(7)
#define PORTC_PC6	bit(6)
#define PORTC_PC5	bit(5)
#define PORTC_PC4	bit(4)
#define PORTC_PC3	bit(3)
#define PORTC_PC2	bit(2)
#define PORTC_PC1	bit(1)
#define PORTC_PC0	bit(0)

/* PORTD bits */
#define PORTD_PD7	bit(7)
#define PORTD_PD6	bit(6)
#define PORTD_PD5	bit(5)
#define PORTD_PD4	bit(4)
#define PORTD_PD3	bit(3)
#define PORTD_PD2	bit(2)
#define PORTD_PD1	bit(1)
#define PORTD_PD0	bit(0)

/* PORTE bits */
#define PORTE_PE7	bit(7)
#define PORTE_PE6	bit(6)
#define PORTE_PE5	bit(5)
#define PORTE_PE4	bit(4)
#define PORTE_PE3	bit(3)
#define PORTE_PE2	bit(2)
#define PORTE_PE1	bit(1)
#define PORTE_PE0	bit(0)

/* PORTF bits */
#define PORTF_PF7	bit(7)
#define PORTF_PF6	bit(6)
#define PORTF_PF5	bit(5)
#define PORTF_PF4	bit(4)
#define PORTF_PF3	bit(3)
#define PORTF_PF2	bit(2)
#define PORTF_PF1	bit(1)
#define PORTF_PF0	bit(0)

/* PORTG bits */
#define PORTG_PG4	bit(4)
#define PORTG_PG3	bit(3)
#define PORTG_PG2	bit(2)
#define PORTG_PG1	bit(1)
#define PORTG_PG0	bit(0)

/* P1DR bits */
#define P1DR_P17DR	bit(7)
#define P1DR_P16DR	bit(6)
#define P1DR_P15DR	bit(5)
#define P1DR_P14DR	bit(4)
#define P1DR_P13DR	bit(3)
#define P1DR_P12DR	bit(2)
#define P1DR_P11DR	bit(1)
#define P1DR_P10DR	bit(0)

/* P2DR bits */
#define P2DR_P27DR	bit(7)
#define P2DR_P26DR	bit(6)
#define P2DR_P25DR	bit(5)
#define P2DR_P24DR	bit(4)
#define P2DR_P23DR	bit(3)
#define P2DR_P22DR	bit(2)
#define P2DR_P21DR	bit(1)
#define P2DR_P20DR	bit(0)

/* P3DR bits */
#define P3DR_P35DR	bit(5)
#define P3DR_P34DR	bit(4)
#define P3DR_P33DR	bit(3)
#define P3DR_P32DR	bit(2)
#define P3DR_P31DR	bit(1)
#define P3DR_P30DR	bit(0)

/* P5DR bits */
#define P5DR_P53DR	bit(3)
#define P5DR_P52DR	bit(2)
#define P5DR_P51DR	bit(1)
#define P5DR_P50DR	bit(0)

/* P6DR bits */
#define P6DR_P67DR	bit(7)
#define P6DR_P66DR	bit(6)
#define P6DR_P65DR	bit(5)
#define P6DR_P64DR	bit(4)
#define P6DR_P63DR	bit(3)
#define P6DR_P62DR	bit(2)
#define P6DR_P61DR	bit(1)
#define P6DR_P60DR	bit(0)

/* PADR bits */
#define PADR_PA7DR	bit(7)
#define PADR_PA6DR	bit(6)
#define PADR_PA5DR	bit(5)
#define PADR_PA4DR	bit(4)
#define PADR_PA3DR	bit(3)
#define PADR_PA2DR	bit(2)
#define PADR_PA1DR	bit(1)
#define PADR_PA0DR	bit(0)

/* PBDR bits */
#define PBDR_PB7DR	bit(7)
#define PBDR_PB6DR	bit(6)
#define PBDR_PB5DR	bit(5)
#define PBDR_PB4DR	bit(4)
#define PBDR_PB3DR	bit(3)
#define PBDR_PB2DR	bit(2)
#define PBDR_PB1DR	bit(1)
#define PBDR_PB0DR	bit(0)

/* PCDR bits */
#define PCDR_PC7DR	bit(7)
#define PCDR_PC6DR	bit(6)
#define PCDR_PC5DR	bit(5)
#define PCDR_PC4DR	bit(4)
#define PCDR_PC3DR	bit(3)
#define PCDR_PC2DR	bit(2)
#define PCDR_PC1DR	bit(1)
#define PCDR_PC0DR	bit(0)

/* PDDR bits */
#define PDDR_PD7DR	bit(7)
#define PDDR_PD6DR	bit(6)
#define PDDR_PD5DR	bit(5)
#define PDDR_PD4DR	bit(4)
#define PDDR_PD3DR	bit(3)
#define PDDR_PD2DR	bit(2)
#define PDDR_PD1DR	bit(1)
#define PDDR_PD0DR	bit(0)

/* PEDR bits */
#define PEDR_PE7DR	bit(7)
#define PEDR_PE6DR	bit(6)
#define PEDR_PE5DR	bit(5)
#define PEDR_PE4DR	bit(4)
#define PEDR_PE3DR	bit(3)
#define PEDR_PE2DR	bit(2)
#define PEDR_PE1DR	bit(1)
#define PEDR_PE0DR	bit(0)

/* PFDR bits */
#define PFDR_PF7DR	bit(7)
#define PFDR_PF6DR	bit(6)
#define PFDR_PF5DR	bit(5)
#define PFDR_PF4DR	bit(4)
#define PFDR_PF3DR	bit(3)
#define PFDR_PF2DR	bit(2)
#define PFDR_PF1DR	bit(1)
#define PFDR_PF0DR	bit(0)

/* PGDR bits */
#define PGDR_PG4DR	bit(4)
#define PGDR_PG3DR	bit(3)
#define PGDR_PG2DR	bit(2)
#define PGDR_PG1DR	bit(1)
#define PGDR_PG0DR	bit(0)

/* PAPCR bits */
#define PAPCR_PA7PCR	bit(7)
#define PAPCR_PA6PCR	bit(6)
#define PAPCR_PA5PCR	bit(5)
#define PAPCR_PA4PCR	bit(4)
#define PAPCR_PA3PCR	bit(3)
#define PAPCR_PA2PCR	bit(2)
#define PAPCR_PA1PCR	bit(1)
#define PAPCR_PA0PCR	bit(0)

/* PBPCR bits */
#define PBPCR_PB7PCR	bit(7)
#define PBPCR_PB6PCR	bit(6)
#define PBPCR_PB5PCR	bit(5)
#define PBPCR_PB4PCR	bit(4)
#define PBPCR_PB3PCR	bit(3)
#define PBPCR_PB2PCR	bit(2)
#define PBPCR_PB1PCR	bit(1)
#define PBPCR_PB0PCR	bit(0)

/* PCPCR bits */
#define PCPCR_PC7PCR	bit(7)
#define PCPCR_PC6PCR	bit(6)
#define PCPCR_PC5PCR	bit(5)
#define PCPCR_PC4PCR	bit(4)
#define PCPCR_PC3PCR	bit(3)
#define PCPCR_PC2PCR	bit(2)
#define PCPCR_PC1PCR	bit(1)
#define PCPCR_PC0PCR	bit(0)

/* PDPCR bits */
#define PDPCR_PD7PCR	bit(7)
#define PDPCR_PD6PCR	bit(6)
#define PDPCR_PD5PCR	bit(5)
#define PDPCR_PD4PCR	bit(4)
#define PDPCR_PD3PCR	bit(3)
#define PDPCR_PD2PCR	bit(2)
#define PDPCR_PD1PCR	bit(1)
#define PDPCR_PD0PCR	bit(0)

/* PEPCR bits */
#define PEPCR_PE7PCR	bit(7)
#define PEPCR_PE6PCR	bit(6)
#define PEPCR_PE5PCR	bit(5)
#define PEPCR_PE4PCR	bit(4)
#define PEPCR_PE3PCR	bit(3)
#define PEPCR_PE2PCR	bit(2)
#define PEPCR_PE1PCR	bit(1)
#define PEPCR_PE0PCR	bit(0)

/* P3ODR bits */
#define P3ODR_P35ODR	bit(5)
#define P3ODR_P34ODR	bit(4)
#define P3ODR_P33ODR	bit(3)
#define P3ODR_P32ODR	bit(2)
#define P3ODR_P31ODR	bit(1)
#define P3ODR_P30ODR	bit(0)

/* PAODR bits */
#define PAODR_PA7ODR	bit(7)
#define PAODR_PA6ODR	bit(6)
#define PAODR_PA5ODR	bit(5)
#define PAODR_PA4ODR	bit(4)
#define PAODR_PA3ODR	bit(3)
#define PAODR_PA2ODR	bit(2)
#define PAODR_PA1ODR	bit(1)
#define PAODR_PA0ODR	bit(0)

/* P1DDR bits */
#define P1DDR_P17DDR	bit(7)
#define P1DDR_P16DDR	bit(6)
#define P1DDR_P15DDR	bit(5)
#define P1DDR_P14DDR	bit(4)
#define P1DDR_P13DDR	bit(3)
#define P1DDR_P12DDR	bit(2)
#define P1DDR_P11DDR	bit(1)
#define P1DDR_P10DDR	bit(0)

/* P2DDR bits */
#define P2DDR_P27DDR	bit(7)
#define P2DDR_P26DDR	bit(6)
#define P2DDR_P25DDR	bit(5)
#define P2DDR_P24DDR	bit(4)
#define P2DDR_P23DDR	bit(3)
#define P2DDR_P22DDR	bit(2)
#define P2DDR_P21DDR	bit(1)
#define P2DDR_P20DDR	bit(0)

/* P3DDR bits */
#define P3DDR_P35DDR	bit(5)
#define P3DDR_P34DDR	bit(4)
#define P3DDR_P33DDR	bit(3)
#define P3DDR_P32DDR	bit(2)
#define P3DDR_P31DDR	bit(1)
#define P3DDR_P30DDR	bit(0)

/* P5DDR bits */
#define P5DDR_P53DDR	bit(3)
#define P5DDR_P52DDR	bit(2)
#define P5DDR_P51DDR	bit(1)
#define P5DDR_P50DDR	bit(0)

/* P6DDR bits */
#define P6DDR_P67DDR	bit(7)
#define P6DDR_P66DDR	bit(6)
#define P6DDR_P65DDR	bit(5)
#define P6DDR_P64DDR	bit(4)
#define P6DDR_P63DDR	bit(3)
#define P6DDR_P62DDR	bit(2)
#define P6DDR_P61DDR	bit(1)
#define P6DDR_P60DDR	bit(0)

/* PADDR bits */
#define PADDR_PA7DDR	bit(7)
#define PADDR_PA6DDR	bit(6)
#define PADDR_PA5DDR	bit(5)
#define PADDR_PA4DDR	bit(4)
#define PADDR_PA3DDR	bit(3)
#define PADDR_PA2DDR	bit(2)
#define PADDR_PA1DDR	bit(1)
#define PADDR_PA0DDR	bit(0)

/* PBDDR bits */
#define PBDDR_PB7DDR	bit(7)
#define PBDDR_PB6DDR	bit(6)
#define PBDDR_PB5DDR	bit(5)
#define PBDDR_PB4DDR	bit(4)
#define PBDDR_PB3DDR	bit(3)
#define PBDDR_PB2DDR	bit(2)
#define PBDDR_PB1DDR	bit(1)
#define PBDDR_PB0DDR	bit(0)

/* PCDDR bits */
#define PCDDR_PC7DDR	bit(7)
#define PCDDR_PC6DDR	bit(6)
#define PCDDR_PC5DDR	bit(5)
#define PCDDR_PC4DDR	bit(4)
#define PCDDR_PC3DDR	bit(3)
#define PCDDR_PC2DDR	bit(2)
#define PCDDR_PC1DDR	bit(1)
#define PCDDR_PC0DDR	bit(0)

/* PDDDR bits */
#define PDDDR_PD7DDR	bit(7)
#define PDDDR_PD6DDR	bit(6)
#define PDDDR_PD5DDR	bit(5)
#define PDDDR_PD4DDR	bit(4)
#define PDDDR_PD3DDR	bit(3)
#define PDDDR_PD2DDR	bit(2)
#define PDDDR_PD1DDR	bit(1)
#define PDDDR_PD0DDR	bit(0)

/* PEDDR bits */
#define PEDDR_PE7DDR	bit(7)
#define PEDDR_PE6DDR	bit(6)
#define PEDDR_PE5DDR	bit(5)
#define PEDDR_PE4DDR	bit(4)
#define PEDDR_PE3DDR	bit(3)
#define PEDDR_PE2DDR	bit(2)
#define PEDDR_PE1DDR	bit(1)
#define PEDDR_PE0DDR	bit(0)

/* PFDDR bits */
#define PFDDR_PF7DDR	bit(7)
#define PFDDR_PF6DDR	bit(6)
#define PFDDR_PF5DDR	bit(5)
#define PFDDR_PF4DDR	bit(4)
#define PFDDR_PF3DDR	bit(3)
#define PFDDR_PF2DDR	bit(2)
#define PFDDR_PF1DDR	bit(1)
#define PFDDR_PF0DDR	bit(0)

/* PGDDR bits */
#define PGDDR_PG4DDR	bit(4)
#define PGDDR_PG3DDR	bit(3)
#define PGDDR_PG2DDR	bit(2)
#define PGDDR_PG1DDR	bit(1)
#define PGDDR_PG0DDR	bit(0)

#endif /* H8S2357_PORTS_H */
