/*
 * $Id: mpc8313.c 1653 2009-06-15 20:19:43Z arniml $
 *
 * Freescale mpc8313 compatible bus driver via BSR
 * Copyright (C) 2011 Miklós Márton
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
 * Written by Miklós Márton <martonmiklosqdev@gmail.com>, 2011.
 */

#include <sysdep.h>

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <urjtag/part.h>
#include <urjtag/chain.h>
#include <urjtag/bus.h>
#include <urjtag/bssignal.h>

#include "buses.h"
#include "generic_bus.h"

#define BUS_WIDTH       16
#define A_WIDTH         26

typedef struct
{
    uint32_t last_adr;

    urj_part_signal_t *la[A_WIDTH];
    urj_part_signal_t *lad[BUS_WIDTH];
    urj_part_signal_t *ncs;
    urj_part_signal_t *nwe;
    urj_part_signal_t *nfoe;
    urj_part_signal_t *nwp;
    int lbc_num_d;
    char revbits;
} bus_params_t;

#define LAST_ADR        ((bus_params_t *) bus->params)->last_adr

#define LA              ((bus_params_t *) bus->params)->la
#define LAD             ((bus_params_t *) bus->params)->lad
#define nCS             ((bus_params_t *) bus->params)->ncs
#define nWE             ((bus_params_t *) bus->params)->nwe
#define nFOE            ((bus_params_t *) bus->params)->nfoe
#define nWP             ((bus_params_t *) bus->params)->nwp
#define REVBITS         ((bus_params_t *) bus->params)->revbits

/**
 * bus->driver->(*new_bus)
 *
 */
static urj_bus_t *
mpc8313_bus_new (urj_chain_t *chain, const urj_bus_driver_t *driver,
                 const urj_param_t *cmd_params[])
{
    urj_bus_t *bus;
    bus_params_t *bp;
    urj_part_t *part;
    char buff[10];
    int i;
    int failed = 0;
    const char *nwppin = NULL, *noepin = NULL, *ncspin = NULL, *nwepin = NULL;

    bus = urj_bus_generic_new (chain, driver, sizeof (bus_params_t));
    REVBITS = 0;

    for (i = 0; cmd_params[i] != NULL; i++)
    {
        switch (cmd_params[i]->key)
        {

        case URJ_BUS_PARAM_KEY_HELP:
            urj_log (URJ_LOG_LEVEL_NORMAL,
                     _("Usage: initbus mpc8313 [NOE=NOE] [NWE=NWE] [NCS=NCS]\n\n"
                       "   NOE     signal name to control output enable pin (LGPL2 for e.g.)\n"
                       "   NWE     signal name to control write enable pin (default - LWE_B0)\n"
                       "   NCS     signal name for the bus Chip select (default - LCS_B0)\n"
                       "   REVBITS reverse bits in data bus (default - no)\n"
                       "   NWP     signal name to control write protection pin if it neccessary (for e.g. TSEC2_RXD3)\n"));
            return NULL;
        case URJ_BUS_PARAM_KEY_NWP:
            nwppin = cmd_params[i]->value.string;
            break;
        case URJ_BUS_PARAM_KEY_NCS:
            ncspin = cmd_params[i]->value.string;
            break;
        case URJ_BUS_PARAM_KEY_NOE:
            noepin = cmd_params[i]->value.string;
            break;
        case URJ_BUS_PARAM_KEY_NWE:
            nwepin = cmd_params[i]->value.string;
            break;
        case URJ_BUS_PARAM_KEY_REVBITS:
            REVBITS = 1;
            break;
        default:
            urj_error_set (URJ_ERROR_SYNTAX, "unrecognised bus parameter '%s'",
                           urj_param_string(&urj_bus_param_list, cmd_params[i]));
            return NULL;
        }
    }

    if (bus == NULL)
        return NULL;

    bp = bus->params;
    bp->lbc_num_d = 16;

    part = bus->part;

    urj_part_set_instruction (part, "SAMPLE/PRELOAD");
    urj_tap_chain_shift_instructions (chain);
    urj_tap_chain_shift_data_registers (chain, 0);
    urj_part_set_instruction (part, "EXTEST");
    urj_tap_chain_shift_instructions (chain);
    urj_tap_chain_shift_data_registers (chain, 1);

    for (i = 0; i < A_WIDTH; i++)
    {
        sprintf (buff, "LA%d", i);
        failed |= urj_bus_generic_attach_sig (part, &(LA[i]), buff);
    }

    for (i = 0; i < BUS_WIDTH; i++)
    {
        sprintf (buff, "LAD%d", i);
        failed |= urj_bus_generic_attach_sig (part, &(LAD[i]), buff);
    }

    if (nwppin != NULL)
    {
         failed |= urj_bus_generic_attach_sig (part, &(nWP), nwppin);
         urj_part_set_signal_high (part, nWP);
    }

    if (noepin == NULL)
        failed = 255;
    else
        failed |= urj_bus_generic_attach_sig (part, &(nFOE), noepin);

    if (nwepin == NULL)
        failed |= urj_bus_generic_attach_sig (part, &(nWE), "LWE_B0");
    else
        failed |= urj_bus_generic_attach_sig (part, &(nWE), nwepin);

    if (ncspin == NULL)
        failed |= urj_bus_generic_attach_sig (part, &(nCS), "LCS_B0");
    else
        failed |= urj_bus_generic_attach_sig (part, &(nCS), ncspin);

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
mpc8313_bus_printinfo (urj_log_level_t ll, urj_bus_t *bus)
{
    int i;

    for (i = 0; i < bus->chain->parts->len; i++)
        if (bus->part == bus->chain->parts->parts[i])
            break;
    urj_log (ll, _("Freescale MPC8313 compatible bus driver via BSR (JTAG part No. %d)\n"),
            i);
}

/**
 * bus->driver->(*area)
 *
 */
static int
mpc8313_bus_area (urj_bus_t *bus, uint32_t adr, urj_bus_area_t *area)
{
    bus_params_t *bp = bus->params;

    area->description = N_("Local Bus Controller");
    area->start  = UINT32_C (0x00000000);
    area->length = UINT64_C (0x40000000);
    area->width = bp->lbc_num_d;

    return URJ_STATUS_OK;
}

static void
setup_address (urj_bus_t *bus, uint32_t a)
{
    int i;
    urj_part_t *p = bus->part;

    for (i = 0; i < A_WIDTH ; i++)
        urj_part_set_signal (p, LA[A_WIDTH - 1 - i], 1, (a >> i) & 1);
}

static void
set_data_in (urj_bus_t *bus, uint32_t adr)
{
    bus_params_t *bp = bus->params;
    urj_part_t *p = bus->part;
    urj_bus_area_t area;
    int i;

    mpc8313_bus_area (bus, adr, &area);
    if (area.width > bp->lbc_num_d)
        return;

    for (i = 0; i < area.width; i++)
        urj_part_set_signal_input (p, LAD[i]);

}

static void
setup_data (urj_bus_t *bus, uint32_t adr, uint32_t d)
{
    int i;
    urj_part_t *p = bus->part;

    for (i = 0; i < BUS_WIDTH; i++)
        urj_part_set_signal (p, LAD[i], 1,
                             (d >> ((REVBITS == 1) ? BUS_WIDTH - 1 - i : i)) &
                             1);
}

static uint32_t
get_data (urj_bus_t *bus, uint32_t adr)
{
    bus_params_t *bp = bus->params;
    urj_part_t *p = bus->part;
    urj_bus_area_t area;
    uint32_t d = 0;
    int i;

    mpc8313_bus_area (bus, adr, &area);

    for (i = 0; i <= bp->lbc_num_d; i++)
        d |= (uint32_t) (urj_part_get_signal (p, LAD[i]) <<
                         (REVBITS ? BUS_WIDTH - 1 - i : i));

    return d;
}

/**
 * bus->driver->(*read_start)
 *
 */
static int
mpc8313_bus_read_start (urj_bus_t *bus, uint32_t adr)
{
    urj_part_t *p = bus->part;

    LAST_ADR = adr;

    /* see Figure 6-45 in [1] */
    urj_part_set_signal_low (p, nCS);
    urj_part_set_signal_high (p, nWE);
    urj_part_set_signal_low (p, nFOE);

    setup_address (bus, adr);
    set_data_in (bus, adr);

    urj_tap_chain_shift_data_registers (bus->chain, 0);

    return URJ_STATUS_OK;
}

/**
 * bus->driver->(*read_next)
 *
 */
static uint32_t
mpc8313_bus_read_next (urj_bus_t *bus, uint32_t adr)
{
    uint32_t d;

    setup_address (bus, adr);
    urj_tap_chain_shift_data_registers (bus->chain, 1);

    d = get_data (bus, LAST_ADR);
    LAST_ADR = adr;
    return d;
}

/**
 * bus->driver->(*read_end)
 *
 */
static uint32_t
mpc8313_bus_read_end (urj_bus_t *bus)
{
    urj_part_t *p = bus->part;

    urj_part_set_signal_high (p, nCS);
    urj_part_set_signal_high (p, nFOE);

    urj_tap_chain_shift_data_registers (bus->chain, 1);

    return get_data (bus, LAST_ADR);
}

/**
 * bus->driver->(*write)
 *
 */
static void
mpc8313_bus_write (urj_bus_t *bus, uint32_t adr, uint32_t data)
{
    urj_part_t *p = bus->part;

    LAST_ADR = adr;


    /* see Figure 6-47 in [1] */
    urj_part_set_signal_low (p, nCS);
    urj_part_set_signal_high (p, nWE);
    urj_part_set_signal_high (p, nFOE);

    if (nWP != NULL)
        urj_part_set_signal_low (p, nWP);

    setup_address (bus, adr);

    setup_data (bus, adr, data);

    urj_tap_chain_shift_data_registers (bus->chain, 0);

    urj_part_set_signal_low (p, nWE);
    urj_tap_chain_shift_data_registers (bus->chain, 0);
    urj_part_set_signal_high (p, nWE);
    urj_part_set_signal_high (p, nCS);
    urj_tap_chain_shift_data_registers (bus->chain, 0);
}

const urj_bus_driver_t urj_bus_mpc8313_bus = {
    "mpc8313",
    N_("Motorola MPC8313 compatible bus driver via BSR"),
    mpc8313_bus_new,
    urj_bus_generic_free,
    mpc8313_bus_printinfo,
    urj_bus_generic_prepare_extest,
    mpc8313_bus_area,
    mpc8313_bus_read_start,
    mpc8313_bus_read_next,
    mpc8313_bus_read_end,
    urj_bus_generic_read,
    urj_bus_generic_write_start,
    mpc8313_bus_write,
    urj_bus_generic_no_init,
    urj_bus_generic_no_enable,
    urj_bus_generic_no_disable,
    URJ_BUS_TYPE_PARALLEL,
};
