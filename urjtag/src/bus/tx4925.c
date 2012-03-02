/*
 * $Id$
 *
 * Copyright (C) 2003 RightHand Technologies, Inc.
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
 * Christian Pellegrin <chri@ascensit.com>, 2003.
 * Modified by Marcel Telka <marcel@telka.sk>, 2003.
 * Modified by Andrew Dyer <adyer@righthandtech.com>, 2003.
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

// FIXME board level write protect is ignored here
//  should be okay because pin isn't implemented
//  on 29LV200 we use now.

typedef struct
{
    urj_part_signal_t *oe;
    urj_part_signal_t *swe;
    urj_part_signal_t *romce[4];
    urj_part_signal_t *sdcs[4];
    urj_part_signal_t *addr[20];
    urj_part_signal_t *data[16];
} bus_params_t;

#define OE      ((bus_params_t *) bus->params)->oe
#define SWE     ((bus_params_t *) bus->params)->swe
#define ROMCE   ((bus_params_t *) bus->params)->romce
#define SDCS    ((bus_params_t *) bus->params)->sdcs
#define ADDR    ((bus_params_t *) bus->params)->addr
#define DATA    ((bus_params_t *) bus->params)->data

// the number of bytes wide that the TX4925
// CS0 signal is set to by the external
// config resistors on A13/A12 at reset
// 1, 2, or 4 are legal values

#define TX4925_FLASH_CS_WIDTH 2

/**
 * bus->driver->(*new_bus)
 *
 */
static urj_bus_t *
tx4925_bus_new (urj_chain_t *chain, const urj_bus_driver_t *driver,
                const urj_param_t *cmd_params[])
{
    urj_bus_t *bus;
    urj_part_t *part;
    char buff[15];
    int i;
    int failed = 0;

    bus = urj_bus_generic_new (chain, driver, sizeof (bus_params_t));
    if (bus == NULL)
        return NULL;
    part = bus->part;

    failed |= urj_bus_generic_attach_sig (part, &(OE), "OE");

    failed |= urj_bus_generic_attach_sig (part, &(SWE), "SWE");

    for (i = 0; i < 4; i++)
    {
        sprintf (buff, "ROMCE_%d", i);
        failed |= urj_bus_generic_attach_sig (part, &(ROMCE[i]), buff);
    }

    for (i = 0; i < 4; i++)
    {
        sprintf (buff, "SDCS_%d", i);
        failed |= urj_bus_generic_attach_sig (part, &(SDCS[i]), buff);
    }

    for (i = 0; i < 20; i++)
    {
        sprintf (buff, "ADDR_%d", i);
        failed |= urj_bus_generic_attach_sig (part, &(ADDR[i]), buff);
    }

    for (i = 0; i < 16; i++)
    {
        sprintf (buff, "DATA_%d", i);
        failed |= urj_bus_generic_attach_sig (part, &(DATA[i]), buff);
    }

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
tx4925_bus_printinfo (urj_log_level_t ll, urj_bus_t *bus)
{
    int i;

    for (i = 0; i < bus->chain->parts->len; i++)
        if (bus->part == bus->chain->parts->parts[i])
            break;
    urj_log (ll, _("Toshiba TX4925 compatible bus driver via BSR (JTAG part No. %d)\n"),
            i);
}

/**
 * bus->driver->(*area)
 *
 */
static int
tx4925_bus_area (urj_bus_t *bus, uint32_t adr, urj_bus_area_t *area)
{
    area->description = NULL;
    area->start = UINT32_C (0x00000000);
    area->length = UINT64_C (0x100000000);
    area->width = 16;

    return URJ_STATUS_OK;
}

static void
select_flash (urj_bus_t *bus)
{
    urj_part_t *p = bus->part;

    urj_part_set_signal_low (p, ROMCE[0]);
    urj_part_set_signal_high (p, ROMCE[1]);
    urj_part_set_signal_high (p, ROMCE[2]);
    urj_part_set_signal_high (p, ROMCE[3]);
    urj_part_set_signal_high (p, SDCS[0]);
    urj_part_set_signal_high (p, SDCS[1]);
    urj_part_set_signal_high (p, SDCS[2]);
    urj_part_set_signal_high (p, SDCS[3]);
}

static void
unselect_flash (urj_bus_t *bus)
{
    urj_part_t *p = bus->part;

    urj_part_set_signal_high (p, ROMCE[0]);
    urj_part_set_signal_high (p, ROMCE[1]);
    urj_part_set_signal_high (p, ROMCE[2]);
    urj_part_set_signal_high (p, ROMCE[3]);
    urj_part_set_signal_high (p, SDCS[0]);
    urj_part_set_signal_high (p, SDCS[1]);
    urj_part_set_signal_high (p, SDCS[2]);
    urj_part_set_signal_high (p, SDCS[3]);
}

static void
setup_address (urj_bus_t *bus, uint32_t a)
{
    int i;
    urj_part_t *p = bus->part;
    int addr_shift = (TX4925_FLASH_CS_WIDTH / 2);

    for (i = 0; i < 20; i++)
        urj_part_set_signal (p, ADDR[i], 1, (a >> (i + addr_shift)) & 1);
}

static void
set_data_in (urj_bus_t *bus)
{
    int i;
    urj_part_t *p = bus->part;

    for (i = 0; i < 16; i++)
        urj_part_set_signal_input (p, DATA[i]);
}

static void
setup_data (urj_bus_t *bus, uint32_t d)
{
    int i;
    urj_part_t *p = bus->part;

    for (i = 0; i < 16; i++)
        urj_part_set_signal (p, DATA[i], 1, (d >> i) & 1);
}

/**
 * bus->driver->(*read_start)
 *
 */
static int
tx4925_bus_read_start (urj_bus_t *bus, uint32_t adr)
{
    urj_part_t *p = bus->part;
    urj_chain_t *chain = bus->chain;

    select_flash (bus);
    setup_address (bus, adr);
    urj_part_set_signal_low (p, OE);
    urj_part_set_signal_high (p, SWE);

    set_data_in (bus);

    urj_tap_chain_shift_data_registers (chain, 0);

    return URJ_STATUS_OK;
}

/**
 * bus->driver->(*read_next)
 *
 */
static uint32_t
tx4925_bus_read_next (urj_bus_t *bus, uint32_t adr)
{
    urj_part_t *p = bus->part;
    urj_chain_t *chain = bus->chain;
    int i;
    uint32_t d = 0;

    setup_address (bus, adr);
    urj_tap_chain_shift_data_registers (chain, 1);

    for (i = 0; i < 16; i++)
        d |= (uint32_t) (urj_part_get_signal (p, DATA[i]) << i);

    return d;
}

/**
 * bus->driver->(*read_end)
 *
 */
static uint32_t
tx4925_bus_read_end (urj_bus_t *bus)
{
    urj_part_t *p = bus->part;
    urj_chain_t *chain = bus->chain;
    int i;
    uint32_t d = 0;

    unselect_flash (bus);
    urj_part_set_signal_high (p, OE);
    urj_part_set_signal_high (p, SWE);

    urj_tap_chain_shift_data_registers (chain, 1);

    for (i = 0; i < 16; i++)
        d |= (uint32_t) (urj_part_get_signal (p, DATA[i]) << i);

    return d;
}

/**
 * bus->driver->(*write)
 *
 */
static void
tx4925_bus_write (urj_bus_t *bus, uint32_t adr, uint32_t data)
{
    urj_part_t *p = bus->part;
    urj_chain_t *chain = bus->chain;

    select_flash (bus);
    urj_part_set_signal_high (p, OE);

    setup_address (bus, adr);
    setup_data (bus, data);

    urj_tap_chain_shift_data_registers (chain, 0);

    urj_part_set_signal_low (p, SWE);
    urj_tap_chain_shift_data_registers (chain, 0);
    urj_part_set_signal_high (p, SWE);
    unselect_flash (bus);
    urj_tap_chain_shift_data_registers (chain, 0);
}

const urj_bus_driver_t urj_bus_tx4925_bus = {
    "tx4925",
    N_("Toshiba TX4925 compatible bus driver via BSR"),
    tx4925_bus_new,
    urj_bus_generic_free,
    tx4925_bus_printinfo,
    urj_bus_generic_prepare_extest,
    tx4925_bus_area,
    tx4925_bus_read_start,
    tx4925_bus_read_next,
    tx4925_bus_read_end,
    urj_bus_generic_read,
    urj_bus_generic_write_start,
    tx4925_bus_write,
    urj_bus_generic_no_init,
    urj_bus_generic_no_enable,
    urj_bus_generic_no_disable,
    URJ_BUS_TYPE_PARALLEL,
};
