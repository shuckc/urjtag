/*
 * $Id$
 *
 * StrongARM SA-1110 GPIO Registers
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
 * [1] Intel Corporation, "Intel StrongARM SA-1110 Microprocessor
 *     Developer's Manual", October 2001, Order Number: 278240-004
 * [2] Intel Corporation, "Intel StrongARM SA-1110 Microprocessor
 *     Specification Update", December 2001, Order Number: 278259-023
 *
 */

#ifndef	SA11X0_GPIO_H
#define	SA11X0_GPIO_H

#include <common.h>

#if LANGUAGE == C
#include <stdint.h>
#endif

/* GPIO Registers */

#define	GPIO_BASE	0x90040000

#if LANGUAGE == C
typedef volatile struct GPIO_registers {
	uint32_t gplr;
	uint32_t gpdr;
	uint32_t gpsr;
	uint32_t gpcr;
	uint32_t grer;
	uint32_t gfer;
	uint32_t gedr;
	uint32_t gafr;
} GPIO_registers;

#ifndef GPIO_pointer
#define	GPIO_pointer	((GPIO_registers*) GPIO_BASE)
#endif

#define	GPLR		GPIO_pointer->gplr
#define	GPDR		GPIO_pointer->gpdr
#define	GPSR		GPIO_pointer->gpsr
#define	GPCR		GPIO_pointer->gpcr
#define	GRER		GPIO_pointer->grer
#define	GFER		GPIO_pointer->gfer
#define	GEDR		GPIO_pointer->gedr
#define	GAFR		GPIO_pointer->gafr
#endif /* LANGUAGE == C */

#define	GPLR_OFFSET	0x00
#define	GPDR_OFFSET	0x04
#define	GPSR_OFFSET	0x08
#define	GPCR_OFFSET	0x0C
#define	GRER_OFFSET	0x10
#define	GFER_OFFSET	0x14
#define	GEDR_OFFSET	0x18
#define	GAFR_OFFSET	0x1C

#endif	/* SA11X0_GPIO_H */
