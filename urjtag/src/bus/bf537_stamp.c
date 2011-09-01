/*
 * $Id$
 *
 * Analog Devices ADSP-BF537 STAMP/EZ-KIT Lite bus driver
 * Copyright (C) 2008-2011 Analog Devices, Inc.
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
 * Written by Jie Zhang <jie.zhang@analog.com>, 2008.
 */

#include "blackfin.h"

typedef struct
{
    bfin_bus_params_t params; /* needs to be first */
} bus_params_t;

static const bfin_bus_default_t bf537_stamp_defaults[] = {
    /* BF527 SDP board uses PG0 as ~FLASH_EN.  */
    {"bf527_sdp", "hwait=PG0"},
    {NULL},
};

static urj_bus_t *
bf537_stamp_bus_new (urj_chain_t *chain, const urj_bus_driver_t *driver,
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
    failed |= bfin_bus_new (bus, cmd_params, bf537_stamp_defaults);

    if (failed)
    {
        urj_bus_generic_free (bus);
        return NULL;
    }

    return bus;
}

BFIN_BUS_DECLARE(bf537_stamp, "BF537 Stamp board");
_BFIN_BUS_DECLARE(bf537_ezkit, bf537_stamp, "BF537 EZ-Kit board");
_BFIN_BUS_DECLARE(bf527_ezkit, bf537_stamp, "BF527 EZ-Kit board");
_BFIN_BUS_DECLARE(bf527_sdp, bf537_stamp, "BF527 SDP board");
_BFIN_BUS_DECLARE(bf538f_ezkit, bf537_stamp, "BF538F EZ-Kit board");
_BFIN_BUS_DECLARE(bf526_ezkit, bf537_stamp, "BF526 EZ-Kit board");
_BFIN_BUS_DECLARE(bf533_ezkit, bf537_stamp, "BF533 EZ-Kit board");
_BFIN_BUS_DECLARE(bf52x, bf537_stamp, "Generic BF52x");
_BFIN_BUS_DECLARE(bf53x, bf537_stamp, "Generic BF53x");
