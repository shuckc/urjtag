/*
 * $Id$
 *
 * Pod signal names
 * Copyright (C) 2008 K. Waschk
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
 */

#ifndef POD_H
#define	POD_H

typedef enum
{
    CS_NONE = 0,                // no/invalid signal
    CS_TDI = (1 << 0),          // out: JTAG/SPI data in
    CS_TCK = (1 << 1),          // out: JTAG/SPI clock
    CS_TMS = (1 << 2),          // out: JTAG test mode select/SPI slave select
    CS_TRST = (1 << 3),         // out: JTAG TAP reset
    CS_RESET = (1 << 4),        // out: system reset
    CS_SCK = (1 << 5),          // out: I2C clock (not yet used)
    CS_SDA = (1 << 6),          // inout: I2C data (not yet used)
    CS_SS = (1 << 7),           // out: SPI slave select (not yet used)
}
pod_sigsel_t;

#endif /* POD_H */
