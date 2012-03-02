/*
 * $Id$
 *
 * Copyright (C) 2002, 2003 ETC s.r.o.
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
 * Written by Marcel Telka <marcel@telka.sk>, 2002, 2003.
 *
 * Documentation:
 * [1] Intel Corporation, "Intel StrongARM SA-1110 Microprocessor
 *     Developer's Manual", October 2001, Order Number: 278240-004
 *
 */

#include <sysdep.h>

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <urjtag/part.h>
#include <urjtag/bus.h>
#include <urjtag/chain.h>
#include <urjtag/bssignal.h>

#include "buses.h"
#include "generic_bus.h"

typedef struct
{
    urj_part_signal_t *a[26];
    urj_part_signal_t *d[32];
    urj_part_signal_t *ncs[6];
    urj_part_signal_t *rd_nwr;
    urj_part_signal_t *nwe;
    urj_part_signal_t *noe;
} bus_params_t;

#define A       ((bus_params_t *) bus->params)->a
#define D       ((bus_params_t *) bus->params)->d
#define nCS     ((bus_params_t *) bus->params)->ncs
#define RD_nWR  ((bus_params_t *) bus->params)->rd_nwr
#define nWE     ((bus_params_t *) bus->params)->nwe
#define nOE     ((bus_params_t *) bus->params)->noe

/**
 * bus->driver->(*new_bus)
 *
 */
static urj_bus_t *
sa1110_bus_new (urj_chain_t *chain, const urj_bus_driver_t *driver,
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

    for (i = 0; i < 26; i++)
    {
        sprintf (buff, "A%d", i);
        failed |= urj_bus_generic_attach_sig (part, &(A[i]), buff);
    }

    for (i = 0; i < 32; i++)
    {
        sprintf (buff, "D%d", i);
        failed |= urj_bus_generic_attach_sig (part, &(D[i]), buff);
    }

    for (i = 0; i < 6; i++)
    {
        sprintf (buff, "nCS%d", i);
        failed |= urj_bus_generic_attach_sig (part, &(nCS[i]), buff);
    }

    failed |= urj_bus_generic_attach_sig (part, &(RD_nWR), "RD_nWR");

    failed |= urj_bus_generic_attach_sig (part, &(nWE), "nWE");

    failed |= urj_bus_generic_attach_sig (part, &(nOE), "nOE");

    if (failed)
    {
        urj_bus_generic_free (bus);
        return NULL;
    }

    return bus;
}

/**
 * bus->driver->(*printinfo)
 *
 */
static void
sa1110_bus_printinfo (urj_log_level_t ll, urj_bus_t *bus)
{
    int i;

    for (i = 0; i < bus->chain->parts->len; i++)
        if (bus->part == bus->chain->parts->parts[i])
            break;
    urj_log (ll, _("Intel SA-1110 compatible bus driver via BSR (JTAG part No. %d)\n"),
            i);
}

/**
 * bus->driver->(*area)
 *
 */
static int
sa1110_bus_area (urj_bus_t *bus, uint32_t adr, urj_bus_area_t *area)
{
    area->description = NULL;
    area->start = UINT32_C (0x00000000);
    area->length = UINT64_C (0x100000000);
    area->width =
        urj_part_get_signal (bus->part,
                             urj_part_find_signal (bus->part,
                                                   "ROM_SEL")) ? 32 : 16;

    return URJ_STATUS_OK;
}

static void
setup_address (urj_bus_t *bus, uint32_t a)
{
    int i;
    urj_part_t *p = bus->part;

    for (i = 0; i < 26; i++)
        urj_part_set_signal (p, A[i], 1, (a >> i) & 1);
}

static void
set_data_in (urj_bus_t *bus)
{
    int i;
    urj_part_t *p = bus->part;
    urj_bus_area_t area;

    sa1110_bus_area (bus, 0, &area);

    for (i = 0; i < area.width; i++)
        urj_part_set_signal_input (p, D[i]);
}

static void
setup_data (urj_bus_t *bus, uint32_t d)
{
    int i;
    urj_part_t *p = bus->part;
    urj_bus_area_t area;

    sa1110_bus_area (bus, 0, &area);

    for (i = 0; i < area.width; i++)
        urj_part_set_signal (p, D[i], 1, (d >> i) & 1);
}

/**
 * bus->driver->(*read_start)
 *
 */
static int
sa1110_bus_read_start (urj_bus_t *bus, uint32_t adr)
{
    /* see Figure 10-12 in [1] */
    urj_part_t *p = bus->part;
    urj_chain_t *chain = bus->chain;

    urj_part_set_signal (p, nCS[0], 1, (adr >> 27) != 0);
    urj_part_set_signal (p, nCS[1], 1, (adr >> 27) != 1);
    urj_part_set_signal (p, nCS[2], 1, (adr >> 27) != 2);
    urj_part_set_signal (p, nCS[3], 1, (adr >> 27) != 3);
    urj_part_set_signal (p, nCS[4], 1, (adr >> 27) != 8);
    urj_part_set_signal (p, nCS[5], 1, (adr >> 27) != 9);
    urj_part_set_signal_high (p, RD_nWR);
    urj_part_set_signal_high (p, nWE);
    urj_part_set_signal_low (p, nOE);

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
sa1110_bus_read_next (urj_bus_t *bus, uint32_t adr)
{
    /* see Figure 10-12 in [1] */
    urj_part_t *p = bus->part;
    urj_chain_t *chain = bus->chain;
    int i;
    uint32_t d = 0;
    urj_bus_area_t area;

    sa1110_bus_area (bus, adr, &area);

    setup_address (bus, adr);
    urj_tap_chain_shift_data_registers (chain, 1);

    for (i = 0; i < area.width; i++)
        d |= (uint32_t) (urj_part_get_signal (p, D[i]) << i);

    return d;
}

/**
 * bus->driver->(*read_end)
 *
 */
static uint32_t
sa1110_bus_read_end (urj_bus_t *bus)
{
    /* see Figure 10-12 in [1] */
    urj_part_t *p = bus->part;
    urj_chain_t *chain = bus->chain;
    int i;
    uint32_t d = 0;
    urj_bus_area_t area;

    sa1110_bus_area (bus, 0, &area);

    urj_part_set_signal_high (p, nCS[0]);
    urj_part_set_signal_high (p, nCS[1]);
    urj_part_set_signal_high (p, nCS[2]);
    urj_part_set_signal_high (p, nCS[3]);
    urj_part_set_signal_high (p, nCS[4]);
    urj_part_set_signal_high (p, nCS[5]);
    urj_part_set_signal_high (p, nOE);
    urj_tap_chain_shift_data_registers (chain, 1);

    for (i = 0; i < area.width; i++)
        d |= (uint32_t) (urj_part_get_signal (p, D[i]) << i);

    return d;
}

/**
 * bus->driver->(*write)
 *
 */
static void
sa1110_bus_write (urj_bus_t *bus, uint32_t adr, uint32_t data)
{
    /* see Figure 10-16 in [1] */
    urj_part_t *p = bus->part;
    urj_chain_t *chain = bus->chain;

    urj_part_set_signal (p, nCS[0], 1, (adr >> 27) != 0);
    urj_part_set_signal (p, nCS[1], 1, (adr >> 27) != 1);
    urj_part_set_signal (p, nCS[2], 1, (adr >> 27) != 2);
    urj_part_set_signal (p, nCS[3], 1, (adr >> 27) != 3);
    urj_part_set_signal (p, nCS[4], 1, (adr >> 27) != 8);
    urj_part_set_signal (p, nCS[5], 1, (adr >> 27) != 9);
    urj_part_set_signal_low (p, RD_nWR);
    urj_part_set_signal_high (p, nWE);
    urj_part_set_signal_high (p, nOE);

    setup_address (bus, adr);
    setup_data (bus, data);

    urj_tap_chain_shift_data_registers (chain, 0);

    urj_part_set_signal_low (p, nWE);
    urj_tap_chain_shift_data_registers (chain, 0);
    urj_part_set_signal_high (p, nWE);
    urj_part_set_signal_high (p, nCS[0]);
    urj_part_set_signal_high (p, nCS[1]);
    urj_part_set_signal_high (p, nCS[2]);
    urj_part_set_signal_high (p, nCS[3]);
    urj_part_set_signal_high (p, nCS[4]);
    urj_part_set_signal_high (p, nCS[5]);
    urj_tap_chain_shift_data_registers (chain, 0);
}

const urj_bus_driver_t urj_bus_sa1110_bus = {
    "sa1110",
    N_("Intel SA-1110 compatible bus driver via BSR"),
    sa1110_bus_new,
    urj_bus_generic_free,
    sa1110_bus_printinfo,
    urj_bus_generic_prepare_extest,
    sa1110_bus_area,
    sa1110_bus_read_start,
    sa1110_bus_read_next,
    sa1110_bus_read_end,
    urj_bus_generic_read,
    urj_bus_generic_write_start,
    sa1110_bus_write,
    urj_bus_generic_no_init,
    urj_bus_generic_no_enable,
    urj_bus_generic_no_disable,
    URJ_BUS_TYPE_PARALLEL,
};
