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

#include <sysdep.h>

#include <stdlib.h>

#include <urjtag/part.h>
#include <urjtag/chain.h>

#include "generic_bus.h"

int
urj_bus_generic_attach_sig (urj_part_t *part, urj_part_signal_t **sig,
                            char *id)
{
    int failed = URJ_STATUS_OK;

    *sig = urj_part_find_signal (part, id);
    if (!*sig)
    {
        printf (_("signal '%s' not found\n"), id);
        failed = URJ_STATUS_FAIL;
    }

    return failed;
}

/**
 * bus->driver->(*free_bus)
 *
 */
void
urj_bus_generic_free (urj_bus_t *bus)
{
    free (bus->params);
    free (bus);
}

/**
 * bus->driver->(*init)
 *
 */
int
urj_bus_generic_no_init (urj_bus_t *bus)
{
    bus->initialized = 1;

    return URJ_STATUS_OK;
}

/**
 * bus->driver->(*prepare)
 *
 */
void
urj_bus_generic_prepare_extest (urj_bus_t *bus)
{
    if (!bus->initialized)
        URJ_BUS_INIT (bus);

    urj_part_set_instruction (bus->part, "EXTEST");
    urj_tap_chain_shift_instructions (bus->chain);
}

/**
 * bus->driver->(*read)
 *
 */
uint32_t
urj_bus_generic_read (urj_bus_t *bus, uint32_t adr)
{
    URJ_BUS_READ_START (bus, adr);
    return URJ_BUS_READ_END (bus);
}
