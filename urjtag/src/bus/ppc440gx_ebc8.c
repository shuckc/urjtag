/*
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

#define PPC440GX_ADDR_LINES 32
#define PPC440GX_DATA_LINES  8

typedef struct
{
    urj_part_signal_t *a[PPC440GX_ADDR_LINES];
    urj_part_signal_t *d[PPC440GX_DATA_LINES];
    urj_part_signal_t *ncs;
    urj_part_signal_t *nwe;
    urj_part_signal_t *noe;
} bus_params_t;

#define A       ((bus_params_t *) bus->params)->a
#define D       ((bus_params_t *) bus->params)->d
#define nCS     ((bus_params_t *) bus->params)->ncs
#define nWE     ((bus_params_t *) bus->params)->nwe
#define nOE     ((bus_params_t *) bus->params)->noe


/**
 * bus->driver->(*new_bus)
 *
 */
static urj_bus_t *
ppc440gx_ebc8_bus_new (urj_chain_t *chain, const urj_bus_driver_t *driver,
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

    for (i = 0; i < PPC440GX_ADDR_LINES; i++)
    {
        sprintf (buff, "EBCADR%d", i);
        failed |= urj_bus_generic_attach_sig (part, &(A[i]), buff);
    }

    for (i = 0; i < PPC440GX_DATA_LINES; i++)
    {
        sprintf (buff, "EBCDATA%d", i);
        failed |= urj_bus_generic_attach_sig (part, &(D[i]), buff);
    }

    failed |= urj_bus_generic_attach_sig (part, &(nCS), "EBCCS0_N");

    failed |= urj_bus_generic_attach_sig (part, &(nWE), "EBCWE_N");

    failed |= urj_bus_generic_attach_sig (part, &(nOE), "EBCOE_N");

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
ppc440gx_ebc8_bus_printinfo (urj_log_level_t ll, urj_bus_t *bus)
{
    int i;

    for (i = 0; i < bus->chain->parts->len; i++)
        if (bus->part == bus->chain->parts->parts[i])
            break;
    urj_log (ll, _("IBM PowerPC 440GX 8-bit compatible bus driver via BSR (JTAG part No. %d)\n"),
            i);
}

/**
 * bus->driver->(*area)
 *
 */
static int
ppc440gx_ebc8_bus_area (urj_bus_t *bus, uint32_t adr, urj_bus_area_t *area)
{
    area->description = NULL;
    area->start = UINT32_C (0x00000000);
    area->length = UINT64_C (0x100000000);      /* ??????????? */
    area->width = PPC440GX_DATA_LINES;

    return URJ_STATUS_OK;
}

static void
setup_address (urj_bus_t *bus, uint32_t a)
{
    int i;
    urj_part_t *p = bus->part;

    for (i = 0; i < PPC440GX_ADDR_LINES; i++)
        urj_part_set_signal (p, A[i], 1,
                             (a >> (PPC440GX_ADDR_LINES - 1 - i)) & 1);
}

static void
set_data_in (urj_bus_t *bus)
{
    int i;
    urj_part_t *p = bus->part;
    urj_bus_area_t area;

    ppc440gx_ebc8_bus_area (bus, 0, &area);

    for (i = 0; i < area.width; i++)
        urj_part_set_signal_input (p, D[i]);
}

static void
setup_data (urj_bus_t *bus, uint32_t d)
{
    int i;
    urj_part_t *p = bus->part;
    urj_bus_area_t area;

    ppc440gx_ebc8_bus_area (bus, 0, &area);

    for (i = 0; i < area.width; i++)
        urj_part_set_signal (p, D[PPC440GX_DATA_LINES - 1 - i], 1,
                             (d >> i) & 1);
}

/**
 * bus->driver->(*read_start)
 *
 */
static int
ppc440gx_ebc8_bus_read_start (urj_bus_t *bus, uint32_t adr)
{
    urj_part_t *p = bus->part;
    urj_chain_t *chain = bus->chain;

    urj_part_set_signal_low (p, nCS);
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
ppc440gx_ebc8_bus_read_next (urj_bus_t *bus, uint32_t adr)
{
    urj_part_t *p = bus->part;
    urj_chain_t *chain = bus->chain;
    int i;
    uint32_t d = 0;
    urj_bus_area_t area;

    ppc440gx_ebc8_bus_area (bus, adr, &area);

    setup_address (bus, adr);
    urj_tap_chain_shift_data_registers (chain, 1);

    for (i = 0; i < area.width; i++)
        d |= (uint32_t) (urj_part_get_signal
                         (p, D[PPC440GX_DATA_LINES - 1 - i]) << i);

    return d;
}

/**
 * bus->driver->(*read_end)
 *
 */
static uint32_t
ppc440gx_ebc8_bus_read_end (urj_bus_t *bus)
{
    urj_part_t *p = bus->part;
    urj_chain_t *chain = bus->chain;
    int i;
    uint32_t d = 0;
    urj_bus_area_t area;

    ppc440gx_ebc8_bus_area (bus, 0, &area);

    urj_part_set_signal_high (p, nCS);
    urj_part_set_signal_high (p, nOE);
    urj_tap_chain_shift_data_registers (chain, 1);

    for (i = 0; i < area.width; i++)
        d |= (uint32_t) (urj_part_get_signal
                         (p, D[PPC440GX_DATA_LINES - 1 - i]) << i);

    return d;
}

/**
 * bus->driver->(*write)
 *
 */
static void
ppc440gx_ebc8_bus_write (urj_bus_t *bus, uint32_t adr, uint32_t data)
{
    urj_part_t *p = bus->part;
    urj_chain_t *chain = bus->chain;

    urj_part_set_signal_low (p, nCS);
    urj_part_set_signal_high (p, nWE);
    urj_part_set_signal_high (p, nOE);

    setup_address (bus, adr);
    setup_data (bus, data);

    urj_tap_chain_shift_data_registers (chain, 0);

    urj_part_set_signal_low (p, nWE);
    urj_tap_chain_shift_data_registers (chain, 0);
    urj_part_set_signal_high (p, nWE);
    urj_part_set_signal_high (p, nCS);
    urj_tap_chain_shift_data_registers (chain, 0);
}

const urj_bus_driver_t urj_bus_ppc440gx_ebc8_bus = {
    "ppc440gx_ebc8",
    N_("IBM PowerPC 440GX 8-bit EBC compatible bus driver via BSR"),
    ppc440gx_ebc8_bus_new,
    urj_bus_generic_free,
    ppc440gx_ebc8_bus_printinfo,
    urj_bus_generic_prepare_extest,
    ppc440gx_ebc8_bus_area,
    ppc440gx_ebc8_bus_read_start,
    ppc440gx_ebc8_bus_read_next,
    ppc440gx_ebc8_bus_read_end,
    urj_bus_generic_read,
    urj_bus_generic_write_start,
    ppc440gx_ebc8_bus_write,
    urj_bus_generic_no_init,
    urj_bus_generic_no_enable,
    urj_bus_generic_no_disable,
    URJ_BUS_TYPE_PARALLEL,
};
