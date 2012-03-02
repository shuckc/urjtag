/*
 * $Id$
 *
 * Motorola MPC824x compatible bus driver via BSR
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
 * Written by Marcel Telka <marcel@telka.sk>, 2003.
 *
 * Documentation:
 * [1] Motorola, Inc., "MPC8245 Integrated Processor User's Manual",
 *     MPC8245UM/D, 10/2001, Rev. 1
 *
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

typedef struct
{
    int boot_nfoe;
    int boot_sdma1;
    uint32_t last_adr;
    urj_part_signal_t *ar[23];
    urj_part_signal_t *nrcs0;
    urj_part_signal_t *nwe;
    urj_part_signal_t *nfoe;
    urj_part_signal_t *d[32];
    int bus_width;
    char revbits, dbg_addr, dbg_data;
} bus_params_t;

#define boot_nFOE       ((bus_params_t *) bus->params)->boot_nfoe
#define boot_SDMA1      ((bus_params_t *) bus->params)->boot_sdma1
#define LAST_ADR        ((bus_params_t *) bus->params)->last_adr
#define AR              ((bus_params_t *) bus->params)->ar
#define nRCS0           ((bus_params_t *) bus->params)->nrcs0
#define nWE             ((bus_params_t *) bus->params)->nwe
#define nFOE            ((bus_params_t *) bus->params)->nfoe
#define D               ((bus_params_t *) bus->params)->d
#define BUS_WIDTH       ((bus_params_t *) bus->params)->bus_width
#define REVBITS         ((bus_params_t *) bus->params)->revbits
#define dbgAddr         ((bus_params_t *) bus->params)->dbg_addr
#define dbgData         ((bus_params_t *) bus->params)->dbg_data

/**
 * bus->driver->(*new_bus)
 *
 */
static urj_bus_t *
mpc824x_bus_new (urj_chain_t *chain, const urj_bus_driver_t *driver,
                 const urj_param_t *cmd_params[])
{
    urj_bus_t *bus;
    urj_part_t *part;
    char buff[10];
    int i;
    int failed = 0;
    urj_part_signal_t *s_nfoe;
    urj_part_signal_t *s_sdma1;
    int bus_width = 8;
    char dfltWidth = 1;
    char revbits = 0, dbg_addr = 0, dbg_data = 0;

    for (i = 0; cmd_params[i] != NULL; i++)
    {
        switch (cmd_params[i]->key)
        {
        case URJ_BUS_PARAM_KEY_WIDTH:
            switch (cmd_params[i]->value.lu)
            {
            case 8:
                bus_width = 8;
                dfltWidth = 0;
                break;
            case 16:
                bus_width = 16;
                dfltWidth = 0;
                break;
            case 32:
                bus_width = 32;
                dfltWidth = 0;
                break;
            case 64:
                //              bus_width = 64;  // Needs to fix, look at setup_data()
                bus_width = 32;
                urj_error_set (URJ_ERROR_UNSUPPORTED,
                               _("   Bus width 64 exists in mpc824x, but not supported by UrJTAG currently"));
                dfltWidth = 1;
                break;
            default:
                urj_error_set (URJ_ERROR_UNSUPPORTED,
                               _("   Only 8, 16, 32 and 64 bus width are supported for Banks 0 and 1"));
                return NULL;
            }
            break;

        case URJ_BUS_PARAM_KEY_REVBITS:
            revbits = 1;
            break;

        // @@@@ RFHH ToDo: lift this from init_bus
        case URJ_BUS_PARAM_KEY_HELP:
            urj_log (URJ_LOG_LEVEL_NORMAL,
                     _("Usage: initbus mpc824x [width=WIDTH] [revbits] [dbgAddr] [dbgData]\n\n"
                     "   WIDTH      data bus width - 8, 16, 32, 64 (default 8)\n"
                     "   revbits    reverse bits in data bus (default - no)\n"
                     "   dbgAddr    display address bus state (default - no)\n"
                     "   dbgData    display data bus state (default - no)\n"));
            return NULL;

        // @@@@ RFHH ToDo: lift this from init_bus
        case URJ_BUS_PARAM_KEY_DBGaDDR:
            dbg_addr = 1;
            break;

        // @@@@ RFHH ToDo: lift this from init_bus
        case URJ_BUS_PARAM_KEY_DBGdATA:
            dbg_data = 1;
            break;

        default:
            urj_error_set (URJ_ERROR_SYNTAX, "unrecognised bus parameter '%s'",
                           urj_param_string(&urj_bus_param_list, cmd_params[i]));
            return NULL;
        }

    }
    if (dfltWidth)
        urj_log (URJ_LOG_LEVEL_NORMAL,
                 _("   Using default bus width %d\n"), bus_width);

    //      REVBITS = 0;

    bus = urj_bus_generic_new (chain, driver, sizeof (bus_params_t));
    if (bus == NULL)
        return NULL;
    part = bus->part;

    BUS_WIDTH = bus_width;
    REVBITS = revbits;
    dbgAddr = dbg_addr;
    dbgData = dbg_data;

    s_nfoe = urj_part_find_signal (part, "nFOE");
    s_sdma1 = urj_part_find_signal (part, "SDMA1");
    urj_part_set_signal_input (part, s_nfoe);
    urj_part_set_signal_input (part, s_sdma1);

    urj_part_set_instruction (part, "SAMPLE/PRELOAD");
    urj_tap_chain_shift_instructions (chain);
    urj_tap_chain_shift_data_registers (chain, 0);
    urj_part_set_instruction (part, "EXTEST");
    urj_tap_chain_shift_instructions (chain);
    urj_tap_chain_shift_data_registers (chain, 1);

    boot_nFOE = urj_part_get_signal (part, s_nfoe);
    boot_SDMA1 = urj_part_get_signal (part, s_sdma1);


    for (i = 0; i <= 10; i++)
    {
        sprintf (buff, "SDMA%d", i);
        failed |= urj_bus_generic_attach_sig (part, &(AR[i]), buff);
    }

    failed |= urj_bus_generic_attach_sig (part, &(AR[11]), "SDBA0");

    for (i = 0; i < 8; i++)
    {
        sprintf (buff, "PAR%d", i);
        failed |= urj_bus_generic_attach_sig (part, &(AR[19 - i]), buff);
    }

    failed |= urj_bus_generic_attach_sig (part, &(AR[20]), "SDBA1");

    failed |= urj_bus_generic_attach_sig (part, &(AR[21]), "SDMA11");

    failed |= urj_bus_generic_attach_sig (part, &(AR[22]), "SDMA12");

    failed |= urj_bus_generic_attach_sig (part, &(nRCS0), "nRCS0");

    failed |= urj_bus_generic_attach_sig (part, &(nWE), "nWE");

    failed |= urj_bus_generic_attach_sig (part, &(nFOE), "nFOE");

    /*
       Freescale MPC824x uses inversed bit order ([1], p. 2-18):
       msb is MDH[0] while lsb is MDH[31]
       Flash chips usually use another bit orded:
       msb is D[31] and lsb is D[0]

       This should be rewired in the PCB (MDH[0] - D[31], ..., MDH[31] - D[0]).
       Otherwise you will have to use "revbits" UrJTAG parameter and
       binary files with reversed bit order.
     */

    for (i = 0; i < 32; i++)
    {                           /* Needs to be fixed for 64-bit bus width */
        sprintf (buff, "MDH%d", 31 - i);
        failed |= urj_bus_generic_attach_sig (part, &(D[i]), buff);
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
mpc824x_bus_printinfo (urj_log_level_t ll, urj_bus_t *bus)
{
    int i;

    for (i = 0; i < bus->chain->parts->len; i++)
        if (bus->part == bus->chain->parts->parts[i])
            break;
    urj_log (ll, _("Motorola MPC824x compatible bus driver via BSR (JTAG part No. %d)\n"),
            i);
}

/**
 * bus->driver->(*area)
 *
 */
static int
mpc824x_bus_area (urj_bus_t *bus, uint32_t adr, urj_bus_area_t *area)
{

    if (adr < UINT32_C (0xFF000000))
    {
        area->description = NULL;
        area->start = UINT32_C (0x00000000);
        area->length = UINT64_C (0xFF000000);
        area->width = 0;

        return URJ_STATUS_OK;
    }

    if (adr < UINT32_C (0xFF800000))
    {
        area->description = N_("Base ROM Interface (Bank 1)");
        area->start = UINT32_C (0xFF000000);
        area->length = UINT64_C (0x00800000);
        area->width = 0;

        return URJ_STATUS_OK;
    }

    if (boot_SDMA1 == 0)
    {
        area->description = N_("Base ROM Interface (Bank 0)");
        area->start = UINT32_C (0xFF800000);
        area->length = UINT64_C (0x00800000);
        area->width = BUS_WIDTH;

        return URJ_STATUS_OK;
    }

    /* extended addresing mode is disabled (SDMA1 is 1) */
    if (adr < UINT32_C (0xFFC00000))
    {
        area->description = NULL;
        area->start = UINT32_C (0xFF800000);
        area->length = UINT64_C (0x00400000);
        area->width = BUS_WIDTH;

        return URJ_STATUS_OK;
    }

    area->description = N_("Base ROM Interface (Bank 0)");
    area->start = UINT32_C (0xFFC00000);
    area->length = UINT64_C (0x00400000);
    area->width = BUS_WIDTH;

    return URJ_STATUS_OK;
}

static void
setup_address (urj_bus_t *bus, uint32_t a)
{
    int i;
    urj_part_t *p = bus->part;

    switch (BUS_WIDTH)
    {
    case 8:                    /* 8-bit data bus */
        for (i = 0; i < 23; i++)
            urj_part_set_signal (p, AR[i], 1, (a >> i) & 1);
        break;
    case 16:                   /* 16-bit data bus */
        for (i = 0; i < 22; i++)
            urj_part_set_signal (p, AR[i], 1, (a >> (i + 1)) & 1);
        break;
    case 32:                   /* 32-bit data bus */
        for (i = 0; i < 21; i++)
            urj_part_set_signal (p, AR[i], 1, (a >> (i + 2)) & 1);
        break;
    case 64:
        for (i = 0; i < 20; i++)
            urj_part_set_signal (p, AR[i], 1, (a >> (i + 3)) & 1);
        break;
    default:
        urj_error_set (URJ_ERROR_UNSUPPORTED,
                       _("Warning: unhandled bus width: %i"), BUS_WIDTH);
        return;
    }

    /* Just for debugging */
    if (dbgAddr)
    {
        int j, k;
        switch (BUS_WIDTH)
        {
        case 8:
            k = 23;
            break;
        case 16:
            k = 22;
            break;
        case 32:
            k = 21;
            break;
        case 64:
            k = 20;
            break;
        default:
            return;
        }

        urj_log (URJ_LOG_LEVEL_DEBUG, _("Addr    [%2d:0]: %06lX   "), k, (long unsigned) a);
        for (i = 0; i < 3; i++)
        {
            for (j = 0; j < 8; j++)
                if ((i * 8 + j) >= (23 - k))
                    urj_log (URJ_LOG_LEVEL_DEBUG, "%1lu",
                            (long unsigned) ((a >> (23 - (i * 8 + j))) & 1));
                else
                    urj_log (URJ_LOG_LEVEL_DEBUG, " ");
            urj_log (URJ_LOG_LEVEL_DEBUG, " ");
        }
        urj_log (URJ_LOG_LEVEL_DEBUG, "\n");
    }

}

static void
set_data_in (urj_bus_t *bus, uint32_t adr)
{
    int i;
    urj_part_t *p = bus->part;
    urj_bus_area_t area;

    mpc824x_bus_area (bus, adr, &area);
    if (area.width > 64)
        return;

    for (i = 0; i < area.width; i++)
        urj_part_set_signal_input (p, D[i]);
}

static void
setup_data (urj_bus_t *bus, uint32_t adr, uint32_t d)
{
    int i;
    urj_part_t *p = bus->part;
    urj_bus_area_t area;

    mpc824x_bus_area (bus, adr, &area);
    if (area.width > 64)
        return;

    for (i = 0; i < area.width; i++)
        urj_part_set_signal (p, D[i], 1,
                             (d >> ((REVBITS == 1) ? BUS_WIDTH - 1 - i : i)) &
                             1);

    /* Just for debugging */
    if (dbgData)
    {
        urj_log (URJ_LOG_LEVEL_DEBUG, _("Data WR [%d:0]: %08lX   "), area.width - 1,
                (long unsigned) d);
        int j;
        int bytes = 0;
        if (BUS_WIDTH == 8)
            bytes = 1;
        else if (BUS_WIDTH == 16)
            bytes = 2;
        else if (BUS_WIDTH == 32)
            bytes = 4;
        else if (BUS_WIDTH == 64)
            bytes = 4;          /* Needs to be fixed - d is 32-bit long, so no 64 bit mode is possible */

        for (i = 0; i < bytes; i++)
        {
            for (j = 0; j < 8; j++)
                if (REVBITS)
                    urj_log (URJ_LOG_LEVEL_DEBUG, "%1lu", (long unsigned)
                                    (d >> (BUS_WIDTH - 1 - (i * 8 + j))) & 1);
                else
                    urj_log (URJ_LOG_LEVEL_DEBUG, "%1lu", (long unsigned)
                                    (d >> ((i * 8 + j))) & 1);
            urj_log (URJ_LOG_LEVEL_DEBUG, " ");
        }
        urj_log (URJ_LOG_LEVEL_DEBUG, "\n");
    }

}

static uint32_t
get_data (urj_bus_t *bus, uint32_t adr)
{
    urj_bus_area_t area;
    int i;
    uint32_t d = 0;
    urj_part_t *p = bus->part;

    mpc824x_bus_area (bus, adr, &area);
    if (area.width > 64)
        return 0;

    for (i = 0; i < area.width; i++)
        d |= (uint32_t) (urj_part_get_signal (p, D[i]) <<
                         ((REVBITS == 1) ? BUS_WIDTH - 1 - i : i));

    /* Just for debugging */
    if (dbgData)
    {
        urj_log (URJ_LOG_LEVEL_DEBUG, _("Data RD [%d:0]: %08lX   "), area.width - 1,
                (long unsigned) d);
        int j;
        int bytes = 0;
        if (BUS_WIDTH == 8)
            bytes = 1;
        else if (BUS_WIDTH == 16)
            bytes = 2;
        else if (BUS_WIDTH == 32)
            bytes = 4;
        else if (BUS_WIDTH == 64)
            bytes = 4;          /* Needs to be fixed - d is 32-bit long, so no 64 bit mode is possible */

        for (i = 0; i < bytes; i++)
        {
            for (j = 0; j < 8; j++)
                if (REVBITS)
                    urj_log (URJ_LOG_LEVEL_DEBUG, "%1lu", (long unsigned)
                                    (d >> (BUS_WIDTH - 1 - (i * 8 + j))) & 1);
                else
                    urj_log (URJ_LOG_LEVEL_DEBUG, "%1lu", (long unsigned) (d >> ((i * 8 + j))) & 1);
            urj_log (URJ_LOG_LEVEL_DEBUG, " ");
        }
        urj_log (URJ_LOG_LEVEL_DEBUG, "\n");
    }

    return d;
}

/**
 * bus->driver->(*read_start)
 *
 */
static int
mpc824x_bus_read_start (urj_bus_t *bus, uint32_t adr)
{
    urj_part_t *p = bus->part;

    LAST_ADR = adr;

    /* see Figure 6-45 in [1] */
    urj_part_set_signal_low (p, nRCS0);
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
mpc824x_bus_read_next (urj_bus_t *bus, uint32_t adr)
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
mpc824x_bus_read_end (urj_bus_t *bus)
{
    urj_part_t *p = bus->part;

    urj_part_set_signal_high (p, nRCS0);
    urj_part_set_signal_high (p, nFOE);

    urj_tap_chain_shift_data_registers (bus->chain, 1);

    return get_data (bus, LAST_ADR);
}

/**
 * bus->driver->(*write)
 *
 */
static void
mpc824x_bus_write (urj_bus_t *bus, uint32_t adr, uint32_t data)
{
    urj_part_t *p = bus->part;

    LAST_ADR = adr;


    /* see Figure 6-47 in [1] */
    urj_part_set_signal_low (p, nRCS0);
    urj_part_set_signal_high (p, nWE);
    urj_part_set_signal_high (p, nFOE);

    setup_address (bus, adr);

    setup_data (bus, adr, data);

    urj_tap_chain_shift_data_registers (bus->chain, 0);

    urj_part_set_signal_low (p, nWE);
    urj_tap_chain_shift_data_registers (bus->chain, 0);
    urj_part_set_signal_high (p, nWE);
    urj_part_set_signal_high (p, nRCS0);
    urj_tap_chain_shift_data_registers (bus->chain, 0);



}

const urj_bus_driver_t urj_bus_mpc824x_bus = {
    "mpc824x",
    N_("Motorola MPC824x compatible bus driver via BSR"),
    mpc824x_bus_new,
    urj_bus_generic_free,
    mpc824x_bus_printinfo,
    urj_bus_generic_prepare_extest,
    mpc824x_bus_area,
    mpc824x_bus_read_start,
    mpc824x_bus_read_next,
    mpc824x_bus_read_end,
    urj_bus_generic_read,
    urj_bus_generic_write_start,
    mpc824x_bus_write,
    urj_bus_generic_no_init,
    urj_bus_generic_no_enable,
    urj_bus_generic_no_disable,
    URJ_BUS_TYPE_PARALLEL,
};
