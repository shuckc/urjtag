/*
 * $Id: bf533_ezkit.c,v 1.1.1.1 2005/03/18 15:03:05 klingler Exp $
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
 *
 */

#include "sysdep.h"

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "part.h"
#include "bus.h"
#include "chain.h"
#include "bssignal.h"
#include "jtag.h"
#include "buses.h"
#include "generic_bus.h"

typedef struct
{
    urj_part_signal_t *ams[4];
    urj_part_signal_t *addr[19];
    urj_part_signal_t *data[16];
    urj_part_signal_t *abe[2];
    urj_part_signal_t *awe;
    urj_part_signal_t *aoe;
    urj_part_signal_t *sras;
    urj_part_signal_t *scas;
    urj_part_signal_t *sms;
    urj_part_signal_t *swe;
} bus_params_t;

#define AMS     ((bus_params_t *) bus->params)->ams
#define ADDR    ((bus_params_t *) bus->params)->addr
#define DATA    ((bus_params_t *) bus->params)->data
#define AWE     ((bus_params_t *) bus->params)->awe
#define AOE     ((bus_params_t *) bus->params)->aoe
#define ABE     ((bus_params_t *) bus->params)->abe
#define SRAS    ((bus_params_t *) bus->params)->sras
#define SCAS    ((bus_params_t *) bus->params)->scas
#define SMS     ((bus_params_t *) bus->params)->sms
#define SWE     ((bus_params_t *) bus->params)->swe

/**
 * bus->driver->(*new_bus)
 *
 */
static urj_bus_t *
bf533_ezkit_bus_new (urj_chain_t *chain, const urj_bus_driver_t *driver,
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
        sprintf (buff, "AMS_B%d", i);
        failed |= urj_bus_generic_attach_sig (part, &(AMS[i]), buff);
    }

    for (i = 0; i < 19; i++)
    {
        sprintf (buff, "ADDR[%d]", i + 1);
        failed |= urj_bus_generic_attach_sig (part, &(ADDR[i]), buff);
    }

    for (i = 0; i < 16; i++)
    {
        sprintf (buff, "DATA[%d]", i);
        failed |= urj_bus_generic_attach_sig (part, &(DATA[i]), buff);
    }

    failed |= urj_bus_generic_attach_sig (part, &(AWE), "AWE_B");

    failed |= urj_bus_generic_attach_sig (part, &(AOE), "AOE_B");

    failed |= urj_bus_generic_attach_sig (part, &(ABE[0]), "ABE_B0");

    failed |= urj_bus_generic_attach_sig (part, &(ABE[1]), "ABE_B1");

    failed |= urj_bus_generic_attach_sig (part, &(SRAS), "SRAS_B");

    failed |= urj_bus_generic_attach_sig (part, &(SCAS), "SCAS_B");

    failed |= urj_bus_generic_attach_sig (part, &(SWE), "SWE_B");

    failed |= urj_bus_generic_attach_sig (part, &(SMS), "SMS_B");

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
bf533_ezkit_bus_printinfo (urj_bus_t *bus)
{
    int i;

    for (i = 0; i < bus->chain->parts->len; i++)
        if (bus->part == bus->chain->parts->parts[i])
            break;
    printf (_
            ("Blackfin BF533 EZKit compatible bus driver via BSR (JTAG part No. %d)\n"),
            i);
}

/**
 * bus->driver->(*area)
 *
 */
static int
bf533_ezkit_bus_area (urj_bus_t *bus, uint32_t adr, urj_bus_area_t *area)
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

    urj_part_set_signal (p, ABE[0], 1, 0);
    urj_part_set_signal (p, ABE[1], 1, 0);

    urj_part_set_signal (p, SRAS, 1, 1);
    urj_part_set_signal (p, SCAS, 1, 1);
    urj_part_set_signal (p, SWE, 1, 1);
    urj_part_set_signal (p, SMS, 1, 1);
}

static void
unselect_flash (urj_bus_t *bus)
{
    urj_part_t *p = bus->part;

    urj_part_set_signal (p, AMS[0], 1, 1);
    urj_part_set_signal (p, AMS[1], 1, 1);
    urj_part_set_signal (p, AMS[2], 1, 1);
    urj_part_set_signal (p, AMS[3], 1, 1);

    urj_part_set_signal (p, ABE[0], 1, 1);
    urj_part_set_signal (p, ABE[1], 1, 1);

    urj_part_set_signal (p, SRAS, 1, 1);
    urj_part_set_signal (p, SCAS, 1, 1);
    urj_part_set_signal (p, SWE, 1, 1);
    urj_part_set_signal (p, SMS, 1, 1);
}

static void
setup_address (urj_bus_t *bus, uint32_t a)
{
    int i;
    urj_part_t *p = bus->part;

    for (i = 0; i < 19; i++)
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
bf533_ezkit_bus_read_start (urj_bus_t *bus, uint32_t adr)
{
    urj_part_t *p = bus->part;
    urj_chain_t *chain = bus->chain;

    select_flash (bus);
    urj_part_set_signal (p, AOE, 1, 0);
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
bf533_ezkit_bus_read_next (urj_bus_t *bus, uint32_t adr)
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
bf533_ezkit_bus_read_end (urj_bus_t *bus)
{
    urj_part_t *p = bus->part;
    urj_chain_t *chain = bus->chain;
    int i;
    uint32_t d = 0;

    unselect_flash (bus);
    urj_part_set_signal (p, AOE, 1, 1);
    urj_part_set_signal (p, AWE, 1, 1);

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
bf533_ezkit_bus_write (urj_bus_t *bus, uint32_t adr, uint32_t data)
{
    urj_part_t *p = bus->part;
    urj_chain_t *chain = bus->chain;

//      printf("Writing %04X to %08X...\n", data, adr);

    select_flash (bus);
    urj_part_set_signal (p, AOE, 1, 1);

    setup_address (bus, adr);
    setup_data (bus, data);

    urj_tap_chain_shift_data_registers (chain, 0);

    urj_part_set_signal (p, AWE, 1, 0);
    urj_tap_chain_shift_data_registers (chain, 0);
    urj_part_set_signal (p, AWE, 1, 1);
    unselect_flash (bus);
    urj_tap_chain_shift_data_registers (chain, 0);
}

const urj_bus_driver_t bf533_ezkit_bus = {
    "bf533_ezkit",
    N_("Blackfin BF533 EZKit board bus driver"),
    bf533_ezkit_bus_new,
    urj_bus_generic_free,
    bf533_ezkit_bus_printinfo,
    urj_bus_generic_prepare_extest,
    bf533_ezkit_bus_area,
    bf533_ezkit_bus_read_start,
    bf533_ezkit_bus_read_next,
    bf533_ezkit_bus_read_end,
    urj_bus_generic_read,
    bf533_ezkit_bus_write,
    urj_bus_generic_no_init
};
