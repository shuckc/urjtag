/*
 * $Id$
 *
 * Copyright (C) 2003 BLXCPU co. Ltd.
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
 * Written by ZHANG WEI <zwblue@sohu.com>, 2003
 *
 * Documentation:
 * [1] AMD, "AMD Alchemy Solutions AU1500 Processor Data Book -
 *     Preliminary Information", June 2003, Publication ID: 30361B
 *
 */


#include <sysdep.h>

#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#include <urjtag/part.h>
#include <urjtag/bus.h>
#include <urjtag/chain.h>
#include <urjtag/bssignal.h>

#include "buses.h"
#include "generic_bus.h"

typedef struct
{
    urj_part_signal_t *rad[32];
    urj_part_signal_t *nrcs[4];
    urj_part_signal_t *nrwe;
    urj_part_signal_t *nroe;
    urj_part_signal_t *rd[32];
} bus_params_t;

#define RAD     ((bus_params_t *) bus->params)->rad
#define nRCS    ((bus_params_t *) bus->params)->nrcs
#define nRWE    ((bus_params_t *) bus->params)->nrwe
#define nROE    ((bus_params_t *) bus->params)->nroe
#define RD      ((bus_params_t *) bus->params)->rd

/**
 * bus->driver->(*new_bus)
 *
 */
static urj_bus_t *
au1500_bus_new (urj_chain_t *chain, const urj_bus_driver_t *driver,
                const urj_param_t *cmd_params[])
{
    urj_bus_t *bus;
    urj_part_t *part;
    char buff[10];
    int i;
    int failed = 0;

    bus = urj_bus_generic_new (chain, driver, sizeof (bus_params_t));
    if (bus == NULL)
        return NULL;
    part = bus->part;

    for (i = 0; i < 32; i++)
    {
        sprintf (buff, "RAD%d", i);
        failed |= urj_bus_generic_attach_sig (part, &(RAD[i]), buff);
    }

    for (i = 0; i < 4; i++)
    {
        sprintf (buff, "RCE_N%d", i);
        failed |= urj_bus_generic_attach_sig (part, &(nRCS[i]), buff);
    }


    failed |= urj_bus_generic_attach_sig (part, &(nRWE), "RWE_N");

    failed |= urj_bus_generic_attach_sig (part, &(nROE), "ROE_N");

    for (i = 0; i < 32; i++)
    {
        sprintf (buff, "RD%d", i);
        failed |= urj_bus_generic_attach_sig (part, &(RD[i]), buff);
    }

    if (failed)
    {
        free (bus->params);
        free (bus);
        return NULL;
    }

    return bus;

}

/**
 * bus->driver->(*printinfo)
 *
 */
static void
au1500_bus_printinfo (urj_log_level_t ll, urj_bus_t *bus)
{
    int i;

    for (i = 0; i < bus->chain->parts->len; i++)
        if (bus->part == bus->chain->parts->parts[i])
            break;
    urj_log (ll, _("AU1500 compatible bus driver via BSR (JTAG part No. %d)\n"),
            i);
}

/**
 * bus->driver->(*area)
 *
 */
static int
au1500_bus_area (urj_bus_t *bus, uint32_t addr, urj_bus_area_t *area)
{
    area->description = NULL;
    area->start = UINT32_C (0x00000000);
    area->length = UINT64_C (0x00100000000);
//      area->width = 16;
    area->width =
        urj_part_get_signal (bus->part,
                             urj_part_find_signal (bus->part,
                                                   "ROMSIZ")) ? 16 : 32;


    return URJ_STATUS_OK;

}

static void
setup_address (urj_bus_t *bus, uint32_t a)
{
    int i;
    urj_part_t *p = bus->part;

    for (i = 0; i < 32; i++)
        urj_part_set_signal (p, RAD[i], 1, (a >> i) & 1);
}

static void
set_data_in (urj_bus_t *bus)
{
    int i;
    urj_part_t *p = bus->part;
    urj_bus_area_t area;

    au1500_bus_area (bus, 0, &area);

    for (i = 0; i < area.width; i++)
        urj_part_set_signal_input (p, RD[i]);

}

static uint32_t
get_data_out (urj_bus_t *bus)
{
    int i;
    urj_part_t *p = bus->part;
    urj_bus_area_t area;
    uint32_t d = 0;

    au1500_bus_area (bus, 0, &area);

    for (i = 0; i < area.width; i++)
        d |= (uint32_t) (urj_part_get_signal (p, RD[i]) << i);

    return d;
}

static void
setup_data (urj_bus_t *bus, uint32_t d)
{
    int i;
    urj_part_t *p = bus->part;
    urj_bus_area_t area;

    au1500_bus_area (bus, 0, &area);

    for (i = 0; i < area.width; i++)
        urj_part_set_signal (p, RD[i], 1, (d >> i) & 1);
}

/**
 * bus->driver->(*read_start)
 *
 */
static int
au1500_bus_read_start (urj_bus_t *bus, uint32_t adr)
{
    urj_part_t *p = bus->part;
    urj_chain_t *chain = bus->chain;

    urj_part_set_signal_low (p, nRCS[0]);
    urj_part_set_signal_high (p, nRCS[1]);
    urj_part_set_signal_high (p, nRCS[2]);
    urj_part_set_signal_high (p, nRCS[3]);
    urj_part_set_signal_high (p, nRWE);
    urj_part_set_signal_low (p, nROE);

    setup_address (bus, adr);
    set_data_in (bus);

    urj_tap_chain_shift_data_registers (chain, 0);

    return URJ_STATUS_OK;
}

/**
 * bus->driver->(*read_next)
 *
 */
static uint32_t
au1500_bus_read_next (urj_bus_t *bus, uint32_t adr)
{
    urj_chain_t *chain = bus->chain;

    setup_address (bus, adr);
    urj_tap_chain_shift_data_registers (chain, 1);

    return get_data_out (bus);
}

/**
 * bus->driver->(*read_end)
 *
 */
static uint32_t
au1500_bus_read_end (urj_bus_t *bus)
{
    urj_part_t *p = bus->part;
    urj_chain_t *chain = bus->chain;

    urj_part_set_signal_high (p, nRCS[0]);
    urj_part_set_signal_high (p, nRCS[1]);
    urj_part_set_signal_high (p, nRCS[2]);
    urj_part_set_signal_high (p, nRCS[3]);
    urj_part_set_signal_high (p, nRWE);
    urj_part_set_signal_high (p, nROE);

    urj_tap_chain_shift_data_registers (chain, 1);

    return get_data_out (bus);
}

/**
 * bus->driver->(*write)
 *
 */
static void
au1500_bus_write (urj_bus_t *bus, uint32_t adr, uint32_t data)
{
    urj_part_t *p = bus->part;
    urj_chain_t *chain = bus->chain;

    urj_part_set_signal_low (p, nRCS[0]);
    urj_part_set_signal_high (p, nRCS[1]);
    urj_part_set_signal_high (p, nRCS[2]);
    urj_part_set_signal_high (p, nRCS[3]);
    urj_part_set_signal_high (p, nRWE);
    urj_part_set_signal_high (p, nROE);

    setup_address (bus, adr);
    setup_data (bus, data);

    urj_tap_chain_shift_data_registers (chain, 0);

    urj_part_set_signal_low (p, nRWE);
    urj_tap_chain_shift_data_registers (chain, 0);

    urj_part_set_signal_high (p, nRWE);
    urj_part_set_signal_high (p, nROE);
    urj_part_set_signal_high (p, nRCS[0]);
    urj_part_set_signal_high (p, nRCS[1]);
    urj_part_set_signal_high (p, nRCS[2]);
    urj_part_set_signal_high (p, nRCS[3]);

    urj_tap_chain_shift_data_registers (chain, 0);
}

const urj_bus_driver_t urj_bus_au1500_bus = {
    "au1500",
    N_("AU1500 BUS Driver via BSR"),
    au1500_bus_new,
    urj_bus_generic_free,
    au1500_bus_printinfo,
    urj_bus_generic_prepare_extest,
    au1500_bus_area,
    au1500_bus_read_start,
    au1500_bus_read_next,
    au1500_bus_read_end,
    urj_bus_generic_read,
    urj_bus_generic_write_start,
    au1500_bus_write,
    urj_bus_generic_no_init,
    urj_bus_generic_no_enable,
    urj_bus_generic_no_disable,
    URJ_BUS_TYPE_PARALLEL,
};
