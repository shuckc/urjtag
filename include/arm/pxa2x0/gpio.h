/*
 * $Id$
 *
 * XScale PXA250/PXA210 GPIO Registers
 * Copyright (C) 2002 ETC s.r.o.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 *
 * Written by Marcel Telka <marcel@telka.sk>, 2002.
 *
 * Documentation:
 * [1] Intel Corporation, "Intel PXA250 and PXA210 Application Processors
 *     Developer's Manual", February 2002, Order Number: 278522-001
 *
 */

#ifndef	PXA2X0_GPIO_H
#define	PXA2X0_GPIO_H

#include <common.h>

#if LANGUAGE == C
#include <stdint.h>
#endif

/* GPIO Registers */

#define	GPIO_BASE	0x40E00000

#if LANGUAGE == C
typedef volatile struct GPIO_registers {
	uint32_t gplr0;
	uint32_t gplr1;
	uint32_t gplr2;
	uint32_t gpdr0;
	uint32_t gpdr1;
	uint32_t gpdr2;
	uint32_t gpsr0;
	uint32_t gpsr1;
	uint32_t gpsr2;
	uint32_t gpcr0;
	uint32_t gpcr1;
	uint32_t gpcr2;
	uint32_t grer0;
	uint32_t grer1;
	uint32_t grer2;
	uint32_t gfer0;
	uint32_t gfer1;
	uint32_t gfer2;
	uint32_t gedr0;
	uint32_t gedr1;
	uint32_t gedr2;
	uint32_t gafr0_l;
	uint32_t gafr0_u;
	uint32_t gafr1_l;
	uint32_t gafr1_u;
	uint32_t gafr2_l;
	uint32_t gafr2_u;
} GPIO_registers;

#ifdef PXA2X0_UNMAPPED
#define	GPIO_pointer	((GPIO_registers*) GPIO_BASE)
#endif

#define	GPLR0		GPIO_pointer->gplr0
#define	GPLR1		GPIO_pointer->gplr1
#define	GPLR2		GPIO_pointer->gplr2
#define	GPDR0		GPIO_pointer->gpdr0
#define	GPDR1		GPIO_pointer->gpdr1
#define	GPDR2		GPIO_pointer->gpdr2
#define	GPSR0		GPIO_pointer->gpsr0
#define	GPSR1		GPIO_pointer->gpsr1
#define	GPSR2		GPIO_pointer->gpsr2
#define	GPCR0		GPIO_pointer->gpcr0
#define	GPCR1		GPIO_pointer->gpcr1
#define	GPCR2		GPIO_pointer->gpcr2
#define	GRER0		GPIO_pointer->grer0
#define	GRER1		GPIO_pointer->grer1
#define	GRER2		GPIO_pointer->grer2
#define	GFER0		GPIO_pointer->gfer0
#define	GFER1		GPIO_pointer->gfer1
#define	GFER2		GPIO_pointer->gfer2
#define	GEDR0		GPIO_pointer->gedr0
#define	GEDR1		GPIO_pointer->gedr1
#define	GEDR2		GPIO_pointer->gedr2
#define	GAFR0_L		GPIO_pointer->gafr0_l
#define	GAFR0_U		GPIO_pointer->gafr0_u
#define	GAFR1_L		GPIO_pointer->gafr1_l
#define	GAFR1_U		GPIO_pointer->gafr1_u
#define	GAFR2_L		GPIO_pointer->gafr2_l
#define	GAFR2_U		GPIO_pointer->gafr2_u
#endif /* LANGUAGE == C */

#define	GPLR0_OFFSET	0x00
#define	GPLR1_OFFSET	0x04
#define	GPLR2_OFFSET	0x08
#define	GPDR0_OFFSET	0x0C
#define	GPDR1_OFFSET	0x10
#define	GPDR2_OFFSET	0x14
#define	GPSR0_OFFSET	0x18
#define	GPSR1_OFFSET	0x1C
#define	GPSR2_OFFSET	0x20
#define	GPCR0_OFFSET	0x24
#define	GPCR1_OFFSET	0x28
#define	GPCR2_OFFSET	0x2C
#define	GRER0_OFFSET	0x30
#define	GRER1_OFFSET	0x34
#define	GRER2_OFFSET	0x38
#define	GFER0_OFFSET	0x3C
#define	GFER1_OFFSET	0x40
#define	GFER2_OFFSET	0x44
#define	GEDR0_OFFSET	0x48
#define	GEDR1_OFFSET	0x4C
#define	GEDR2_OFFSET	0x50
#define	GAFR0_L_OFFSET	0x54
#define	GAFR0_U_OFFSET	0x58
#define	GAFR1_L_OFFSET	0x5C
#define	GAFR1_U_OFFSET	0x60
#define	GAFR2_L_OFFSET	0x64
#define	GAFR2_U_OFFSET	0x68

/* GPIO bits */

#define	GPIO0_GP0	bit(0)
#define	GPIO0_GP1	bit(1)
#define	GPIO0_GP2	bit(2)
#define	GPIO0_GP3	bit(3)
#define	GPIO0_GP4	bit(4)
#define	GPIO0_GP5	bit(5)
#define	GPIO0_GP6	bit(6)
#define	GPIO0_GP7	bit(7)
#define	GPIO0_GP8	bit(8)
#define	GPIO0_GP9	bit(9)
#define	GPIO0_GP10	bit(10)
#define	GPIO0_GP11	bit(11)
#define	GPIO0_GP12	bit(12)
#define	GPIO0_GP13	bit(13)
#define	GPIO0_GP14	bit(14)
#define	GPIO0_GP15	bit(15)
#define	GPIO0_GP16	bit(16)
#define	GPIO0_GP17	bit(17)
#define	GPIO0_GP18	bit(18)
#define	GPIO0_GP19	bit(19)
#define	GPIO0_GP20	bit(20)
#define	GPIO0_GP21	bit(21)
#define	GPIO0_GP22	bit(22)
#define	GPIO0_GP23	bit(23)
#define	GPIO0_GP24	bit(24)
#define	GPIO0_GP25	bit(25)
#define	GPIO0_GP26	bit(26)
#define	GPIO0_GP27	bit(27)
#define	GPIO0_GP28	bit(28)
#define	GPIO0_GP29	bit(29)
#define	GPIO0_GP30	bit(30)
#define	GPIO0_GP31	bit(31)
#define	GPIO1_GP32	bit(0)
#define	GPIO1_GP33	bit(1)
#define	GPIO1_GP34	bit(2)
#define	GPIO1_GP35	bit(3)
#define	GPIO1_GP36	bit(4)
#define	GPIO1_GP37	bit(5)
#define	GPIO1_GP38	bit(6)
#define	GPIO1_GP39	bit(7)
#define	GPIO1_GP40	bit(8)
#define	GPIO1_GP41	bit(9)
#define	GPIO1_GP42	bit(10)
#define	GPIO1_GP43	bit(11)
#define	GPIO1_GP44	bit(12)
#define	GPIO1_GP45	bit(13)
#define	GPIO1_GP46	bit(14)
#define	GPIO1_GP47	bit(15)
#define	GPIO1_GP48	bit(16)
#define	GPIO1_GP49	bit(17)
#define	GPIO1_GP50	bit(18)
#define	GPIO1_GP51	bit(19)
#define	GPIO1_GP52	bit(20)
#define	GPIO1_GP53	bit(21)
#define	GPIO1_GP54	bit(22)
#define	GPIO1_GP55	bit(23)
#define	GPIO1_GP56	bit(24)
#define	GPIO1_GP57	bit(25)
#define	GPIO1_GP58	bit(26)
#define	GPIO1_GP59	bit(27)
#define	GPIO1_GP60	bit(28)
#define	GPIO1_GP61	bit(29)
#define	GPIO1_GP62	bit(30)
#define	GPIO1_GP63	bit(31)
#define	GPIO2_GP64	bit(0)
#define	GPIO2_GP65	bit(1)
#define	GPIO2_GP66	bit(2)
#define	GPIO2_GP67	bit(3)
#define	GPIO2_GP68	bit(4)
#define	GPIO2_GP69	bit(5)
#define	GPIO2_GP70	bit(6)
#define	GPIO2_GP71	bit(7)
#define	GPIO2_GP72	bit(8)
#define	GPIO2_GP73	bit(9)
#define	GPIO2_GP74	bit(10)
#define	GPIO2_GP75	bit(11)
#define	GPIO2_GP76	bit(12)
#define	GPIO2_GP77	bit(13)
#define	GPIO2_GP78	bit(14)
#define	GPIO2_GP79	bit(15)
#define	GPIO2_GP80	bit(16)

#endif /* PXA2X0_GPIO_H */
