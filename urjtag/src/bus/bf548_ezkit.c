/*
 * bf548_ezkit.c
 *
 * Analog Devices ADSP-BF548 EZ-KIT Lite bus driver
 * Copyright (C) 2008 Analog Devices, Inc.
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

#include "sysdep.h"

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <urjtag/part.h>
#include <urjtag/bus.h>
#include <urjtag/chain.h>
#include <urjtag/bssignal.h>
#include <urjtag/jtag.h>

#include "buses.h"
#include "generic_bus.h"

typedef struct
{
    urj_part_signal_t *ams[4];
    urj_part_signal_t *addr[24];
    urj_part_signal_t *data[16];
    urj_part_signal_t *awe;
    urj_part_signal_t *are;
    urj_part_signal_t *aoe;
    urj_part_signal_t *dcs0;
    urj_part_signal_t *nce;
} bus_params_t;

#define AMS     ((bus_params_t *) bus->params)->ams
#define ADDR    ((bus_params_t *) bus->params)->addr
#define DATA    ((bus_params_t *) bus->params)->data
#define AOE     ((bus_params_t *) bus->params)->aoe
#define AWE     ((bus_params_t *) bus->params)->awe
#define ARE     ((bus_params_t *) bus->params)->are
#define DCS0    ((bus_params_t *) bus->params)->dcs0
#define NCE     ((bus_params_t *) bus->params)->nce

/**
 * bus->driver->(*new_bus)
 *
 */
static urj_bus_t *
bf548_ezkit_bus_new (urj_chain_t *chain, const urj_bus_driver_t *driver,
                     char *cmd_params[])
{
    urj_bus_t *bus;
    urj_part_t *part;
    char buff[15];
    int i;
    int failed = 0;

    bus = calloc (1, sizeof (urj_bus_t));
    if (!bus)
        return NULL;

    bus->driver = driver;
    bus->params = calloc (1, sizeof (bus_params_t));
    if (!bus->params)
    {
        free (bus);
        return NULL;
    }

    bus->chain = chain;
    bus->part = part = chain->parts->parts[chain->active_part];

    for (i = 0; i < 4; i++)
    {
        sprintf (buff, "AMS%dB", i);
        failed |= urj_bus_generic_attach_sig (part, &(AMS[i]), buff);
    }

    for (i = 0; i < 3; i++)
    {
        sprintf (buff, "ADDR%d", i + 1);
        failed |= urj_bus_generic_attach_sig (part, &(ADDR[i]), buff);
    }

    for (i = 3; i < 9; i++)
    {
        sprintf (buff, "PORTH_%d", i + 5);
        failed |= urj_bus_generic_attach_sig (part, &(ADDR[i]), buff);
    }

    for (i = 9; i < 24; i++)
    {
        sprintf (buff, "PORTI_%d", i - 9);
        failed |= urj_bus_generic_attach_sig (part, &(ADDR[i]), buff);
    }

    for (i = 0; i < 16; i++)
    {
        sprintf (buff, "DATA%d", i);
        failed |= urj_bus_generic_attach_sig (part, &(DATA[i]), buff);
    }

    failed |= urj_bus_generic_attach_sig (part, &(AWE), "AWEB");

    failed |= urj_bus_generic_attach_sig (part, &(ARE), "AREB");

    failed |= urj_bus_generic_attach_sig (part, &(AOE), "AOEB");

    failed |= urj_bus_generic_attach_sig (part, &(DCS0), "CS0_B");

    failed |= urj_bus_generic_attach_sig (part, &(NCE), "PORTJ_1");

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
bf548_ezkit_bus_printinfo (urj_bus_t *bus)
{
    int i;

    for (i = 0; i < bus->chain->parts->len; i++)
        if (bus->part == bus->chain->parts->parts[i])
            break;
    printf (_
            ("Blackfin BF548 EZ-KIT compatible bus driver via BSR (JTAG part No. %d)\n"),
            i);
}

/**
 * bus->driver->(*area)
 *
 */
static int
bf548_ezkit_bus_area (urj_bus_t *bus, uint32_t adr, urj_bus_area_t *area)
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

    urj_part_set_signal (p, AMS[0], 1, 0);
    urj_part_set_signal (p, AMS[1], 1, 1);
    urj_part_set_signal (p, AMS[2], 1, 1);
    urj_part_set_signal (p, AMS[3], 1, 1);
    urj_part_set_signal (p, DCS0, 1, 1);
    urj_part_set_signal (p, NCE, 1, 1);
}

static void
unselect_flash (urj_bus_t *bus)
{
    urj_part_t *p = bus->part;

    urj_part_set_signal (p, AMS[0], 1, 1);
    urj_part_set_signal (p, AMS[1], 1, 1);
    urj_part_set_signal (p, AMS[2], 1, 1);
    urj_part_set_signal (p, AMS[3], 1, 1);
    urj_part_set_signal (p, DCS0, 1, 1);
    urj_part_set_signal (p, NCE, 1, 1);
}

static void
setup_address (urj_bus_t *bus, uint32_t a)
{
    int i;
    urj_part_t *p = bus->part;

    for (i = 0; i < 24; i++)
        urj_part_set_signal (p, ADDR[i], 1, (a >> (i + 1)) & 1);
}

static void
set_data_in (urj_bus_t *bus)
{
    int i;
    urj_part_t *p = bus->part;

    for (i = 0; i < 16; i++)
        urj_part_set_signal (p, DATA[i], 0, 0);
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
static void
bf548_ezkit_bus_read_start (urj_bus_t *bus, uint32_t adr)
{
    urj_part_t *p = bus->part;
    urj_chain_t *chain = bus->chain;

    select_flash (bus);
    urj_part_set_signal (p, AOE, 1, 0);
    urj_part_set_signal (p, ARE, 1, 0);
    urj_part_set_signal (p, AWE, 1, 1);

    setup_address (bus, adr);
    set_data_in (bus);

    urj_tap_chain_shift_data_registers (chain, 0);
}

/**
 * bus->driver->(*read_next)
 *
 */
static uint32_t
bf548_ezkit_bus_read_next (urj_bus_t *bus, uint32_t adr)
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
bf548_ezkit_bus_read_end (urj_bus_t *bus)
{
    urj_part_t *p = bus->part;
    urj_chain_t *chain = bus->chain;
    int i;
    uint32_t d = 0;

    urj_part_set_signal (p, AOE, 1, 1);
    urj_part_set_signal (p, ARE, 1, 1);
    urj_part_set_signal (p, AWE, 1, 1);
    unselect_flash (bus);

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
bf548_ezkit_bus_write (urj_bus_t *bus, uint32_t adr, uint32_t data)
{
    urj_part_t *p = bus->part;
    urj_chain_t *chain = bus->chain;

    select_flash (bus);
    urj_part_set_signal (p, AOE, 1, 0);
    urj_part_set_signal (p, ARE, 1, 1);

    setup_address (bus, adr);
    setup_data (bus, data);

    urj_tap_chain_shift_data_registers (chain, 0);

    urj_part_set_signal (p, AWE, 1, 0);
    urj_tap_chain_shift_data_registers (chain, 0);
    urj_part_set_signal (p, AWE, 1, 1);
    urj_part_set_signal (p, AOE, 1, 1);
    unselect_flash (bus);
    urj_tap_chain_shift_data_registers (chain, 0);
}

const urj_bus_driver_t urj_bus_bf548_ezkit_bus = {
    "bf548_ezkit",
    N_("Blackfin BF548 EZ-KIT board bus driver"),
    bf548_ezkit_bus_new,
    urj_bus_generic_free,
    bf548_ezkit_bus_printinfo,
    urj_bus_generic_prepare_extest,
    bf548_ezkit_bus_area,
    bf548_ezkit_bus_read_start,
    bf548_ezkit_bus_read_next,
    bf548_ezkit_bus_read_end,
    urj_bus_generic_read,
    bf548_ezkit_bus_write,
    urj_bus_generic_no_init
};
