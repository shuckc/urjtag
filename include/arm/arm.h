/*
 * $Id$
 *
 * ARM specific declarations
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
 * [1] ARM Limited, "ARM Architecture Reference Manual", June 2000,
 *     Order Number: ARM DDI 0100E
 *
 */

#ifndef	ARM_H
#define	ARM_H

#include <common.h>

/* PSR bits - see 2.5 in [1] */

#define	PSR_N		bit(31)
#define	PSR_Z		bit(30)
#define	PSR_C		bit(29)
#define	PSR_V		bit(28)
#define	PSR_Q		bit(27)		/* E variants of the ARMV5 and above - see 2.5.1 in [1] */

#define	PSR_I		bit(7)
#define	PSR_F		bit(6)
#define	PSR_T		bit(5)
#define	PSR_MODE_MASK	0x0000001F
#define	PSR_MODE(x)	(x & PSR_MODE_MASK)

#define	PSR_MODE_USR	PSR_MODE(0x10)
#define	PSR_MODE_FIQ	PSR_MODE(0x11)
#define	PSR_MODE_IRQ	PSR_MODE(0x12)
#define	PSR_MODE_SVC	PSR_MODE(0x13)
#define	PSR_MODE_ABT	PSR_MODE(0x17)
#define	PSR_MODE_UND	PSR_MODE(0x1B)
#define	PSR_MODE_SYS	PSR_MODE(0x1F)	/* ARMV4 and above */

#endif /* ARM_H */
