/*
 * $Id$
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

#ifndef URJ_BUS_GENERIC_BUS_H
#define URJ_BUS_GENERIC_BUS_H

#include <stddef.h>

#include <urjtag/bus.h>

int urj_bus_generic_attach_sig (urj_part_t *part, urj_part_signal_t **sig,
                                const char *id);

urj_bus_t *urj_bus_generic_new (urj_chain_t *chain,
                                const urj_bus_driver_t *driver,
                                size_t param_size);
void urj_bus_generic_free (urj_bus_t *bus);
int urj_bus_generic_no_init (urj_bus_t *bus);
int urj_bus_generic_no_enable (urj_bus_t *bus);
int urj_bus_generic_no_disable (urj_bus_t *bus);
void urj_bus_generic_prepare_extest (urj_bus_t *bus);
int urj_bus_generic_write_start(urj_bus_t *bus, uint32_t adr);
uint32_t urj_bus_generic_read (urj_bus_t *bus, uint32_t adr);

#endif /* URJ_BUS_GENERIC_BUS_H */
