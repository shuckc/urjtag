/*
 * $Id$
 *
 * Freescale MPC5200 compatible bus driver via BSR
 * Copyright (C) 2003 Marcel Telka
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
 * Written by Asier Llano <a.llano@usyscom.com>, 2004.
 *
 * Documentation:
 * [1] Freescale, "Freescale MPC5200 Users Guide", Rev. 2, 08/2004
 *     Order Number: MPC5200UG
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

#define LPC_NUM_CS      6
#define LPC_NUM_AD      32
#define LPC_ADDR_TO_CS(a) ((a) >> bp->lpc_num_ad)
#define LPC_ADDR_SIZE   (((long unsigned long) 1 << bp->lpc_num_ad) * LPC_NUM_CS)

typedef struct
{
    uint32_t last_adr;
    urj_part_signal_t *ad[LPC_NUM_AD];
    urj_part_signal_t *ncs[LPC_NUM_CS];
    urj_part_signal_t *nwe;
    urj_part_signal_t *noe;
    urj_part_signal_t *ata_iso;
    urj_part_signal_t *nale;
    int muxed;
    int lpc_num_ad;
    int lpc_num_d;
} bus_params_t;

#define LAST_ADR        ((bus_params_t *) bus->params)->last_adr
#define AD              ((bus_params_t *) bus->params)->ad
#define nCS             ((bus_params_t *) bus->params)->ncs
#define nWE             ((bus_params_t *) bus->params)->nwe
#define nOE             ((bus_params_t *) bus->params)->noe
#define nALE            ((bus_params_t *) bus->params)->nale
#define ATA_ISO         ((bus_params_t *) bus->params)->ata_iso

/**
 * bus->driver->(*new_bus)
 *
 */
static urj_bus_t *
mpc5200_bus_new (urj_chain_t *chain, const urj_bus_driver_t *driver,
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

    bp->lpc_num_ad = 24;
    bp->lpc_num_d = 8;

    for (i = 0; cmd_params[i] != NULL; i++)
    {
        switch (cmd_params[i]->key)
        {
        case URJ_BUS_PARAM_KEY_MUX:
            bp->lpc_num_ad = 25;
            bp->lpc_num_d = 16;
            bp->muxed = 1;
            break;
        default:
            urj_bus_generic_free (bus);
            urj_error_set (URJ_ERROR_SYNTAX, "unrecognised bus parameter '%s'",
                           urj_param_string(&urj_bus_param_list, cmd_params[i]));
            return NULL;
        }
    }
    urj_log (URJ_LOG_LEVEL_NORMAL,
             "%sMUXed %db address, %db data bus\n", (bp->muxed ? "" : "Non-"),
            bp->lpc_num_ad, bp->lpc_num_d);

    /* Get the signals */
    for (i = 0; i < LPC_NUM_AD; i++)
    {
        sprintf (buff, "EXT_AD_%d", i);
        failed |= urj_bus_generic_attach_sig (part, &(AD[i]), buff);
    }

    for (i = 0; i < LPC_NUM_CS; i++)
    {
        sprintf (buff, "LP_CS%d_B", i);
        failed |= urj_bus_generic_attach_sig (part, &(nCS[i]), buff);
    }

    failed |= urj_bus_generic_attach_sig (part, &(nWE), "LP_RW");

    failed |= urj_bus_generic_attach_sig (part, &(nOE), "LP_OE");

    failed |= urj_bus_generic_attach_sig (part, &(nALE), "LP_ALE_B");

    failed |= urj_bus_generic_attach_sig (part, &(ATA_ISO), "ATA_ISOLATION");

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
mpc5200_bus_printinfo (urj_log_level_t ll, urj_bus_t *bus)
{
    int i;

    for (i = 0; i < bus->chain->parts->len; i++)
        if (bus->part == bus->chain->parts->parts[i])
            break;
    urj_log (ll, _("Freescale MPC5200 compatible bus driver via BSR (JTAG part No. %d)\n"),
            i);
}

/**
 * bus->driver->(*area)
 *
 */
static int
mpc5200_bus_area (urj_bus_t *bus, uint32_t adr, urj_bus_area_t *area)
{
    bus_params_t *bp = (bus_params_t *) bus->params;

    if (adr < LPC_ADDR_SIZE)
    {
        area->description = N_("LocalPlus Bus");
        area->start = UINT32_C (0x00000000);
        area->length = LPC_ADDR_SIZE;
        area->width = bp->lpc_num_d;
        return URJ_STATUS_OK;
    }

    area->description = NULL;
    area->start = LPC_ADDR_SIZE;
    area->length = UINT64_C (0x100000000) - LPC_ADDR_SIZE;
    area->width = 0;
    return URJ_STATUS_OK;
}

static void
setup_address (urj_bus_t *bus, uint32_t a)
{
    bus_params_t *bp = (bus_params_t *) bus->params;
    urj_part_t *p = bus->part;
    int i;

    for (i = 0; i < bp->lpc_num_ad; i++)
        urj_part_set_signal (p, AD[i], 1, (a >> i) & 1);
}

static void
set_data_in (urj_bus_t *bus, uint32_t adr)
{
    bus_params_t *bp = (bus_params_t *) bus->params;
    urj_part_t *p = bus->part;
    urj_bus_area_t area;
    int i;

    mpc5200_bus_area (bus, adr, &area);
    if (area.width > bp->lpc_num_d)
        return;

    for (i = 0; i < area.width; i++)
        urj_part_set_signal_input (p, AD[i + (LPC_NUM_AD - bp->lpc_num_d)]);
}

static void
setup_data (urj_bus_t *bus, uint32_t adr, uint32_t d)
{
    bus_params_t *bp = (bus_params_t *) bus->params;
    urj_part_t *p = bus->part;
    urj_bus_area_t area;
    int i;

    mpc5200_bus_area (bus, adr, &area);
    if (area.width > bp->lpc_num_d)
        return;

    for (i = 0; i < area.width; i++)
        urj_part_set_signal (p, AD[i + (LPC_NUM_AD - bp->lpc_num_d)], 1,
                             (d >> i) & 1);
}

static uint32_t
get_data (urj_bus_t *bus, uint32_t adr)
{
    bus_params_t *bp = (bus_params_t *) bus->params;
    urj_bus_area_t area;
    uint32_t d = 0;
    urj_part_t *p = bus->part;
    int i;

    mpc5200_bus_area (bus, adr, &area);
    if (area.width > bp->lpc_num_d)
        return 0;

    for (i = 0; i < area.width; i++)
    {
        d |= (uint32_t) (urj_part_get_signal
                         (p, AD[i + (LPC_NUM_AD - bp->lpc_num_d)]) << i);
    }
    return d;
}

/**
 * bus->driver->(*read_start)
 *
 */
static int
mpc5200_bus_read_start (urj_bus_t *bus, uint32_t adr)
{
    bus_params_t *bp = (bus_params_t *) bus->params;
    urj_part_t *p = bus->part;
    uint8_t cs = LPC_ADDR_TO_CS (adr);
    int i;

    LAST_ADR = adr;

    /* see Figure 6-45 in [1] */

    for (i = 0; i < LPC_NUM_CS; i++)
    {
        urj_part_set_signal (p, nCS[i], 1, !(cs == i));
    }

    urj_part_set_signal_high (p, ATA_ISO);
    urj_part_set_signal_high (p, nWE);
    urj_part_set_signal_low (p, nOE);

    setup_address (bus, adr);

    if (!bp->muxed)
        set_data_in (bus, adr);
    else
    {
        urj_part_set_signal_low (p, nALE);
        urj_tap_chain_shift_data_registers (bus->chain, 0);
        urj_part_set_signal_high (p, nALE);
    }
    urj_tap_chain_shift_data_registers (bus->chain, 0);

    return URJ_STATUS_OK;
}

/**
 * bus->driver->(*read_next)
 *
 */
static uint32_t
mpc5200_bus_read_next (urj_bus_t *bus, uint32_t adr)
{
    bus_params_t *bp = (bus_params_t *) bus->params;
    urj_part_t *p = bus->part;
    uint32_t d;

    if (!bp->muxed)
    {
        setup_address (bus, adr);
        urj_tap_chain_shift_data_registers (bus->chain, 1);
        d = get_data (bus, LAST_ADR);
    }
    else
    {
        set_data_in (bus, adr);
        urj_tap_chain_shift_data_registers (bus->chain, 0);

        urj_tap_chain_shift_data_registers (bus->chain, 1);
        d = get_data (bus, LAST_ADR);

        setup_address (bus, adr);
        LAST_ADR = adr;

        urj_part_set_signal_low (p, nALE);
        urj_tap_chain_shift_data_registers (bus->chain, 0);
        urj_part_set_signal_high (p, nALE);
        urj_tap_chain_shift_data_registers (bus->chain, 0);
    }

    LAST_ADR = adr;
    return d;
}

/**
 * bus->driver->(*read_end)
 *
 */
static uint32_t
mpc5200_bus_read_end (urj_bus_t *bus)
{
    bus_params_t *bp = (bus_params_t *) bus->params;
    urj_part_t *p = bus->part;
    int i;

    if (bp->muxed)
    {
        set_data_in (bus, LAST_ADR);
        urj_tap_chain_shift_data_registers (bus->chain, 0);
    }
    for (i = 0; i < LPC_NUM_CS; i++)
    {
        urj_part_set_signal_high (p, nCS[i]);
    }
    urj_part_set_signal_high (p, nOE);

    urj_tap_chain_shift_data_registers (bus->chain, 1);

    return get_data (bus, LAST_ADR);
}

/**
 * bus->driver->(*write)
 *
 */
static void
mpc5200_bus_write (urj_bus_t *bus, uint32_t adr, uint32_t data)
{
    bus_params_t *bp = (bus_params_t *) bus->params;
    /* see Figure 6-47 in [1] */
    urj_part_t *p = bus->part;
    urj_chain_t *chain = bus->chain;
    uint8_t cs = LPC_ADDR_TO_CS (adr);
    int i;

    if (bp->muxed)
    {
        setup_address (bus, adr);
        urj_part_set_signal_low (p, nALE);
        urj_tap_chain_shift_data_registers (chain, 0);
        urj_part_set_signal_high (p, nALE);
        urj_tap_chain_shift_data_registers (chain, 0);
    }

    for (i = 0; i < LPC_NUM_CS; i++)
    {
        urj_part_set_signal (p, nCS[i], 1, !(cs == i));
    }
    urj_part_set_signal_high (p, ATA_ISO);
    urj_part_set_signal_high (p, nWE);
    urj_part_set_signal_high (p, nOE);

    if (!bp->muxed)
        setup_address (bus, adr);
    setup_data (bus, adr, data);

    urj_tap_chain_shift_data_registers (chain, 0);

    urj_part_set_signal_low (p, nWE);
    urj_tap_chain_shift_data_registers (chain, 0);
    urj_part_set_signal_high (p, nWE);
    urj_tap_chain_shift_data_registers (chain, 0);
}

const urj_bus_driver_t urj_bus_mpc5200_bus = {
    "mpc5200",
    N_("Freescale MPC5200 compatible bus driver via BSR, parameter: [mux]"),
    mpc5200_bus_new,
    urj_bus_generic_free,
    mpc5200_bus_printinfo,
    urj_bus_generic_prepare_extest,
    mpc5200_bus_area,
    mpc5200_bus_read_start,
    mpc5200_bus_read_next,
    mpc5200_bus_read_end,
    urj_bus_generic_read,
    urj_bus_generic_write_start,
    mpc5200_bus_write,
    urj_bus_generic_no_init,
    urj_bus_generic_no_enable,
    urj_bus_generic_no_disable,
    URJ_BUS_TYPE_PARALLEL,
};
