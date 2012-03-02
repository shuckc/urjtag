/**
**  @file s3c4510x.c
**
**  $Id$
**
**  Copyright (C) 2003, All Rights Reserved
**
**  This program is free software; you can redistribute it and/or
**  modify it under the terms of the GNU General Public License
**  as published by the Free Software Foundation; either version 2
**  of the License, or (at your option) any later version.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program; if not, write to the Free Software
**  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
**  02111-1307, USA.
**
**  @author
**    Jiun-Shian Ho <asky@syncom.com.tw>,
**    Copy from other bus drivers written by Marcel Telka <marcel@telka.sk>
**
**    Krzysztof Blaszkowski <info@sysmikro.com.pl>
**    - fixed bug with driving nWBE, nECS, nSDCS (for SDRAM),
**    - fixed bug with preparing bus state after each urj_tap_chain_shift_data_registers().
**      tested on "peek" command only (2003/10/07).
**
**  @brief
**    Bus driver for Samsung S3C4510X (ARM7TDMI) micro controller.
**
**  @par Reference Documentations
**    - [1] Samsung Electronics Co., Ltd.,
**      "S3C4510B 32-Bit RISC Microcontroller User's Manual",
**      Revision 1, August 2000, Order Number: 21-S3-C4510B-082000
**
**  @note
**    - This bus driver is coded basing on S3C4510B.
**      However, Samsung do NOT giving a special JTAG ID-Code for this chip.
**    - Data Bus width is detected by B0SIZE[0:1];
**      the bus parameter is defined as 32-bit, but actually controlled by
**      @ref dbus_width. Make sure that B0SIZE[0:1] is welded correct.
**      Otherwise, you must modify @ref s3c4510_bus_width().
**    - ROM/Flash is selected by nRCS[5:0], now suppose only nRCS0.
**      So is nWBE[4:0], now suppose only nWBE0
**    - Unfortunately, B0SIZE isn't known before first SCAN/PRELOAD.
**      Is bus driver allowed to do JTAG activity during URJ_BUS_AREA or bus_new?
**
=============================================================================*/


#include <sysdep.h>

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <urjtag/log.h>
#include <urjtag/part.h>
#include <urjtag/bus.h>
#include <urjtag/chain.h>
#include <urjtag/bssignal.h>
#include <urjtag/tap_state.h>

#include "buses.h"
#include "generic_bus.h"



/** @brief  Bus driver for Samsung S3C4510X */
typedef struct
{
    urj_part_signal_t *a[22];          /**< Only 22-bits addressing */
    urj_part_signal_t *d[32];          /**< Data bus */
    urj_part_signal_t *nrcs[6];        /**< not ROM/SRAM/Flash Chip Select;
                              ** Only using nRCS0. */
    urj_part_signal_t *necs[4];
    urj_part_signal_t *nsdcs[4];

    urj_part_signal_t *nwbe[4];        /**< not Write Byte Enable */
    urj_part_signal_t *noe;            /**< not Output Enable */
    int dbuswidth;
} bus_params_t;

#define A       ((bus_params_t *) bus->params)->a
#define D       ((bus_params_t *) bus->params)->d
#define nRCS    ((bus_params_t *) bus->params)->nrcs
#define nECS    ((bus_params_t *) bus->params)->necs
#define nSDCS   ((bus_params_t *) bus->params)->nsdcs
#define nWBE    ((bus_params_t *) bus->params)->nwbe
#define nOE     ((bus_params_t *) bus->params)->noe

#define dbus_width ((bus_params_t *) bus->params)->dbuswidth
/** @brief  Width of Data Bus. Detected by B0SIZE[1:0]  */

/**
 * bus->driver->(*new_bus)
 *
 */
static urj_bus_t *
s3c4510_bus_new (urj_chain_t *chain, const urj_bus_driver_t *driver,
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

    dbus_width = 16;

    for (i = 0; i < 22; i++)
    {
        sprintf (buff, "ADDR%d", i);
        failed |= urj_bus_generic_attach_sig (part, &(A[i]), buff);
    }

    for (i = 0; i < 32; i++)
    {
        sprintf (buff, "XDATA%d", i);
        failed |= urj_bus_generic_attach_sig (part, &(D[i]), buff);
    }

    for (i = 0; i < 6; i++)
    {
        sprintf (buff, "nRCS%d", i);
        failed |= urj_bus_generic_attach_sig (part, &(nRCS[i]), buff);
    }

    for (i = 0; i < 4; i++)
    {
        sprintf (buff, "nECS%d", i);
        failed |= urj_bus_generic_attach_sig (part, &(nECS[i]), buff);
    }

    for (i = 0; i < 4; i++)
    {
        sprintf (buff, "nRAS%d", i);    /* those are nSDCS for SDRAMs only */
        failed |= urj_bus_generic_attach_sig (part, &(nSDCS[i]), buff);
    }

    for (i = 0; i < 4; i++)
    {
        sprintf (buff, "nWBE%d", i);
        failed |= urj_bus_generic_attach_sig (part, &(nWBE[i]), buff);
    }

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
s3c4510_bus_printinfo (urj_log_level_t ll, urj_bus_t *bus)
{
    int i;

    for (i = 0; i < bus->chain->parts->len; i++)
        if (bus->part == bus->chain->parts->parts[i])
            break;
    urj_log (ll, _("Samsung S3C4510B compatibile bus driver via BSR (JTAG part No. %d) RCS0=%ubit\n"),
            i, dbus_width);
}

/**
 * bus->driver->(*init)
 *
 */
static int
s3c4510_bus_init (urj_bus_t *bus)
{
    urj_part_t *p = bus->part;
    urj_chain_t *chain = bus->chain;

    if (urj_tap_state (chain) != URJ_TAP_STATE_RUN_TEST_IDLE)
    {
        /* silently skip initialization if TAP isn't in RUNTEST/IDLE state
           this is required to avoid interfering with detect when initbus
           is contained in the part description file
           URJ_BUS_INIT() will be called latest by URJ_BUS_PREPARE() */
        return URJ_STATUS_OK;
    }

    urj_part_set_instruction (p, "SAMPLE/PRELOAD");
    urj_tap_chain_shift_instructions (chain);
    urj_tap_chain_shift_data_registers (chain, 0);

    bus->initialized = 1;

    return URJ_STATUS_OK;
}

/**
 * bus->driver->(*area)
 *
 */
static int
s3c4510_bus_area (urj_bus_t *bus, uint32_t adr, urj_bus_area_t *area)
{
    int b0size0, b0size1;       // , endian;

    area->description = NULL;
    area->start = UINT32_C (0x00000000);
    area->length = UINT64_C (0x100000000);

    // endian = urj_part_get_signal( bus->part, urj_part_find_signal( bus->part, "LITTLE" ));
    b0size0 = urj_part_get_signal (bus->part,
                                   urj_part_find_signal (bus->part, "B0SIZE0"));
    b0size1 = urj_part_get_signal (bus->part,
                                   urj_part_find_signal (bus->part, "B0SIZE1"));

    switch ((b0size1 << 1) | b0size0)
    {
    case 1:
        area->width = dbus_width = 8;
        return URJ_STATUS_OK;
    case 2:
        area->width = dbus_width = 16;
        return URJ_STATUS_OK;
    case 3:
        area->width = dbus_width = 32;
        return URJ_STATUS_OK;
    default:
        urj_error_set (URJ_ERROR_INVALID, ("B0SIZE[1:0] 0x%01x: Unknown"),
                       (b0size1 << 1) | b0size0);
        area->width = 0;
        return URJ_STATUS_FAIL;
    }
}

static void
s3c4510_bus_setup_ctrl (urj_bus_t *bus, int mode)
{
    int k;
    urj_part_t *p = bus->part;

    for (k = 0; k < 6; k++)
        urj_part_set_signal (p, nRCS[k], 1, (mode & (1 << k)) ? 1 : 0);

    for (k = 0; k < 4; k++)
        urj_part_set_signal_high (p, nECS[k]);

    for (k = 0; k < 4; k++)
        urj_part_set_signal_high (p, nSDCS[k]);

    for (k = 0; k < 4; k++)
        urj_part_set_signal (p, nWBE[k], 1, (mode & (1 << (k + 8))) ? 1 : 0);

    urj_part_set_signal (p, nOE, 1, (mode & (1 << 16)) ? 1 : 0);
}

static void
setup_address (urj_bus_t *bus, uint32_t a)
{
    int i, so;
    urj_part_t *p = bus->part;

    switch (dbus_width)
    {
    case 32:
        so = 2;
        break;
    case 16:
        so = 1;
        break;
    default:
        so = 0;
        break;
    }

    for (i = 0; i < 22; i++)
        urj_part_set_signal (p, A[i], 1, (a >> (i + so)) & 1);
}

static void
set_data_in (urj_bus_t *bus)
{
    int i;
    urj_part_t *p = bus->part;

    for (i = 0; i < dbus_width; i++)
        urj_part_set_signal_input (p, D[i]);
}

static void
setup_data (urj_bus_t *bus, uint32_t d)
{
    int i;
    urj_part_t *p = bus->part;

    for (i = 0; i < dbus_width; i++)
        urj_part_set_signal (p, D[i], 1, (d >> i) & 1);
    /* Set other bits as 0 */
    for (i = dbus_width; i < 32; i++)
        urj_part_set_signal_low (p, D[i]);
}

/**
 * bus->driver->(*read_start)
 *
 */
static int
s3c4510_bus_read_start (urj_bus_t *bus, uint32_t adr)
{
    /* see Figure 4-19 in [1] */
    urj_chain_t *chain = bus->chain;

    s3c4510_bus_setup_ctrl (bus, 0x00fffe);     /* nOE=0, nRCS0 =0 */
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
s3c4510_bus_read_next (urj_bus_t *bus, uint32_t adr)
{
    /* see Figure 4-20 in [1] */
    urj_part_t *p = bus->part;
    urj_chain_t *chain = bus->chain;
    int i;
    uint32_t d = 0;

    s3c4510_bus_setup_ctrl (bus, 0x00fffe);     /* nOE=0, nRCS0 =0 */
    setup_address (bus, adr);
    set_data_in (bus);
    urj_tap_chain_shift_data_registers (chain, 1);

    for (i = 0; i < dbus_width; i++)
        d |= (uint32_t) (urj_part_get_signal (p, D[i]) << i);

    return d;
}

/**
 * bus->driver->(*read_end)
 *
 */
static uint32_t
s3c4510_bus_read_end (urj_bus_t *bus)
{
    /* see Figure 4-19 in [1] */
    urj_part_t *p = bus->part;
    urj_chain_t *chain = bus->chain;
    int i;
    uint32_t d = 0;

    s3c4510_bus_setup_ctrl (bus, 0x01ffff);     /* nOE=1, nRCS0 =1 */
    urj_tap_chain_shift_data_registers (chain, 1);

    for (i = 0; i < dbus_width; i++)
        d |= (uint32_t) (urj_part_get_signal (p, D[i]) << i);

    return d;
}

/**
 * bus->driver->(*write)
 *
 *  @brief
 *    ROM/SRAM/FlashPage Write Access Timing
 */
static void
s3c4510_bus_write (urj_bus_t *bus, uint32_t adr, uint32_t data)
{
    /* see Figure 4-21 in [1] */
    urj_chain_t *chain = bus->chain;

    s3c4510_bus_setup_ctrl (bus, 0x01fffe);     /* nOE=1, nRCS0 =0 */
    setup_address (bus, adr);
    setup_data (bus, data);

    urj_tap_chain_shift_data_registers (chain, 0);

    switch (dbus_width)
    {
    default:
    case 8:
        s3c4510_bus_setup_ctrl (bus, 0x01fefe); /* nOE=1, nRCS0 =0, nWBE0=0 */
        break;
    case 16:
        s3c4510_bus_setup_ctrl (bus, 0x01fcfe); /* nOE=1, nRCS0 =0, nWBE0-1=0 */
        break;

    case 32:
        s3c4510_bus_setup_ctrl (bus, 0x01f0fe); /* nOE=1, nRCS0 =0, nWBE0-3=0 */
        break;
    }

    setup_address (bus, adr);
    setup_data (bus, data);

    urj_tap_chain_shift_data_registers (chain, 0);

    s3c4510_bus_setup_ctrl (bus, 0x01ffff);     /* nOE=1, nRCS0 =1 */
    urj_tap_chain_shift_data_registers (chain, 0);

    urj_log (URJ_LOG_LEVEL_DEBUG, "URJ_BUS_WRITE %08lx @ %08lx\n",
             (long unsigned) data, (long unsigned) adr);
}

const urj_bus_driver_t urj_bus_s3c4510_bus = {
    "s3c4510x",
    N_("Samsung S3C4510B compatible bus driver via BSR"),
    s3c4510_bus_new,
    urj_bus_generic_free,
    s3c4510_bus_printinfo,
    urj_bus_generic_prepare_extest,
    s3c4510_bus_area,
    s3c4510_bus_read_start,
    s3c4510_bus_read_next,
    s3c4510_bus_read_end,
    urj_bus_generic_read,
    urj_bus_generic_write_start,
    s3c4510_bus_write,
    s3c4510_bus_init,
    urj_bus_generic_no_enable,
    urj_bus_generic_no_disable,
    URJ_BUS_TYPE_PARALLEL,
};
