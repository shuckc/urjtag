/*
 * $Id$
 *
 * Analog Devices ADSP-BF548 EZ-KIT Lite bus driver
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
    urj_part_signal_t *dcs0;  /* DDR */
    urj_part_signal_t *nce;   /* NAND */
} bus_params_t;

#define DCS0    ((bus_params_t *) bus->params)->dcs0
#define NCE     ((bus_params_t *) bus->params)->nce

static void
bf548_ezkit_unselect_flash (urj_bus_t *bus)
{
    urj_part_t *part = bus->part;

    urj_part_set_signal_high (part, DCS0);
    urj_part_set_signal_high (part, NCE);
}

static void
bf548_ezkit_select_flash (urj_bus_t *bus, uint32_t addr)
{
    bf548_ezkit_unselect_flash (bus);
}

static urj_bus_t *
bf548_ezkit_bus_new (urj_chain_t *chain, const urj_bus_driver_t *driver,
                     const urj_param_t *cmd_params[])
{
    urj_bus_t *bus;
    urj_part_t *part;
    bfin_bus_params_t *params;
    int failed = 0;

    bus = urj_bus_generic_new (chain, driver, sizeof (bus_params_t));
    if (bus == NULL)
        return NULL;
    part = bus->part;

    params = bus->params;
    params->async_size = 64 * 1024 * 1024;
    params->ams_cnt = 4;
    params->abe_cnt = 2;
    params->addr_cnt = 24;
    params->data_cnt = 16;
    params->select_flash = bf548_ezkit_select_flash;
    params->unselect_flash = bf548_ezkit_unselect_flash;
    failed |= bfin_bus_new (bus, cmd_params, NULL);

    failed |= urj_bus_generic_attach_sig (part, &DCS0, "CS0_B");
    failed |= urj_bus_generic_attach_sig (part, &NCE, "PJ1");

    if (failed)
    {
        urj_bus_generic_free (bus);
        return NULL;
    }

    return bus;
}

BFIN_BUS_DECLARE(bf548_ezkit, "BF548 EZ-KIT board");
