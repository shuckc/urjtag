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

#include <urjtag/error.h>
#include <urjtag/part.h>
#include <urjtag/chain.h>

#include "generic_bus.h"

int
urj_bus_generic_attach_sig (urj_part_t *part, urj_part_signal_t **sig,
                            const char *id)
{
    *sig = urj_part_find_signal (part, id);
    if (!*sig)
    {
        urj_error_set (URJ_ERROR_NOTFOUND, "signal '%s'", id);
        return URJ_STATUS_FAIL;
    }

    return URJ_STATUS_OK;
}

urj_bus_t *
urj_bus_generic_new (urj_chain_t *chain, const urj_bus_driver_t *driver,
                     size_t param_size)
{
    urj_bus_t *bus;

    bus = calloc (1, sizeof (urj_bus_t));
    if (bus == NULL)
    {
        urj_error_set (URJ_ERROR_OUT_OF_MEMORY, "calloc(%zd,%zd) fails",
                       (size_t) 1, sizeof (urj_bus_t));
        return NULL;
    }

    bus->driver = driver;
    bus->params = calloc (1, param_size);
    if (bus->params == NULL)
    {
        free (bus);
        urj_error_set (URJ_ERROR_OUT_OF_MEMORY, "calloc(%zd,%zd) fails",
                       (size_t) 1, param_size);
        return NULL;
    }

    bus->chain = chain;
    // @@@@ RFHH shouldn't we verify chain->active_part etc?
    bus->part = chain->parts->parts[chain->active_part];

    return bus;
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
 * bus->driver->(*enable)
 *
 */
int
urj_bus_generic_no_enable (urj_bus_t *bus)
{
    bus->enabled = 1;

    return URJ_STATUS_OK;
}

/**
 * bus->driver->(*disable)
 *
 */
int
urj_bus_generic_no_disable (urj_bus_t *bus)
{
    bus->enabled = 0;

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
 * bus->driver->(*write_start)
 *
 */
int
urj_bus_generic_write_start (urj_bus_t *bus, uint32_t adr)
{
    return 0;
}

/**
 * bus->driver->(*read)
 *
 */
uint32_t
urj_bus_generic_read (urj_bus_t *bus, uint32_t adr)
{
    // @@@@ RFHH check status
    URJ_BUS_READ_START (bus, adr);
    return URJ_BUS_READ_END (bus);
}
