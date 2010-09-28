/*
 * bf518_ezbrd.c
 *
 * Analog Devices ADSP-BF518F EZ-BRD bus driver
 * Copyright (C) 2009, 2010 Analog Devices, Inc.
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
 * Written by Jie Zhang <jie.zhang@analog.com>, 2009.
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

typedef struct
{
    urj_part_signal_t *ams[2];
    urj_part_signal_t *addr[19];
    urj_part_signal_t *data[16];
    urj_part_signal_t *abe[2];
    urj_part_signal_t *awe;
    urj_part_signal_t *are;
    urj_part_signal_t *sras;
    urj_part_signal_t *scas;
    urj_part_signal_t *sms;
    urj_part_signal_t *swe;
} bus_params_t;

#define AMS     ((bus_params_t *) bus->params)->ams
#define ADDR    ((bus_params_t *) bus->params)->addr
#define DATA    ((bus_params_t *) bus->params)->data
#define AWE     ((bus_params_t *) bus->params)->awe
#define ARE     ((bus_params_t *) bus->params)->are
#define ABE     ((bus_params_t *) bus->params)->abe
#define SRAS    ((bus_params_t *) bus->params)->sras
#define SCAS    ((bus_params_t *) bus->params)->scas
#define SMS     ((bus_params_t *) bus->params)->sms
#define SWE     ((bus_params_t *) bus->params)->swe

/*
 * bus->driver->(*new_bus)
 *
 */
static urj_bus_t *
bf518f_ezbrd_bus_new (urj_chain_t *chain, const urj_bus_driver_t *driver,
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

    for (i = 0; i < 2; i++)
    {
        sprintf (buff, "AMS_B%d", i);
        failed |= urj_bus_generic_attach_sig (part, &(AMS[i]), buff);
    }

    for (i = 0; i < 19; i++)
    {
        sprintf (buff, "ADDR%d", i + 1);
        failed |= urj_bus_generic_attach_sig (part, &(ADDR[i]), buff);
    }

    for (i = 0; i < 16; i++)
    {
        sprintf (buff, "DATA%d", i);
        failed |= urj_bus_generic_attach_sig (part, &(DATA[i]), buff);
    }

    failed |= urj_bus_generic_attach_sig (part, &(AWE), "AWE_n");

    failed |= urj_bus_generic_attach_sig (part, &(ARE), "ARE_n");

    failed |= urj_bus_generic_attach_sig (part, &(ABE[0]), "ABE_B0");

    failed |= urj_bus_generic_attach_sig (part, &(ABE[1]), "ABE_B1");

    failed |= urj_bus_generic_attach_sig (part, &(SRAS), "SRAS_n");

    failed |= urj_bus_generic_attach_sig (part, &(SCAS), "SCAS_n");

    failed |= urj_bus_generic_attach_sig (part, &(SWE), "SWE_n");

    failed |= urj_bus_generic_attach_sig (part, &(SMS), "SMS_n");

    if (failed)
    {
        urj_bus_generic_free (bus);
        return NULL;
    }

    return bus;
}

/**
 * bus->driver->(*area)
 *
 */

#define ASYNC_MEM_BASE 0x20000000
#define ASYNC_MEM_SIZE (2 * 1024 * 1024)
#define IS_ASYNC_ADDR(addr) ({ \
       unsigned long __addr = (unsigned long) addr; \
       __addr >= ASYNC_MEM_BASE && __addr < ASYNC_MEM_BASE + ASYNC_MEM_SIZE; \
       })
#define ASYNC_BANK(addr) (((addr) & (ASYNC_MEM_SIZE - 1)) >> 20)

static int
bf518f_ezbrd_bus_area (urj_bus_t *bus, uint32_t adr, urj_bus_area_t *area)
{
    if (adr < ASYNC_MEM_BASE)
    {
        /* we can only wiggle SDRAM pins directly, so cannot drive it */
        urj_error_set (URJ_ERROR_OUT_OF_BOUNDS,
                       _("reading external memory not supported"));
        return URJ_STATUS_FAIL;
    }
    else if (IS_ASYNC_ADDR(adr))
    {
        area->description = "asynchronous memory";
        area->start = ASYNC_MEM_BASE;
        area->length = ASYNC_MEM_SIZE;
        area->width = 16;
    }
    else
    {
        /* L1 needs core to access it */
        urj_error_set (URJ_ERROR_OUT_OF_BOUNDS,
                       _("reading on-chip memory not supported"));
        return URJ_STATUS_FAIL;
    }
    return URJ_STATUS_OK;
}

static void
select_flash (urj_bus_t *bus, uint32_t adr)
{
    urj_part_t *p = bus->part;

    urj_part_set_signal (p, AMS[0], 1, !(ASYNC_BANK(adr) == 0));
    urj_part_set_signal (p, AMS[1], 1, !(ASYNC_BANK(adr) == 1));

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
static int
bf518f_ezbrd_bus_read_start (urj_bus_t *bus, uint32_t adr)
{
    urj_part_t *p = bus->part;
    urj_chain_t *chain = bus->chain;

    select_flash (bus, adr);
    urj_part_set_signal (p, ARE, 1, 0);
    urj_part_set_signal (p, AWE, 1, 1);

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
bf518f_ezbrd_bus_read_next (urj_bus_t *bus, uint32_t adr)
{
    urj_part_t *p = bus->part;
    urj_chain_t *chain = bus->chain;
    int i;
    uint32_t d = 0;

    setup_address( bus, adr );
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
bf518f_ezbrd_bus_read_end (urj_bus_t *bus)
{
    urj_part_t *p = bus->part;
    urj_chain_t *chain = bus->chain;
    int i;
    uint32_t d = 0;

    unselect_flash (bus);
    urj_part_set_signal (p, ARE, 1, 1);
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
bf518f_ezbrd_bus_write (urj_bus_t *bus, uint32_t adr, uint32_t data)
{
    urj_part_t *p = bus->part;
    urj_chain_t *chain = bus->chain;

    select_flash (bus, adr);
    urj_part_set_signal (p, ARE, 1, 1);

    setup_address (bus, adr);
    setup_data (bus, data);

    urj_tap_chain_shift_data_registers (chain, 0);

    urj_part_set_signal (p, AWE, 1, 0);
    urj_tap_chain_shift_data_registers (chain, 0);
    urj_part_set_signal (p, AWE, 1, 1);
    unselect_flash (bus);
    urj_tap_chain_shift_data_registers (chain, 0);
}

/**
 * bus->driver->(*printinfo)
 *
 */
static void
bf518f_ezbrd_bus_printinfo (urj_log_level_t ll, urj_bus_t *bus)
{
    int i;

    for (i = 0; i < bus->chain->parts->len; i++)
        if (bus->part == bus->chain->parts->parts[i])
            break;
    urj_log (ll, _("%s (JTAG part No. %d)\n"), bus->driver->description, i);
}

#define BF518F_EZBRD_BUS_FUNCTIONS \
    bf518f_ezbrd_bus_new, \
    urj_bus_generic_free, \
    bf518f_ezbrd_bus_printinfo, \
    urj_bus_generic_prepare_extest, \
    bf518f_ezbrd_bus_area, \
    bf518f_ezbrd_bus_read_start, \
    bf518f_ezbrd_bus_read_next, \
    bf518f_ezbrd_bus_read_end, \
    urj_bus_generic_read, \
    bf518f_ezbrd_bus_write, \
    urj_bus_generic_no_init

#ifdef ENABLE_BUS_BF518F_EZBRD

const urj_bus_driver_t urj_bus_bf518f_ezbrd_bus =
{
    "bf518f_ezbrd",
    N_("Blackfin BF518F EZ-BRD bus driver via BSR"),
    BF518F_EZBRD_BUS_FUNCTIONS
};

#endif /* #ifdef ENABLE_BUS_BF518F_EZBRD */

#ifdef ENABLE_BUS_BF51X

const urj_bus_driver_t urj_bus_bf51x_bus =
{
    "bf51x",
    N_("Generic Blackfin BF51x bus driver via BSR"),
    BF518F_EZBRD_BUS_FUNCTIONS
};

#endif /* #ifdef ENABLE_BUS_BF51X */
