/*
 * $Id$
 *
 * Analog Device's SHARC 21369 compatible bus driver via BSR
 * Copyright (C) 2009- Monami Software Limited Partnership, JAPAN.
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
 * Written by Masaki Muranaka <monaka at monami-software.com>
 *  Original version written by Ajith Kumar P.C <ajithpc@kila.com>
 *
 * Documentation:
 *      [1] Analog Devices Inc.,"ADSP-21369 SHARC Technical Reference", September 1998
 *      [2] Analog Devices Inc.,"ADSP-21369 SHARC User's Manual", September 1998
 *
 */

#include <sysdep.h>

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <urjtag/chain.h>
#include <urjtag/part.h>
#include <urjtag/bus.h>
#include <urjtag/bssignal.h>

#include "buses.h"
#include "generic_bus.h"

//no SDRAM access

typedef struct {
    uint32_t last_adr;
    urj_part_signal_t *addr[24];
    urj_part_signal_t *data[32];
    urj_part_signal_t *ms0;    //boot memory select
    urj_part_signal_t *ms1;    //boot memory select
    urj_part_signal_t *nwe;
    urj_part_signal_t *nrd;
} bus_params_t;

#define LAST_ADR ((bus_params_t *) bus->params)->last_adr
#define ADDR     ((bus_params_t *) bus->params)->addr
#define DATA     ((bus_params_t *) bus->params)->data
#define MS0      ((bus_params_t *) bus->params)->ms0
#define MS1      ((bus_params_t *) bus->params)->ms1
#define nWE      ((bus_params_t *) bus->params)->nwe
#define nRD      ((bus_params_t *) bus->params)->nrd

/**
 * bus->driver->(*new_bus)
 *
 */
static urj_bus_t *
sharc_21369_bus_new (urj_chain_t *chain, const urj_bus_driver_t *driver,
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

    for (i = 0; i < 24; i++) {
        sprintf( buff, "ADDR%d", i );
        failed |= urj_bus_generic_attach_sig (part, &(ADDR[i]), buff);
    }

    for (i = 0; i < 32; i++) {
        sprintf( buff, "DATA%d", i );
        failed |= urj_bus_generic_attach_sig( part, &(DATA[i]), buff );
    }

    failed |= urj_bus_generic_attach_sig( part, &(MS0), "MS0_B" );
    failed |= urj_bus_generic_attach_sig( part, &(MS1), "MS1_B" );
    failed |= urj_bus_generic_attach_sig( part, &(nWE), "WR_B"  );
    failed |= urj_bus_generic_attach_sig( part, &(nRD), "RD_B"  );

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
sharc_21369_bus_printinfo (urj_log_level_t ll, urj_bus_t *bus )
{
    int i;

    for (i = 0; i < bus->chain->parts->len; i++)
        if (bus->part == bus->chain->parts->parts[i])
            break;
    urj_log(ll, _("Analog Device's SHARC 21369 compatible bus driver via BSR (JTAG part No. %d)\n"),
            i );
}

/**
 * bus->driver->(*area)
 *
 */
static int
sharc_21369_ezkit_bus_area ( urj_bus_t *bus, uint32_t adr, urj_bus_area_t *area)
{
    urj_part_t *p = bus->part;

    if (UINT32_C(0x200000) <= adr && adr <= UINT32_C(0x027FFFF))
    {
        area->description = N_("Boot Memory Select");
        area->start = UINT32_C(0x200000);
        area->length = UINT64_C(0x080000);
        area->width = 8;

        urj_part_set_signal_high( p, MS0);
        urj_part_set_signal_low( p, MS1);

    }
    else
    {
        area->description = NULL;
        area->start = UINT32_C(0xffffffff);
        area->length = UINT64_C(0x080000);
        area->width = 0;

        urj_part_set_signal_high(p, MS0);
        urj_part_set_signal_high(p, MS1);
    }

    return URJ_STATUS_OK;
}

static void
setup_address (urj_bus_t *bus, uint32_t a)
{
    int i;
    urj_part_t *p = bus->part;

    for (i = 0; i < 24; i++)
        urj_part_set_signal (p, ADDR[i], 1, (a >> i) & 1);
}

static void
set_data_in(urj_bus_t *bus, uint32_t adr)
{
    int i;
    urj_part_t *p = bus->part;
    urj_bus_area_t area;

    bus->driver->area (bus, adr, &area);

    for (i = 0; i < area.width; i++)
        urj_part_set_signal( p, DATA[i], 0, 0 );
}


static void
setup_data (urj_bus_t *bus, uint32_t adr, uint32_t d)
{
    int i;
    urj_part_t *p = bus->part;
    urj_bus_area_t area;

    bus->driver->area (bus, adr, &area);

    for (i = 0; i < area.width; i++)
        urj_part_set_signal(p, DATA[i], 1, (d >> i) & 1);
}

/**
 * bus->driver->(*read_start)
 *
 */
static int
sharc_21369_bus_read_start (urj_bus_t *bus, uint32_t adr)
{
    urj_chain_t *chain = bus->chain;
    urj_part_t *p = bus->part;
    urj_bus_area_t area;

    bus->driver->area (bus, adr, &area);

    LAST_ADR = adr;

    urj_part_set_signal_high (p, nWE);
    urj_part_set_signal_low (p, nRD);

    setup_address (bus, adr);
    set_data_in (bus, adr);

    urj_tap_chain_shift_data_registers (chain, 0);

    return URJ_STATUS_OK;
}

/**
 * bus->driver->(*read_next)
 *
 */
static uint32_t
sharc_21369_bus_read_next (urj_bus_t *bus, uint32_t adr )
{
    urj_part_t *p = bus->part;
    urj_chain_t *chain = bus->chain;
    uint32_t d = 0;
    int i;
    urj_bus_area_t area;

    LAST_ADR = adr;

    bus->driver->area (bus, adr, &area);

    setup_address( bus, adr );
    urj_tap_chain_shift_data_registers( chain, 1 );

    for (i = 0; i < area.width; i++)
        d |= (uint32_t) (urj_part_get_signal (p, DATA[i]) << i);

    return d;
}

/**
 * bus->driver->(*read_end)
 *
 */
static uint32_t
sharc_21369_bus_read_end( urj_bus_t *bus )
{
    urj_part_t *p = bus->part;
    urj_chain_t *chain = bus->chain;
    int i;
    uint32_t d = 0;
    urj_bus_area_t area;

    bus->driver->area (bus, LAST_ADR, &area);

    urj_part_set_signal_high (p, nWE);
    urj_part_set_signal_high (p, nRD);

    urj_tap_chain_shift_data_registers (chain, 1);

    for (i = 0; i < area.width; i++)
        d |= (uint32_t) (urj_part_get_signal (p, DATA[i]) << i);

    return d;
}

/**
 * bus->driver->(*write)
 *
 */
static void
sharc_21369_bus_write (urj_bus_t *bus, uint32_t adr, uint32_t data)
{
    urj_part_t *p = bus->part;
    urj_chain_t *chain = bus->chain;
    urj_bus_area_t area;

    bus->driver->area (bus, adr, &area);

    urj_part_set_signal_high (p, nWE);
    urj_part_set_signal_high (p, nRD);

    setup_address (bus, adr);
    setup_data (bus, adr, data);

    urj_tap_chain_shift_data_registers (chain, 0);

    urj_part_set_signal_low (p, nWE);
    urj_tap_chain_shift_data_registers (chain, 0);
    urj_part_set_signal_high (p, nWE);
    urj_tap_chain_shift_data_registers (chain, 0);
}

const urj_bus_driver_t urj_bus_sharc_21369_ezkit_bus = {
    "SHARC_21369_EZKIT",
    N_("SHARC_21369 EZ-KIT bus driver via BSR"),
    sharc_21369_bus_new,
    urj_bus_generic_free,
    sharc_21369_bus_printinfo,
    urj_bus_generic_prepare_extest,
    sharc_21369_ezkit_bus_area,
    sharc_21369_bus_read_start,
    sharc_21369_bus_read_next,
    sharc_21369_bus_read_end,
    urj_bus_generic_read,
    urj_bus_generic_write_start,
    sharc_21369_bus_write,
    urj_bus_generic_no_init,
    urj_bus_generic_no_enable,
    urj_bus_generic_no_disable,
    URJ_BUS_TYPE_PARALLEL,
};
