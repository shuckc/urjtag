/*
 * bf518_ezbrd.c
 *
 * Analog Devices ADSP-BF518F EZ-BRD bus driver
 * Copyright (C) 2009-2011 Analog Devices, Inc.
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
 * Written by Jie Zhang <jie.zhang@analog.com>, 2009.
 */

#include "blackfin.h"

typedef struct
{
    bfin_bus_params_t params; /* needs to be first */
} bus_params_t;

static urj_bus_t *
bf518f_ezbrd_bus_new (urj_chain_t *chain, const urj_bus_driver_t *driver,
                      const urj_param_t *cmd_params[])
{
    urj_bus_t *bus;
    bfin_bus_params_t *params;
    int failed = 0;

    bus = urj_bus_generic_new (chain, driver, sizeof (bus_params_t));
    if (bus == NULL)
        return NULL;

    params = bus->params;
    params->async_size = 4 * 1024 * 1024;
    params->ams_cnt = 4;
    params->abe_cnt = 2;
    params->addr_cnt = 19;
    params->data_cnt = 16;
    params->sdram = 1;
    failed |= bfin_bus_new (bus, cmd_params, NULL);

    if (failed)
    {
        urj_bus_generic_free (bus);
        return NULL;
    }

    return bus;
}

BFIN_BUS_DECLARE(bf518f_ezbrd, "BF518F EZ-Board");
_BFIN_BUS_DECLARE(bf51x, bf518f_ezbrd, "Generic BF51x");
