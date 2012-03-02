/*
 * $Id$
 *
 * Copyright (C) 2003 ETC s.r.o.
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
 * Written by Matan Ziv-Av.
 * Modified by Marcel Telka <marcel@telka.sk>, 2003.
 *
 */

#include <sysdep.h>

#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#include <urjtag/part.h>
#include <urjtag/bus.h>
#include <urjtag/chain.h>
#include <urjtag/bssignal.h>

#include "buses.h"
#include "generic_bus.h"

//#define USE_BCM_EJTAG

typedef struct
{
    urj_part_signal_t *io_ad[32];
    urj_part_signal_t *io_cs_l[8];
    urj_part_signal_t *io_rw;
    urj_part_signal_t *io_wr_l;
    urj_part_signal_t *io_oe_l;
} bus_params_t;

#define IO_AD   ((bus_params_t *) bus->params)->io_ad
#define IO_CS_L ((bus_params_t *) bus->params)->io_cs_l
#define IO_RW   ((bus_params_t *) bus->params)->io_rw
#define IO_WR_L ((bus_params_t *) bus->params)->io_wr_l
#define IO_OE_L ((bus_params_t *) bus->params)->io_oe_l

/**
 * bus->driver->(*new_bus)
 *
 */
static urj_bus_t *
bcm1250_bus_new (urj_chain_t *chain, const urj_bus_driver_t *driver,
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

    for (i = 0; i < 32; i++)
    {
        sprintf (buff, "IO_AD%d", i);
        failed |= urj_bus_generic_attach_sig (part, &(IO_AD[i]), buff);
    }

    for (i = 0; i < 8; i++)
    {
        sprintf (buff, "IO_CS_L%d", i);
        failed |= urj_bus_generic_attach_sig (part, &(IO_CS_L[i]), buff);
    }

    failed |= urj_bus_generic_attach_sig (part, &(IO_RW), "IO_RW");

    failed |= urj_bus_generic_attach_sig (part, &(IO_WR_L), "IO_WR_L");

    failed |= urj_bus_generic_attach_sig (part, &(IO_OE_L), "IO_OE_L");

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
bcm1250_bus_printinfo (urj_log_level_t ll, urj_bus_t *bus)
{
    int i;

    for (i = 0; i < bus->chain->parts->len; i++)
        if (bus->part == bus->chain->parts->parts[i])
            break;
    urj_log (ll, _("Broadcom BCM1250 compatible bus driver via BSR (JTAG part No. %d)\n"),
            i);
}

/**
 * bus->driver->(*area)
 *
 */
static int
bcm1250_bus_area (urj_bus_t *bus, uint32_t addr, urj_bus_area_t *area)
{
    area->description = NULL;
    area->start = UINT32_C (0x00000000);
    area->length = UINT64_C (0x100000000);
    area->width = 8;

    return URJ_STATUS_OK;
}

#ifndef USE_BCM_EJTAG

static void
setup_address (urj_bus_t *bus, uint32_t a)
{
    int i;
    urj_part_t *p = bus->part;

    for (i = 0; i < 24; i++)
    {
        urj_part_set_signal (p, IO_AD[i], 1, (a >> i) & 1);
    }
}

static void
set_data_in (urj_bus_t *bus)
{
    int i;
    urj_part_t *p = bus->part;

    for (i = 0; i < 8; i++)
    {
        urj_part_set_signal_input (p, IO_AD[i + 24]);
    }
}

static void
setup_data (urj_bus_t *bus, uint32_t d)
{
    int i;
    urj_part_t *p = bus->part;

    for (i = 0; i < 8; i++)
    {
        urj_part_set_signal (p, IO_AD[i + 24], 1, (d >> i) & 1);
    }
}

/**
 * bus->driver->(*read_start)
 *
 */
static int
bcm1250_bus_read_start (urj_bus_t *bus, uint32_t adr)
{
    urj_part_t *p = bus->part;
    urj_chain_t *chain = bus->chain;

    urj_part_set_signal_low (p, IO_CS_L[0]);
    urj_part_set_signal_high (p, IO_CS_L[1]);
    urj_part_set_signal_high (p, IO_CS_L[2]);
    urj_part_set_signal_high (p, IO_CS_L[3]);
    urj_part_set_signal_high (p, IO_CS_L[4]);
    urj_part_set_signal_high (p, IO_CS_L[5]);
    urj_part_set_signal_high (p, IO_CS_L[6]);
    urj_part_set_signal_high (p, IO_CS_L[7]);
    urj_part_set_signal_high (p, IO_RW);
    urj_part_set_signal_high (p, IO_WR_L);
    urj_part_set_signal_low (p, IO_OE_L);

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
bcm1250_bus_read_next (urj_bus_t *bus, uint32_t adr)
{
    urj_part_t *p = bus->part;
    urj_chain_t *chain = bus->chain;

    setup_address (bus, adr);
    urj_tap_chain_shift_data_registers (chain, 1);

    {
        int i;
        uint32_t d = 0;

        for (i = 0; i < 8; i++)
        {
            d |= (uint32_t) (urj_part_get_signal (p, IO_AD[i + 24]) << i);
        }

        return d;
    }
}

/**
 * bus->driver->(*read_end)
 *
 */
static uint32_t
bcm1250_bus_read_end (urj_bus_t *bus)
{
    urj_part_t *p = bus->part;
    urj_chain_t *chain = bus->chain;

    urj_part_set_signal_high (p, IO_CS_L[0]);
    urj_part_set_signal_high (p, IO_OE_L);
    urj_tap_chain_shift_data_registers (chain, 1);

    {
        int i;
        uint32_t d = 0;

        for (i = 0; i < 8; i++)
        {
            d |= (uint32_t) (urj_part_get_signal (p, IO_AD[i + 24]) << i);
        }

        return d;
    }
}

/**
 * bus->driver->(*write)
 *
 */
static void
bcm1250_bus_write (urj_bus_t *bus, uint32_t adr, uint32_t data)
{
    urj_part_t *p = bus->part;
    urj_chain_t *chain = bus->chain;

    urj_part_set_signal_low (p, IO_CS_L[0]);
    urj_part_set_signal_high (p, IO_CS_L[1]);
    urj_part_set_signal_high (p, IO_CS_L[2]);
    urj_part_set_signal_high (p, IO_CS_L[3]);
    urj_part_set_signal_high (p, IO_CS_L[4]);
    urj_part_set_signal_high (p, IO_CS_L[5]);
    urj_part_set_signal_high (p, IO_CS_L[6]);
    urj_part_set_signal_high (p, IO_CS_L[7]);
    urj_part_set_signal_low (p, IO_RW);
    urj_part_set_signal_high (p, IO_WR_L);
    urj_part_set_signal_high (p, IO_OE_L);

    setup_address (bus, adr);
    setup_data (bus, data);

    urj_tap_chain_shift_data_registers (chain, 0);

    urj_part_set_signal_low (p, IO_WR_L);
    urj_tap_chain_shift_data_registers (chain, 0);

    urj_part_set_signal_high (p, IO_WR_L);
    urj_tap_chain_shift_data_registers (chain, 0);
}

#else /* #ifndef USE_BCM_EJTAG */

static int addr;
static uint64_t base = 0x1fc00000;

static int
bcm1250_ejtag_do (urj_bus_t *bus, uint64_t ad, uint64_t da, int read,
                  int type, unsigned char *buf)
{

    urj_part_t *p = bus->part;
    urj_chain_t *chain = bus->chain;
    char ctrl[15] = "010000000000";
    char addrr[80] = "0000" "111" "000"
        "11111111111111111111111111111111"
        "00000000" "00011111" "11000000" "00000000" "000";
    int j, k, n, m;
    uint64_t a;

    urj_log (URJ_LOG_LEVEL_DETAIL, "BCM1250: ejtag_do(%08Lx, %08Lx, %i, %i)\n",
             ad, da, read, type);

    a = ad >> 5;
    for (j = 0; j < 35; j++)
    {
        addrr[76 - j] = '0' + (a & 1);
        a >>= 1;
    }

    j = (1 << type) - 1;
    for (m = 10; m < 42; m++)
        addrr[m] = '0';
    n = ad & (~j & 0x1f);
    for (m = n; m < n + (1 << type); m++)
        addrr[m + 10] = '1';

    ctrl[2] = '0';
    ctrl[3] = '0';
    urj_part_set_instruction (p, "CONTROLL");
    urj_tap_chain_shift_instructions (chain);
    j = strlen (ctrl);
    k = 0;
    while (j > 0)
    {
        j--;
        p->active_instruction->data_register->in->data[j] = ctrl[k++] & 1;
    }
    urj_tap_chain_shift_data_registers (chain, 1);


    if (read)
    {
        addrr[7] = '0';
        addrr[8] = '0';
        addrr[9] = '0';
    }
    else
    {
        addrr[7] = '0';
        addrr[8] = '1';
        addrr[9] = '0';
    }

    urj_part_set_instruction (p, "ADDR");
    urj_tap_chain_shift_instructions (chain);
    j = strlen (addrr);
    k = 0;
    while (j > 0)
    {
        j--;
        p->active_instruction->data_register->in->data[j] = addrr[k++] & 1;
    }
    urj_tap_chain_shift_data_registers (chain, 0);

    if (!read)
    {
        urj_part_set_instruction (p, "DATA");
        urj_tap_chain_shift_instructions (chain);
        for (j = 0; j < 277; j++)
            p->active_instruction->data_register->in->data[j] = j & 1;
        p->active_instruction->data_register->in->data[259] = 1;
        p->active_instruction->data_register->in->data[258] = 0;
        p->active_instruction->data_register->in->data[257] = 0;
        p->active_instruction->data_register->in->data[256] = 1;
        j = 0;
        if (type < 5)
        {
            k = 256 - (n + (1 << type)) * 8;
            while (j < (8 << type))
            {
                p->active_instruction->data_register->in->data[k + j] =
                    da & 1;
                da >>= 1;
                j++;
            }
        }
        else
        {
            int r;
            for (r = 0; r < 32; r++)
            {
                int s, t;
                t = buf[r];
                for (s = 0; s < 8; s++)
                {
                    p->active_instruction->data_register->in->data[248 -
                                                                   r * 8 +
                                                                   s] = t & 1;
                    t >>= 1;
                }
            }
        }
        urj_tap_chain_shift_data_registers (chain, 0);
    }


    ctrl[2] = '1';
    if (!read)
        ctrl[3] = '1';
    urj_part_set_instruction (p, "CONTROLL");
    urj_tap_chain_shift_instructions (chain);
    j = strlen (ctrl);
    k = 0;
    while (j > 0)
    {
        j--;
        p->active_instruction->data_register->in->data[j] = ctrl[k++] & 1;
    }
    urj_tap_chain_shift_data_registers (chain, 1);
    if (urj_log_state.level <= URJ_LOG_LEVEL_DETAIL || read)
    {
        volatile int q;
        int to;
        const urj_tap_register_t *out;

        out = p->active_instruction->data_register->out;

        to = 5;
        for (q = 0; q < 100; q++);
        urj_part_set_instruction (p, "DATA");
        urj_tap_chain_shift_instructions (chain);
        urj_tap_chain_shift_data_registers (chain, 1);

        while ((out->data[276 - 17] == 0) && to--)
        {
            urj_tap_chain_shift_data_registers (chain, 1);
        }
        for (j = n; j < n + (1 << type); j++)
        {
            buf[j] = 0;
            for (m = 0; m < 8; m++)
            {
                buf[j] <<= 1;
                buf[j] += out->data[255 - (j * 8) - m] & 1;
            }
            urj_log (URJ_LOG_LEVEL_DETAIL, "%02x ", buf[j]);
        }
        if (urj_log_state.level <= URJ_LOG_LEVEL_DETAIL)
        {
            urj_log (URJ_LOG_LEVEL_DETAIL, "\n");

            urj_log (URJ_LOG_LEVEL_DETAIL, " status:\n");
            for (j = 0; j < 21; j++)
            {
                urj_log (URJ_LOG_LEVEL_DETAIL, "%c", '0' + out->data[276 - j]);
                if ((j == 5) || (j == 11) || (j == 12) || (j == 16)
                    || (j == 17))
                    urj_log (URJ_LOG_LEVEL_DETAIL, " ");
            }
            urj_log (URJ_LOG_LEVEL_DETAIL, "\n");
        }
    }
    return URJ_STATUS_OK;
}

/**
 * bus->driver->(*read_start)
 *
 */
static void
bcm1250_bus_read_start (urj_bus_t *bus, uint32_t adr)
{
    addr = adr;
}

/**
 * bus->driver->(*read)
 *
 */
static uint32_t
bcm1250_bus_read (urj_bus_t *bus, uint32_t adr)
{
    unsigned char buf[32];
    bcm1250_ejtag_do (bus, adr + base, 0, 1, 0, buf, 0);
    return buf[adr & 0x1f];

}

/**
 * bus->driver->(*read_next)
 *
 */
static uint32_t
bcm1250_bus_read_next (urj_bus_t *bus, uint32_t adr)
{
    uint32_t t;
    t = bcm1250_bus_read (bus, addr);
    addr = adr;
    return t;
}

/**
 * bus->driver->(*read_end)
 *
 */
static uint32_t
bcm1250_bus_read_end (urj_bus_t *bus)
{
    return bcm1250_bus_read (bus, addr);
}

/**
 * bus->driver->(*write)
 *
 */
static void
bcm1250_bus_write (urj_bus_t *bus, uint32_t adr, uint32_t data)
{
    unsigned char buf[32];
    bcm1250_ejtag_do (bus, adr + base, data, 0, 0, buf, 0);
}

#endif /* #else #ifndef USE_BCM_EJTAG */

const urj_bus_driver_t urj_bus_bcm1250_bus = {
    "bcm1250",
    N_("Broadcom BCM1250 compatible bus driver via BSR"),
    bcm1250_bus_new,
    urj_bus_generic_free,
    bcm1250_bus_printinfo,
    urj_bus_generic_prepare_extest,
    bcm1250_bus_area,
    bcm1250_bus_read_start,
    bcm1250_bus_read_next,
    bcm1250_bus_read_end,
#ifndef USE_BCM_EJTAG
    urj_bus_generic_read,
#else
    bcm1250_bus_read,
#endif
    urj_bus_generic_write_start,
    bcm1250_bus_write,
    urj_bus_generic_no_init,
    urj_bus_generic_no_enable,
    urj_bus_generic_no_disable,
    URJ_BUS_TYPE_PARALLEL,
};
