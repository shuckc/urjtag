/*
 * $Id$
 *
 * XScale PXA250/PXA210 I2S Registers
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
 * [2] Intel Corporation, "Intel PXA250 and PXA210 Application Processors
 *     Specification Update", April 2002, Order Number: 278534-004
 *
 */

#ifndef	PXA2X0_I2S_H
#define	PXA2X0_I2S_H

#ifndef uint32_t
typedef	unsigned int	uint32_t;
#endif

/* I2S Registers */

#define	I2S_BASE	0x40400000

typedef volatile struct I2S_registers {
	uint32_t sacr0;
	uint32_t sacr1;
	uint32_t __reserved1;
	uint32_t sasr0;
	uint32_t __reserved2;
	uint32_t saimr;
	uint32_t saicr;
	uint32_t __reserved3[17];
	uint32_t sadiv;
	uint32_t __reserved4[7];
	uint32_t sadr;
} I2S_registers;

#ifndef I2S_pointer
#define	I2S_pointer	((I2S_registers*) I2S_BASE)
#endif

#define	SACR0		I2S_pointer->sacr0
#define	SACR1		I2S_pointer->sacr1
#define	SASR0		I2S_pointer->sasr0
#define	SAIMR		I2S_pointer->saimr
#define	SAICR		I2S_pointer->saicr
#define	SADIV		I2S_pointer->sadiv
#define	SADR		I2S_pointer->sadr

#endif	/* PXA2X0_I2S_H */
