/*
 * $Id$
 *
 * Altera UP3 Education Kit bus driver via BSR
 * Copyright (C) 2005 Kent Palmkvist
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
 * Written by Kent Palmkvist <kentp@isy.liu.se>, 2005.
 *
 * Documentation:
 * [1] System Level Solutions Inc., "UP3 Education Kit, Reference Manual",
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

static int databusio[16] =
    { 94, 96, 98, 100, 102, 104, 106, 113, 95, 97, 99, 101, 103, 105, 107,
    114
};
static int addrbusio[20] =
    { 93, 88, 87, 86, 85, 84, 83, 63, 64, 65, 66, 67, 68, 74, 75, 76, 77, 82,
    81, 78
};

typedef struct
{
    uint32_t last_adr;
    urj_part_signal_t *ad[20];
    urj_part_signal_t *dq[16];
    urj_part_signal_t *nsdce;
    urj_part_signal_t *sdclk;
    urj_part_signal_t *noe;
    urj_part_signal_t *nsrce;
    urj_part_signal_t *nflce;
    urj_part_signal_t *nflbyte;
    urj_part_signal_t *nflby;
    urj_part_signal_t *nwe;
    urj_part_signal_t *lcde;
    urj_part_signal_t *lcdrs;
    urj_part_signal_t *lcdrw;
} bus_params_t;

#define LAST_ADR        ((bus_params_t *) bus->params)->last_adr
#define AD              ((bus_params_t *) bus->params)->ad
#define DQ              ((bus_params_t *) bus->params)->dq
#define nSDce           ((bus_params_t *) bus->params)->nsdce
#define nOE             ((bus_params_t *) bus->params)->noe
#define nSRce           ((bus_params_t *) bus->params)->nsrce
#define nFLce           ((bus_params_t *) bus->params)->nflce
#define nFLbyte         ((bus_params_t *) bus->params)->nflbyte
#define nFLby           ((bus_params_t *) bus->params)->nflby
#define nWE             ((bus_params_t *) bus->params)->nwe
#define SDclk           ((bus_params_t *) bus->params)->sdclk
#define LCDe            ((bus_params_t *) bus->params)->lcde
#define LCDrs           ((bus_params_t *) bus->params)->lcdrs
#define LCDrw           ((bus_params_t *) bus->params)->lcdrw

/* All addresses and length are in Bytes */
/* Assume 8 bit flash data bus */
#define FLASHSTART      UINT32_C(0x0000000)
#define FLASHSIZE       UINT64_C(0x0200000)     /* Number of bytes */
/* Assume 16 bit SRAM data bus */
#define SRAMSTART       0x0200000
#define SRAMSIZE        0x0020000
#define LCDSTART        0x0300000
#define LCDSIZE         0x0100000

/**
 * bus->driver->(*new_bus)
 *
 */
static urj_bus_t *
slsup3_bus_new (urj_chain_t *chain, const urj_bus_driver_t *driver,
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

    for (i = 0; i < 20; i++)
    {
        sprintf (buff, "IO%d", addrbusio[i]);
        failed |= urj_bus_generic_attach_sig (part, &(AD[i]), buff);
    }

    for (i = 0; i < 16; i++)
    {
        sprintf (buff, "IO%d", databusio[i]);
        failed |= urj_bus_generic_attach_sig (part, &(DQ[i]), buff);
    }

    failed |= urj_bus_generic_attach_sig (part, &(nOE), "IO118");

    failed |= urj_bus_generic_attach_sig (part, &(nSRce), "IO116");

    failed |= urj_bus_generic_attach_sig (part, &(nSDce), "IO119");

    failed |= urj_bus_generic_attach_sig (part, &(nFLce), "IO117");

    failed |= urj_bus_generic_attach_sig (part, &(nFLbyte), "IO115");

    failed |= urj_bus_generic_attach_sig (part, &(nFLby), "IO80");

    failed |= urj_bus_generic_attach_sig (part, &(nWE), "IO79");

    failed |= urj_bus_generic_attach_sig (part, &(SDclk), "IO11");

    failed |= urj_bus_generic_attach_sig (part, &(LCDe), "IO50");

    failed |= urj_bus_generic_attach_sig (part, &(LCDrs), "IO108");

    failed |= urj_bus_generic_attach_sig (part, &(LCDrw), "IO73");

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
slsup3_bus_printinfo (urj_log_level_t ll, urj_bus_t *bus)
{
    int i;

    for (i = 0; i < bus->chain->parts->len; i++)
        if (bus->part == bus->chain->parts->parts[i])
            break;
    urj_log (ll, _("SLS UP3 bus driver via BSR (JTAG part No. %d)\n"), i);
}

/**
 * bus->driver->(*area)
 *
 */
static int
slsup3_bus_area (urj_bus_t *bus, uint32_t adr, urj_bus_area_t *area)
{
    if ((adr >= FLASHSTART) && (adr < (FLASHSTART + FLASHSIZE)))
    {
        area->description = N_("Flash Memory (2 MByte) byte mode");
        area->start = FLASHSTART;
        area->length = FLASHSIZE;
        area->width = 8;        /* 16 */

        return URJ_STATUS_OK;
    }

    if ((adr >= SRAMSTART) && (adr < (SRAMSTART + SRAMSIZE)))
    {
        area->description = N_("SRAM 128KByte (64K x 16)");
        area->start = SRAMSTART;
        area->length = SRAMSIZE;
        area->width = 16;

        return URJ_STATUS_OK;
    }

    if ((adr >= LCDSTART) && (adr < (LCDSTART + LCDSIZE)))
    {
        area->description = N_("LCD Display (RS select by A0)");
        area->start = LCDSTART;
        area->length = LCDSIZE;
        area->width = 8;

        return URJ_STATUS_OK;
    }

    area->description = NULL;
    area->start = UINT32_C (0x0400000);
    area->length = UINT64_C (0xFFC00000);
    area->width = 0;

    return URJ_STATUS_OK;
}

static void
setup_address (urj_bus_t *bus, uint32_t a)
{
    int i;
    urj_part_t *p = bus->part;
    urj_bus_area_t area;

    slsup3_bus_area (bus, a, &area);
    if (area.width > 16)
        return;

    urj_part_set_signal (p, LCDrs, 1, a & 1);

    /* FLASH memory address setup. Use DQ15 to select byte */
    if ((a >= (FLASHSTART)) && (a < (FLASHSTART + FLASHSIZE)))
    {
        for (i = 0; i < 20; i++)
            urj_part_set_signal (p, AD[i], 1, (a >> (i + 1)) & 1);
        urj_part_set_signal_low (p, nFLce);
        urj_part_set_signal (p, DQ[15], 1, (a & 1));
    }
    else
        urj_part_set_signal_high (p, nFLce);

    /* SRAM memory address setup */
    if ((a >= SRAMSTART) && (a < (SRAMSTART + SRAMSIZE)))
    {
        urj_part_set_signal_low (p, nSRce);
        for (i = 0; i < 20; i++)
            urj_part_set_signal (p, AD[i], 1,
                                 (a >> (i + (area.width / 8) - 1)) & 1);
    }
    else
        urj_part_set_signal_high (p, nSRce);


}

static void
set_data_in (urj_bus_t *bus, uint32_t adr)
{
    int i;
    urj_part_t *p = bus->part;
    urj_bus_area_t area;

    slsup3_bus_area (bus, adr, &area);
    if (area.width > 16)
        return;

    for (i = 0; i < area.width; i++)
        urj_part_set_signal_input (p, DQ[i]);
}

static void
setup_data (urj_bus_t *bus, uint32_t adr, uint32_t d)
{
    int i;
    urj_part_t *p = bus->part;
    urj_bus_area_t area;

    slsup3_bus_area (bus, adr, &area);
    if (area.width > 16)
        return;

    for (i = 0; i < area.width; i++)
        urj_part_set_signal (p, DQ[i], 1, (d >> i) & 1);
}

static uint32_t
get_data (urj_bus_t *bus, uint32_t adr)
{
    urj_bus_area_t area;
    int i;
    uint32_t d = 0;
    urj_part_t *p = bus->part;

    slsup3_bus_area (bus, adr, &area);
    if (area.width > 16)
        return 0;

    for (i = 0; i < area.width; i++)
        d |= (uint32_t) (urj_part_get_signal (p, DQ[i]) << i);

    return d;
}

/**
 * bus->driver->(*read_start)
 *
 */
static int
slsup3_bus_read_start (urj_bus_t *bus, uint32_t adr)
{
    urj_part_t *p = bus->part;

    LAST_ADR = adr;

    urj_part_set_signal_high (p, nSDce);       /* Inihibit SDRAM */
    urj_part_set_signal_low (p, nOE);
    urj_part_set_signal_high (p, nSRce);
    urj_part_set_signal_high (p, nFLce);
    urj_part_set_signal_low (p, nFLbyte);
    urj_part_set_signal_high (p, nWE);
    urj_part_set_signal_low (p, SDclk);
    urj_part_set_signal_low (p, LCDe);
    urj_part_set_signal_high (p, LCDrw);

    setup_address (bus, adr);

    if ((adr >= LCDSTART) && (adr < (LCDSTART + LCDSIZE)))
    {
        urj_part_set_signal_high (p, LCDe);
        urj_tap_chain_shift_data_registers (bus->chain, 0);
        urj_part_set_signal_low (p, LCDe);
    }

    set_data_in (bus, adr);

    urj_tap_chain_shift_data_registers (bus->chain, 0);

    return URJ_STATUS_OK;
}

/**
 * bus->driver->(*read_next)
 *
 */
static uint32_t
slsup3_bus_read_next (urj_bus_t *bus, uint32_t adr)
{
    uint32_t d;

    urj_part_t *p = bus->part;

    setup_address (bus, adr);

    if ((adr >= LCDSTART) && (adr < (LCDSTART + LCDSIZE)))
    {
        urj_part_set_signal_high (p, LCDe);
        urj_tap_chain_shift_data_registers (bus->chain, 0);
        urj_part_set_signal_low (p, LCDe);
    }

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
slsup3_bus_read_end (urj_bus_t *bus)
{
    urj_part_t *p = bus->part;
    uint32_t d;

    if ((LAST_ADR >= LCDSTART) && (LAST_ADR < (LCDSTART + LCDSIZE)))
    {
        urj_part_set_signal_high (p, LCDe);
        urj_tap_chain_shift_data_registers (bus->chain, 0);
        urj_part_set_signal_low (p, LCDe);
    }

    urj_part_set_signal_high (p, nOE);

    urj_tap_chain_shift_data_registers (bus->chain, 1);

    d = get_data (bus, LAST_ADR);

    return d;
}

/**
 * bus->driver->(*write)
 *
 */
static void
slsup3_bus_write (urj_bus_t *bus, uint32_t adr, uint32_t data)
{
    urj_part_t *p = bus->part;
    urj_chain_t *chain = bus->chain;

    urj_part_set_signal_high (p, nSDce);       /* Inihibit SDRAM */
    urj_part_set_signal_high (p, nOE);
    urj_part_set_signal_high (p, nSRce);
    urj_part_set_signal_high (p, nFLce);
    urj_part_set_signal_low (p, nFLbyte);
    urj_part_set_signal_high (p, nWE);
    urj_part_set_signal_low (p, SDclk);
    urj_part_set_signal_low (p, LCDe);
    urj_part_set_signal_low (p, LCDrw);

    setup_address (bus, adr);
    setup_data (bus, adr, data);

    if ((adr >= LCDSTART) && (adr < (LCDSTART + LCDSIZE)))
    {
        urj_tap_chain_shift_data_registers (chain, 0);
        urj_part_set_signal_high (p, LCDe);
        urj_tap_chain_shift_data_registers (bus->chain, 0);
        urj_part_set_signal_low (p, LCDe);
        urj_tap_chain_shift_data_registers (bus->chain, 0);
    }
    else
    {

        urj_tap_chain_shift_data_registers (chain, 0);

        urj_part_set_signal_low (p, nWE);
        urj_tap_chain_shift_data_registers (chain, 0);
        urj_part_set_signal_high (p, nWE);
        urj_tap_chain_shift_data_registers (chain, 0);
    }
}

const urj_bus_driver_t urj_bus_slsup3_bus = {
    "slsup3",
    N_("SLS UP3 compatible bus driver via BSR"),
    slsup3_bus_new,
    urj_bus_generic_free,
    slsup3_bus_printinfo,
    urj_bus_generic_prepare_extest,
    slsup3_bus_area,
    slsup3_bus_read_start,
    slsup3_bus_read_next,
    slsup3_bus_read_end,
    urj_bus_generic_read,
    urj_bus_generic_write_start,
    slsup3_bus_write,
    urj_bus_generic_no_init,
    urj_bus_generic_no_enable,
    urj_bus_generic_no_disable,
    URJ_BUS_TYPE_PARALLEL,
};
