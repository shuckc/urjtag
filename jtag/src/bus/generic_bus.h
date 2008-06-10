/*
 * $Id: generic_bus.h $
 *
 * Generic bus driver utility functions
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
 * Written by H Hartley Sweeten <hsweeten@visionengravers.com>, 2008.
 *
 */

#ifndef GENERIC_BUS_H
#define GENERIC_BUS_H

#include "bus.h"

int generic_bus_attach_sig( part_t *part, signal_t **sig, char *id );

void generic_bus_free( bus_t *bus );
uint32_t generic_bus_read( bus_t *bus, uint32_t adr );

#endif /* GENERIC_BUS_H */
