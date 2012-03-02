/*
 * Freescale MPC837X compatible bus driver via BSR
 * Copyright (C) 2010 Andrzej Jalowiecki
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
 *
 * Documentation:
 * [1] Freescale, "Freescale MPC837x Users Guide"
 *     Order Number: MPC8379UG
 *
 */

#include "sysdep.h"

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <urjtag/chain.h>
#include <urjtag/part.h>
#include <urjtag/bus.h>
#include <urjtag/bssignal.h>

#include "buses.h"
#include "generic_bus.h"

#define LBC_NUM_LCS 4
#define LBC_NUM_LWE 4
#define LBC_NUM_LAD 32

typedef struct {
    uint32_t last_adr;
    urj_part_signal_t *nlcs[LBC_NUM_LCS];
    urj_part_signal_t *lad[LBC_NUM_LAD];
    urj_part_signal_t *la[LBC_NUM_LAD];
    urj_part_signal_t *nlwe[LBC_NUM_LWE];
    urj_part_signal_t *nloe;
    urj_part_signal_t *ale;
    urj_part_signal_t *lbctl;
    int lbc_muxed;
    int lbc_num_ad;
    int lbc_num_d;
} bus_params_t;

#define LAST_ADR ((bus_params_t *) bus->params)->last_adr /* Last used address */
#define nCS      ((bus_params_t *) bus->params)->nlcs     /* Chipselect# */
#define nWE      ((bus_params_t *) bus->params)->nlwe     /* Write enable# */
#define nOE      ((bus_params_t *) bus->params)->nloe     /* Output enable# */
#define ALE      ((bus_params_t *) bus->params)->ale      /* Addres strobe */
#define BCTL     ((bus_params_t *) bus->params)->lbctl    /* Write /Read# */

#define LAD      ((bus_params_t *) bus->params)->lad      /* Addres/Data Bus Mux */
#define LA       ((bus_params_t *) bus->params)->la       /* Addres Bus nonMux */

/**
 * bus->driver->(*new_bus)
 *
 */
static urj_bus_t *
mpc837x_bus_new (urj_chain_t *chain, const urj_bus_driver_t *driver,
                 const urj_param_t *cmd_params[])
{
    urj_bus_t *bus;
    bus_params_t *bp;
    urj_part_t *part;
    char buff[10];
    int i;
    int failed = 0;

    bus = urj_bus_generic_new (chain, driver, sizeof (bus_params_t));
    if (bus == NULL)
        return NULL;

    part = bus->part;
    bp = bus->params;

    /* default values */
    bp->lbc_muxed = 0;
    bp->lbc_num_d = 8;
    bp->lbc_num_ad = 25;

    for (i = 0; cmd_params[i] != NULL; i++)
    {
        switch (cmd_params[i]->key)
        {
        case URJ_BUS_PARAM_KEY_HELP:
            urj_bus_generic_free (bus);
            urj_log (URJ_LOG_LEVEL_NORMAL,
                     _("Usage: initbus mpc837x [mux] [width=WIDTH]\n"
                       "            MUX        multiplexed data bus (default no)\n"
                       "            WIDTH      data bus width - 8, 16, 32 (default 8)\n"));
            return NULL;
        case URJ_BUS_PARAM_KEY_MUX:
            bp->lbc_muxed = 1;
            break;
        case URJ_BUS_PARAM_KEY_WIDTH:
            switch (cmd_params[i]->value.lu)
            {
            case 8:
                bp->lbc_num_d = 8;
                break;
            case 16:
                bp->lbc_num_d = 16;
                break;
            case 32:
                bp->lbc_num_d = 32;
                break;
            default:
                urj_error_set (URJ_ERROR_UNSUPPORTED,
                               _("    Only 8, 16, 32 bus width are suported\n"));
            }
            break;
        default:
            urj_bus_generic_free (bus);
            urj_error_set (URJ_ERROR_SYNTAX, "unrecognised bus parameter '%s'",
            urj_param_string (&urj_bus_param_list, cmd_params[i]));
            return NULL;
        }
    }

    if (!bp->lbc_muxed && (bp->lbc_num_d > 16))
    {
        urj_bus_generic_free (bus);
        urj_error_set (URJ_ERROR_UNSUPPORTED,
                       _("    Only 8 and 16 non multiplexed bus width are suported\n"));
        return NULL;
    }

    if (bp->lbc_muxed)
        bp->lbc_num_ad = 32;

    /* Get the signals */
    if (bp->lbc_muxed)
    {
        failed |= urj_bus_generic_attach_sig (part, &(ALE), "LALE");
        for (i = 0; i < LBC_NUM_LAD; i++)
        {
            sprintf (buff, "LAD%d", i);
            failed |= urj_bus_generic_attach_sig (part, &(LAD[i]), buff);
        }
    }
    else
    {
        failed |= urj_bus_generic_attach_sig (part, &(LA[7]),  "LDP2");
        failed |= urj_bus_generic_attach_sig (part, &(LA[8]),  "LDP3");
        failed |= urj_bus_generic_attach_sig (part, &(LA[9]),  "LGPL5");
        failed |= urj_bus_generic_attach_sig (part, &(LA[10]), "LALE");
        for (i = 11; i < 27; i++)
        {
            sprintf (buff, "LAD%d", i + 5);
            failed |= urj_bus_generic_attach_sig (part, &(LA[i]), buff);
        }
    }

    for (i = 27; i < LBC_NUM_LAD; i++)
    {
        sprintf (buff, "LA%d", i);
        failed |= urj_bus_generic_attach_sig (part, &(LA[i]), buff);
    }

    for (i = 0; i < LBC_NUM_LCS; i++)
    {
        sprintf (buff, "LCS_B%d", i);
        failed |= urj_bus_generic_attach_sig (part, &(nCS[i]), buff);
    }

    for (i = 0; i < LBC_NUM_LWE; i++)
    {
        sprintf (buff, "LWE_B%d", i);
        failed |= urj_bus_generic_attach_sig (part, &(nWE[i]), buff);
    }

    failed |= urj_bus_generic_attach_sig (part, &(nOE), "LGPL2");
    failed |= urj_bus_generic_attach_sig (part, &(BCTL), "LBCTL");

    if (failed)
    {
        urj_bus_generic_free (bus);
        return NULL;
    }

    urj_log (URJ_LOG_LEVEL_NORMAL,
             "%sMUXed %db address, %db data bus\n",
             ((bp->lbc_muxed) ? "" : "Non-"),
             bp->lbc_num_ad, bp->lbc_num_d);

    return bus;
}

/**
 * bus->driver->(*printinfo)
 *
 */
static void
mpc837x_bus_printinfo (urj_log_level_t ll, urj_bus_t *bus)
{
    int i;

    for (i = 0; i < bus->chain->parts->len; i++)
        if (bus->part == bus->chain->parts->parts[i])
            break;
    urj_log (ll, _("Freescale MPC837X compatible bus driver via BSR (JTAG part No. %d)\n"), i);
}

/**
 * bus->driver->(*area)
 *
 */
static int
mpc837x_bus_area (urj_bus_t *bus, uint32_t adr, urj_bus_area_t *area)
{
    bus_params_t *bp = bus->params;

    area->description = N_("Local Bus Controller");
    area->start = UINT32_C(0x00000000);
    area->length = UINT64_C(0x100000000);
    area->width = bp->lbc_num_d;
    return URJ_STATUS_OK;
}

static void
setup_address (urj_bus_t *bus, uint32_t a)
{
    bus_params_t *bp = bus->params;
    urj_part_t *p = bus->part;
    int i;

    if (bp->lbc_muxed)
    {
        for (i = 0; i < bp->lbc_num_ad; i++)
            urj_part_set_signal (p, LAD[LBC_NUM_LAD - i - 1], 1, (a >> i) & 1);

        for (i = 0; i < 5; i++)
            urj_part_set_signal (p, LA[LBC_NUM_LAD - i - 1], 1, (a >> i) & 1);
    }
    else
    {
        for (i = 0; i < bp->lbc_num_ad; i++)
            urj_part_set_signal (p, LA[LBC_NUM_LAD - i - 1], 1, (a >> i) & 1);
    }
}

static void
set_data_in (urj_bus_t *bus, uint32_t adr)
{
    bus_params_t *bp = bus->params;
    urj_part_t *p = bus->part;
    urj_bus_area_t area;
    int i;

    mpc837x_bus_area (bus, adr, &area);
    if (area.width > bp->lbc_num_d)
        return;

    for (i = 0; i < area.width; i++)
        urj_part_set_signal_input (p, LAD[bp->lbc_num_d - i - 1]);
}

static void
setup_data (urj_bus_t *bus, uint32_t adr, uint32_t d)
{
    bus_params_t *bp = bus->params;
    urj_part_t *p = bus->part;
    urj_bus_area_t area;
    int i;

    mpc837x_bus_area (bus, adr, &area);
    if (area.width > bp->lbc_num_d)
        return;

    for (i = 0; i < area.width; i++)
        urj_part_set_signal (p, LAD[bp->lbc_num_d - i - 1], 1, (d >> i) & 1);
}

static uint32_t
get_data (urj_bus_t *bus, uint32_t adr)
{
    bus_params_t *bp = bus->params;
    urj_part_t *p = bus->part;
    urj_bus_area_t area;
    uint32_t d = 0;
    int i;

    mpc837x_bus_area (bus, adr, &area);
    if (area.width > bp->lbc_num_d)
        return 0;

    for (i = 0; i < area.width; i++)
        d |= (urj_part_get_signal (p, LAD[bp->lbc_num_d - i - 1]) << i);
    return d;
}

/**
 * bus->driver->(*read_start)
 *
 */
static int
mpc837x_bus_read_start (urj_bus_t *bus, uint32_t adr)
{
    bus_params_t *bp = bus->params;
    urj_chain_t *chain = bus->chain;
    urj_part_t *p = bus->part;
    uint8_t cs;
    int i;

    LAST_ADR = adr;
    cs = 0;

    for (i = 0; i < LBC_NUM_LCS; i++)
        urj_part_set_signal (p, nCS[i], 1, !(cs == i));

    for (i = 0; i < LBC_NUM_LWE; i++)
        urj_part_set_signal_high (p, nWE[i]);

    setup_address (bus, adr);

    if (bp->lbc_muxed)
    {
        urj_part_set_signal_high (p, BCTL);    /* Address Out */
        urj_part_set_signal_high (p, ALE);
        urj_part_set_signal_high (p, nOE);
        urj_tap_chain_shift_data_registers (chain, 0);
        urj_part_set_signal_low (p, BCTL);    /* Data In */
        urj_part_set_signal_low (p, ALE);
        urj_part_set_signal_low (p, nOE);
    }
    else
    {
        urj_part_set_signal_low (p, BCTL);    /* Data In */
        urj_part_set_signal_low (p, nOE);
        set_data_in (bus, adr);
    }

    urj_tap_chain_shift_data_registers (chain, 0);

    return URJ_STATUS_OK;
}

/**
 * bus->driver->(*read_next)
 *
 */
static uint32_t
mpc837x_bus_read_next (urj_bus_t *bus, uint32_t adr)
{
    bus_params_t *bp = bus->params;
    urj_chain_t *chain = bus->chain;
    urj_part_t *p = bus->part;
    uint32_t d;

    if (bp->lbc_muxed)
    {
        set_data_in (bus, adr);
        urj_tap_chain_shift_data_registers (chain, 0);

        urj_tap_chain_shift_data_registers (chain, 1);
        d = get_data (bus, LAST_ADR);

        setup_address (bus, adr);
        LAST_ADR = adr;

        urj_part_set_signal_high (p, BCTL);    /* Address Out */
        urj_part_set_signal_high (p, ALE);
        urj_part_set_signal_high (p, nOE);
        urj_tap_chain_shift_data_registers (chain, 0);

        urj_part_set_signal_low (p, BCTL);    /* Data In */
        urj_part_set_signal_low (p, ALE);
        urj_part_set_signal_low (p, nOE);
        urj_tap_chain_shift_data_registers (chain, 0);
    }
    else
    {
        setup_address (bus, adr);    /* Data In */
        urj_tap_chain_shift_data_registers (chain, 1);
        d = get_data (bus, LAST_ADR);
    }

    LAST_ADR = adr;
    return d;
}

/**
 * bus->driver->(*read_end)
 *
 */
static uint32_t
mpc837x_bus_read_end (urj_bus_t *bus)
{
    bus_params_t *bp = bus->params;
    urj_chain_t *chain = bus->chain;
    urj_part_t *p = bus->part;
    int i;

    if (bp->lbc_muxed)
    {
        set_data_in (bus, LAST_ADR);
        urj_tap_chain_shift_data_registers (chain, 0);
        urj_part_set_signal_high (p, ALE);
    }

    for (i = 0; i < LBC_NUM_LCS; i++)
        urj_part_set_signal_high (p, nCS[i]);

    urj_part_set_signal_high (p, BCTL);
    urj_part_set_signal_high (p, nOE);

    urj_tap_chain_shift_data_registers (chain, 1);

    return get_data (bus, LAST_ADR);
}

/**
 * bus->driver->(*write)
 *
 */
static void
mpc837x_bus_write (urj_bus_t *bus, uint32_t adr, uint32_t data)
{
    bus_params_t *bp =  bus->params;
    urj_chain_t *chain = bus->chain;
    urj_part_t *p = bus->part;
    urj_bus_area_t area;
    uint8_t cs;
    int i;

    mpc837x_bus_area (bus, adr, &area);
    if (area.width > bp->lbc_num_d)
        return;

    cs = 0;
    urj_part_set_signal_high (p, BCTL);
    urj_part_set_signal_high (p, nOE);

    for (i = 0; i < LBC_NUM_LWE; i++)
        urj_part_set_signal_high (p, nWE[i]);

    if (bp->lbc_muxed)
    {
        setup_address (bus, adr);
        urj_part_set_signal_high (p, ALE);
        urj_tap_chain_shift_data_registers (chain, 0);
        urj_part_set_signal_low (p, ALE);
        urj_tap_chain_shift_data_registers (chain, 0);
    }
    else
        setup_address (bus, adr);

    for (i = 0; i < LBC_NUM_LCS; i++)
        urj_part_set_signal (p, nCS[i], 1, !(cs == i));

    setup_data (bus, adr, data);

    urj_tap_chain_shift_data_registers (chain, 0);

    switch (area.width)
    {
    case 32:
        urj_part_set_signal_low (p, nWE[3]);
        urj_part_set_signal_low (p, nWE[2]);
    case 16:
        urj_part_set_signal_low (p, nWE[1]);
    case 8:
        urj_part_set_signal_low (p, nWE[0]);
    default:
        break;
    }

    urj_tap_chain_shift_data_registers (chain, 0);

    for (i = 0; i < LBC_NUM_LWE; i++)
        urj_part_set_signal_high (p, nWE[i]);

    urj_tap_chain_shift_data_registers (chain, 0);
}

const urj_bus_driver_t urj_bus_mpc837x_bus = {
    "mpc837x",
    N_("Freescale MPC837x compatible bus driver via BSR, parameter: [mux] [width]"),
    mpc837x_bus_new,
    urj_bus_generic_free,
    mpc837x_bus_printinfo,
    urj_bus_generic_prepare_extest,
    mpc837x_bus_area,
    mpc837x_bus_read_start,
    mpc837x_bus_read_next,
    mpc837x_bus_read_end,
    urj_bus_generic_read,
    urj_bus_generic_write_start,
    mpc837x_bus_write,
    urj_bus_generic_no_init,
    urj_bus_generic_no_enable,
    urj_bus_generic_no_disable,
    URJ_BUS_TYPE_PARALLEL,
};
