/*
 * $Id$
 *
 * Copyright (C) 2003 ETC s.r.o.
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
 * Written by Marcel Telka <marcel@telka.sk>, 2003.
 *
 */

#include <sysdep.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <urjtag/error.h>
#include <urjtag/bus.h>
#include <urjtag/chain.h>
#include <urjtag/part.h>
#include <urjtag/cmd.h>

#include "buses.h"

const urj_bus_driver_t * const urj_bus_drivers[] = {
#define _URJ_BUS(bus) &urj_bus_##bus##_bus,
#include "buses_list.h"
    NULL                        /* last must be NULL */
};

urj_bus_t *urj_bus = NULL;
urj_buses_t urj_buses = { 0, NULL };

void
urj_bus_buses_free (void)
{
    int i;

    for (i = 0; i < urj_buses.len; i++)
        URJ_BUS_FREE (urj_buses.buses[i]);

    free (urj_buses.buses);
    urj_buses.len = 0;
    urj_buses.buses = NULL;
    urj_bus = NULL;
}

int
urj_bus_buses_add (urj_bus_t *abus)
{
    urj_bus_t **b;

    if (abus == NULL)
    {
        urj_error_set (URJ_ERROR_INVALID, "abus == NULL");
        return URJ_STATUS_FAIL;
    }

    b = realloc (urj_buses.buses, (urj_buses.len + 1) * sizeof (urj_bus_t *));
    if (b == NULL)
    {
        urj_error_set (URJ_ERROR_OUT_OF_MEMORY, _("realloc(%s,%zd) fails"),
                       "urj_buses.buses",
                       (urj_buses.len + 1) * sizeof (urj_bus_t *));
        return URJ_STATUS_FAIL;
    }
    urj_buses.buses = b;
    urj_buses.buses[urj_buses.len++] = abus;
    if (urj_bus == NULL)
        urj_bus = abus;

    return URJ_STATUS_OK;
}

int
urj_bus_buses_delete (urj_bus_t *abus)
{
    int i;
    urj_bus_t **b;

    for (i = 0; i < urj_buses.len; i++)
        if (abus == urj_buses.buses[i])
            break;
    if (i >= urj_buses.len)
    {
        urj_error_set (URJ_ERROR_NOTFOUND, "abus not in global bus list");
        return URJ_STATUS_FAIL;
    }

    while (i + 1 < urj_buses.len)
    {
        urj_buses.buses[i] = urj_buses.buses[i + 1];
        i++;
    }
    urj_buses.len--;
    b = realloc (urj_buses.buses, urj_buses.len * sizeof (urj_bus_t *));
    if (b == NULL && urj_buses.len > 0)
    {
        urj_error_set (URJ_ERROR_OUT_OF_MEMORY, _("realloc(%s,%zd) fails"),
                       "urj_buses.buses", urj_buses.len * sizeof (urj_bus_t *));
        return URJ_STATUS_FAIL;
    }
    urj_buses.buses = b;

    if (urj_bus == abus)
    {
        if (urj_buses.len > 0)
            urj_bus = urj_buses.buses[0];
        // @@@@ RFHH else: urj_bus is a dangling pointer?
        else
            urj_bus = NULL;
    }

    return URJ_STATUS_OK;
}

int
urj_bus_buses_set (int n)
{
    if (n >= urj_buses.len)
    {
        urj_error_set(URJ_ERROR_INVALID, _("invalid bus number"));
        return URJ_STATUS_FAIL;
    }

    urj_bus = urj_buses.buses[n];

    return URJ_STATUS_OK;
}

int
urj_bus_init (urj_chain_t *chain, const char *drivername, char *params[])
{
    int ret;
    size_t i;
    const urj_param_t **bus_params;
    const urj_bus_driver_t *bus_driver;

    if (urj_cmd_test_cable (chain) != URJ_STATUS_OK)
        return URJ_STATUS_FAIL;

    if (urj_tap_chain_active_part (chain) == NULL)
        return URJ_STATUS_FAIL;

    for (i = 0; urj_bus_drivers[i] != NULL; ++i)
        if (strcasecmp (urj_bus_drivers[i]->name, drivername) == 0)
            break;

    bus_driver = urj_bus_drivers[i];
    if (bus_driver == NULL)
    {
        urj_error_set (URJ_ERROR_NOTFOUND, "Unknown bus: %s", drivername);
        return URJ_STATUS_FAIL;
    }

    ret = urj_param_init_list (&bus_params, params, &urj_bus_param_list);
    if (ret != URJ_STATUS_OK)
        return ret;

    if (urj_bus_init_bus (chain, bus_driver, bus_params) == NULL)
        ret = URJ_STATUS_FAIL;
    else
        ret = URJ_STATUS_OK;

    urj_param_clear (&bus_params);
    return ret;
}

urj_bus_t *
urj_bus_init_bus (urj_chain_t *chain, const urj_bus_driver_t *bus_driver,
                  const urj_param_t *param[])
{
    urj_bus_t *abus;
    int i;

    if (urj_tap_chain_active_part (chain) == NULL)
        return NULL;

    abus = bus_driver->new_bus (chain, bus_driver, param);
    if (abus == NULL)
        // retain error state
        return NULL;

    if (urj_bus_buses_add (abus) != URJ_STATUS_OK)
    {
        // @@@@ RFHH I added this FREE() + bail out. Is that correct?
        URJ_BUS_FREE(abus);
        return NULL;
    }

    if (URJ_BUS_INIT (abus) != URJ_STATUS_OK)
    {
        // @@@@ RFHH I added this FREE() + bail out. Is that correct?
        URJ_BUS_FREE(abus);
        return NULL;
    }

    for (i = 0; i < urj_buses.len; i++)
        if (urj_buses.buses[i] == urj_bus)
            break;
    if (i != urj_buses.len - 1)
        urj_log (URJ_LOG_LEVEL_NORMAL, _("Initialized bus %d, active bus %d\n"),
                 urj_buses.len - 1, i);

    return abus;
}

static const urj_param_descr_t bus_param[] =
{
    { URJ_BUS_PARAM_KEY_MUX,        URJ_PARAM_TYPE_BOOL,    "MUX", },
    { URJ_BUS_PARAM_KEY_OCD,        URJ_PARAM_TYPE_BOOL,    "OCD", },
    { URJ_BUS_PARAM_KEY_HSBC,       URJ_PARAM_TYPE_BOOL,    "HSBC", },
    { URJ_BUS_PARAM_KEY_HSBU,       URJ_PARAM_TYPE_BOOL,    "HSBU", },
    { URJ_BUS_PARAM_KEY_X8,         URJ_PARAM_TYPE_BOOL,    "X8", },
    { URJ_BUS_PARAM_KEY_X16,        URJ_PARAM_TYPE_BOOL,    "X16", },
    { URJ_BUS_PARAM_KEY_X32,        URJ_PARAM_TYPE_BOOL,    "X32", },
    { URJ_BUS_PARAM_KEY_WIDTH,      URJ_PARAM_TYPE_LU,      "WIDTH", },
    { URJ_BUS_PARAM_KEY_AMODE,      URJ_PARAM_TYPE_LU,      "AMODE", },
    { URJ_BUS_PARAM_KEY_OPCODE,     URJ_PARAM_TYPE_STRING,  "OPCODE", },
    { URJ_BUS_PARAM_KEY_LEN,        URJ_PARAM_TYPE_LU,      "LEN", },
    { URJ_BUS_PARAM_KEY_ALSB,       URJ_PARAM_TYPE_STRING,  "ALSB", },
    { URJ_BUS_PARAM_KEY_AMSB,       URJ_PARAM_TYPE_STRING,  "AMSB", },
    { URJ_BUS_PARAM_KEY_DLSB,       URJ_PARAM_TYPE_STRING,  "DLSB", },
    { URJ_BUS_PARAM_KEY_DMSB,       URJ_PARAM_TYPE_STRING,  "DMSB", },
    { URJ_BUS_PARAM_KEY_CS,         URJ_PARAM_TYPE_STRING,  "CS", },
    { URJ_BUS_PARAM_KEY_NCS,        URJ_PARAM_TYPE_STRING,  "NCS", },
    { URJ_BUS_PARAM_KEY_OE,         URJ_PARAM_TYPE_STRING,  "OE", },
    { URJ_BUS_PARAM_KEY_NOE,        URJ_PARAM_TYPE_STRING,  "NOE", },
    { URJ_BUS_PARAM_KEY_WE,         URJ_PARAM_TYPE_STRING,  "WE", },
    { URJ_BUS_PARAM_KEY_NWE,        URJ_PARAM_TYPE_STRING,  "NWE", },
    { URJ_BUS_PARAM_KEY_WP,         URJ_PARAM_TYPE_STRING,  "WP", },
    { URJ_BUS_PARAM_KEY_NWP,        URJ_PARAM_TYPE_STRING,  "NWP", },
    { URJ_BUS_PARAM_KEY_REVBITS,    URJ_PARAM_TYPE_BOOL,    "REVBITS", },
    { URJ_BUS_PARAM_KEY_HELP,       URJ_PARAM_TYPE_BOOL,    "HELP", },
    { URJ_BUS_PARAM_KEY_DBGaDDR,    URJ_PARAM_TYPE_BOOL,    "DBGaDDR", },
    { URJ_BUS_PARAM_KEY_DBGdATA,    URJ_PARAM_TYPE_BOOL,    "DBGdATA", },
    { URJ_BUS_PARAM_KEY_HWAIT,      URJ_PARAM_TYPE_STRING,  "HWAIT", },
};

const urj_param_list_t urj_bus_param_list =
{
    .list = bus_param,
    .n    = ARRAY_SIZE (bus_param)
};
