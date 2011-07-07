/*
 * $Id$
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
 * Written by Marko Roessler <marko.roessler@indakom.de>, 2004
 *
 * based on sa1110.c
 * Written by Marcel Telka <marcel@telka.sk>, 2002, 2003.
 *
 * Documentation:
 *
 * [1] Sharp Microelectronics, "LH7A400 Universal SOC Preliminary
 *     Users's Guide", May 2003, Reference No. SMA02010
 *
 *
 * Notes:
 *        - this bus driver ONLY works for the asynchronous boot mode!
 *        - use only to access flash devices
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

#define ADR_NUM         24
#define D_NUM           32
#define nCS_NUM         4
#define WIDTH_NUM       2

typedef struct
{
    urj_part_signal_t *a[ADR_NUM];
    urj_part_signal_t *d[D_NUM];
    urj_part_signal_t *ncs[nCS_NUM];
    urj_part_signal_t *nwe;
    urj_part_signal_t *noe;
    urj_part_signal_t *width[WIDTH_NUM];
} bus_params_t;

#define A       ((bus_params_t *) bus->params)->a
#define D       ((bus_params_t *) bus->params)->d
#define nCS     ((bus_params_t *) bus->params)->ncs
#define nWE     ((bus_params_t *) bus->params)->nwe
#define nOE     ((bus_params_t *) bus->params)->noe
#define WIDTH   ((bus_params_t *) bus->params)->width

/**
 * bus->driver->(*new_bus)
 *
 */
static urj_bus_t *
lh7a400_bus_new (urj_chain_t *chain, const const urj_bus_driver_t *driver,
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

    for (i = 0; i < ADR_NUM; i++)
    {
        sprintf (buff, "A%d", i);
        failed |= urj_bus_generic_attach_sig (part, &(A[i]), buff);
    }

    for (i = 0; i < D_NUM; i++)
    {
        sprintf (buff, "D%d", i);
        failed |= urj_bus_generic_attach_sig (part, &(D[i]), buff);
    }

    for (i = 0; i < nCS_NUM; i++)
    {
        sprintf (buff, "nCS%d", i);
        failed |= urj_bus_generic_attach_sig (part, &(nCS[i]), buff);
    }

    for (i = 0; i < WIDTH_NUM; i++)
    {
        sprintf (buff, "WIDTH%d", i);
        failed |= urj_bus_generic_attach_sig (part, &(WIDTH[i]), buff);
    }

    failed |= urj_bus_generic_attach_sig (part, &(nWE), "nWE0");

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
lh7a400_bus_printinfo (urj_log_level_t ll, urj_bus_t *bus)
{
    int i;

    for (i = 0; i < bus->chain->parts->len; i++)
        if (bus->part == bus->chain->parts->parts[i])
            break;
    urj_log (ll, _("Sharp LH7A400 compatible bus driver via BSR (JTAG part No. %d)\n"),
            i);
}

/**
 * bus->driver->(*area)
 *
 */
static int
lh7a400_bus_area (urj_bus_t *bus, uint32_t adr, urj_bus_area_t *area)
{
    unsigned int width;

    area->description = NULL;
    area->start = UINT32_C (0x00000000);
    area->length = UINT64_C (0x10000000);
    area->width = -1;   // some gcc versions detect uninitialised use

    /* we determine the size of the flash that was booted from [1] table 3.1 */
    width =
        urj_part_get_signal (bus->part,
                             urj_part_find_signal (bus->part, "WIDTH0"));
    width |=
        urj_part_get_signal (bus->part,
                             urj_part_find_signal (bus->part, "WIDTH1")) << 1;

    if (width < 0)
        return URJ_STATUS_FAIL;

    switch (width)
    {
    case 0:
        area->width = 8;
        break;
    case 1:
        area->width = 16;
        break;
    case 2:
    case 3:
        area->width = 32;
    }

    return URJ_STATUS_OK;
}

static void
setup_address (urj_bus_t *bus, uint32_t a)
{
    int i;
    urj_part_t *p = bus->part;

    for (i = 0; i < ADR_NUM; i++)
        urj_part_set_signal (p, A[i], 1, (a >> i) & 1);
}

static void
set_data_in (urj_bus_t *bus)
{
    int i;
    urj_part_t *p = bus->part;
    urj_bus_area_t area;

    // @@@@ RFHH check result
    lh7a400_bus_area (bus, 0, &area);

    for (i = 0; i < area.width; i++)
        urj_part_set_signal (p, D[i], 0, 0);

}

static void
setup_data (urj_bus_t *bus, uint32_t d)
{
    int i;
    urj_part_t *p = bus->part;
    urj_bus_area_t area;

    lh7a400_bus_area (bus, 0, &area);

    for (i = 0; i < area.width; i++)
        urj_part_set_signal (p, D[i], 1, (d >> i) & 1);
}

/**
 * bus->driver->(*read_start)
 *
 */
static int
lh7a400_bus_read_start (urj_bus_t *bus, uint32_t adr)
{
    /* see Figure 3-3 in [1] */
    urj_part_t *p = bus->part;
    urj_chain_t *chain = bus->chain;

    urj_part_set_signal (p, nCS[0], 1, (adr >> 27) != 0);
    urj_part_set_signal (p, nWE, 1, 1);
    urj_part_set_signal (p, nOE, 1, 0);

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
lh7a400_bus_read_next (urj_bus_t *bus, uint32_t adr)
{
    /* see Figure 3-3 in [1] */
    urj_part_t *p = bus->part;
    urj_chain_t *chain = bus->chain;
    int i;
    uint32_t d = 0;
    urj_bus_area_t area;

    lh7a400_bus_area (bus, adr, &area);

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
lh7a400_bus_read_end (urj_bus_t *bus)
{
    /* see Figure 3-3 in [1] */
    urj_part_t *p = bus->part;
    urj_chain_t *chain = bus->chain;
    int i;
    uint32_t d = 0;
    urj_bus_area_t area;

    lh7a400_bus_area (bus, 0, &area);

    urj_part_set_signal (p, nCS[0], 1, 1);
    urj_part_set_signal (p, nOE, 1, 1);

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
lh7a400_bus_write (urj_bus_t *bus, uint32_t adr, uint32_t data)
{
    /* see Figure 3-3 in [1] */
    urj_part_t *p = bus->part;
    urj_chain_t *chain = bus->chain;

    urj_part_set_signal (p, nCS[0], 1, (adr >> 27) != 0);
    urj_part_set_signal (p, nWE, 1, 1);
    urj_part_set_signal (p, nOE, 1, 1);

    setup_address (bus, adr);
    setup_data (bus, data);

    urj_tap_chain_shift_data_registers (chain, 0);

    urj_part_set_signal (p, nWE, 1, 0);
    urj_tap_chain_shift_data_registers (chain, 0);

    urj_part_set_signal (p, nWE, 1, 1);
    urj_part_set_signal (p, nCS[0], 1, 1);
    urj_tap_chain_shift_data_registers (chain, 0);
}

const urj_bus_driver_t urj_bus_lh7a400_bus = {
    "lh7a400",
    N_("Sharp LH7A400 compatible bus driver via BSR (flash access only!)"),
    lh7a400_bus_new,
    urj_bus_generic_free,
    lh7a400_bus_printinfo,
    urj_bus_generic_prepare_extest,
    lh7a400_bus_area,
    lh7a400_bus_read_start,
    lh7a400_bus_read_next,
    lh7a400_bus_read_end,
    urj_bus_generic_read,
    urj_bus_generic_write_start,
    lh7a400_bus_write,
    urj_bus_generic_no_init,
    urj_bus_generic_no_enable,
    urj_bus_generic_no_disable,
    URJ_BUS_TYPE_PARALLEL,
};
