/*
 * $Id$
 *
 * XScale PXA250/PXA210 I2C Registers
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

#ifndef	PXA2X0_I2C_H
#define	PXA2X0_I2C_H

#ifndef uint32_t
typedef	unsigned int	uint32_t;
#endif

/* I2C Registers */

#define	I2C_BASE	0x40300000

typedef volatile struct I2C_registers {
	uint32_t __reserved1[1440];
	uint32_t ibmr;
	uint32_t __reserved2;
	uint32_t idbr;
	uint32_t __reserved3;
	uint32_t icr;
	uint32_t __reserved4;
	uint32_t isr;
	uint32_t __reserved5;
	uint32_t isar;
} I2C_registers;

#ifndef I2C_pointer
#define	I2C_pointer	((I2C_registers*) I2C_BASE)
#endif

#define	IBMR		I2C_pointer->ibmr
#define	IDBR		I2C_pointer->idbr
#define	ICR		I2C_pointer->icr
#define	ISR		I2C_pointer->isr
#define	ISAR		I2C_pointer->isar

#endif	/* PCA2X0_I2C_H */
