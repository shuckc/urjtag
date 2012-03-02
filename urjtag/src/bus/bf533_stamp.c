/*
 * $Id$
 *
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
 * Written by Christian Pellegrin <chri@ascensit.com>, 2003.
 * Modified by Marcel Telka <marcel@telka.sk>, 2003.
 */

#include "blackfin.h"

typedef struct
{
    bfin_bus_params_t params; /* needs to be first */
    urj_part_signal_t *pf[2];
} bus_params_t;

#define PF      ((bus_params_t *) bus->params)->pf

static void
bf533_stamp_unselect_flash (urj_bus_t *bus)
{
    urj_part_t *part = bus->part;

    urj_part_set_signal_low (part, PF[0]);
    urj_part_set_signal_low (part, PF[1]);
}

static void
bf533_stamp_select_flash (urj_bus_t *bus, uint32_t addr)
{
    bf533_stamp_unselect_flash (bus);
}

static urj_bus_t *
bf533_stamp_bus_new (urj_chain_t *chain, const urj_bus_driver_t *driver,
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
    params->async_size = 4 * 1024 * 1024;
    params->ams_cnt = 4;
    params->abe_cnt = 2;
    params->addr_cnt = 19;
    params->data_cnt = 16;
    params->select_flash = bf533_stamp_select_flash;
    params->unselect_flash = bf533_stamp_unselect_flash;
    params->sdram = 1;
    failed |= bfin_bus_new (bus, cmd_params, NULL);

    failed |= urj_bus_generic_attach_sig (part, &PF[0], "PF0");
    failed |= urj_bus_generic_attach_sig (part, &PF[1], "PF1");

    if (failed)
    {
        urj_bus_generic_free (bus);
        return NULL;
    }

    return bus;
}

BFIN_BUS_DECLARE(bf533_stamp, "BF533 Stamp board");
