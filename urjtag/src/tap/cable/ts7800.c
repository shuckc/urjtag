/*
 * $Id$
 *
 * Technologic Systems TS-7800 SoC GPIO JTAG Cable Driver
 * Copyright (C) 2008 Catalin Ionescu
 * Based on Vision EP9307 SoM GPIO JTAG Cable Driver
 * Copyright (C) 2007, 2008 H Hartley Sweeten
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
 * Written by Catalin Ionescu <catalin.ionescu@radioconsult.ro>, 2008
 *
 */

#include <sysdep.h>

#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

#include <urjtag/cable.h>
#include <urjtag/chain.h>

#include "generic.h"

#include <cmd.h>

#define GPIO_BASE               0xF1010000
#define GPIO_OUT                (0x0100/4)
#define GPIO_DIR                (0x0104/4)
#define GPIO_INPOL              (0x010C/4)
#define GPIO_IN                 (0x0110/4)
#define GPIO_INTEDGE            (0x0118/4)
#define GPIO_INTLEV             (0x011C/4)

#define TDO                     4
#define TDI                     2
#define TMS                     5
#define TCK                     1

#define HGPIO(b)                (1 << (b))
#define ts7800_TDO              HGPIO(TDO)
#define ts7800_TDI              HGPIO(TDI)
#define ts7800_TMS              HGPIO(TMS)
#define ts7800_TCK              HGPIO(TCK)

#define GPIO_INPUT_MASK         ((ts7800_TCK)|(ts7800_TMS)|(ts7800_TDI))
#define GPIO_OUTPUT_MASK        ts7800_TDO
#define GPIO_BITMASK            (~((ts7800_TDO)|(ts7800_TDI)|(ts7800_TMS)|(ts7800_TCK)))

typedef struct
{
    int fd_dev_mem;
    void *map_base;
    size_t map_size;
    uint32_t *gpio_base;
    int signals;
    uint32_t lastout;
} ts7800_params_t;

static int
ts7800_gpio_open (urj_cable_t *cable)
{
    ts7800_params_t *p = cable->params;
    off_t map_mask;

    /* Open the memory device so we can access the hardware registers */
    p->fd_dev_mem = open ("/dev/mem", O_RDWR | O_SYNC);
    if (p->fd_dev_mem == -1)
    {
        urj_error_IO_set (_("unable to open /dev/mem"));
        return URJ_STATUS_FAIL;
    }

    p->map_size = getpagesize ();
    map_mask = p->map_size - 1;

    /* Map the GPIO registers */
    p->map_base =
        mmap (0, p->map_size, PROT_READ | PROT_WRITE, MAP_SHARED,
              p->fd_dev_mem, GPIO_BASE & ~map_mask);
    if (p->map_base == MAP_FAILED)
    {
        urj_error_IO_set (_("unable to mmap the GPIO registers"));
        close (p->fd_dev_mem);
        return URJ_STATUS_FAIL;
    }

    /* Create the pointers to access the GPIO registers */
    p->gpio_base = p->map_base;

    /* Set the GPIO pins as inputs/outputs as needed for the JTAG interface */
    p->gpio_base[GPIO_DIR] =
        (p->gpio_base[GPIO_DIR] & GPIO_BITMASK) | GPIO_OUTPUT_MASK;

    p->lastout = p->gpio_base[GPIO_OUT];

    return URJ_STATUS_OK;
}

static int
ts7800_gpio_close (urj_cable_t *cable)
{
    ts7800_params_t *p = cable->params;

    /* Unmap the GPIO registers */
    if (munmap (p->map_base, p->map_size) == -1)
    {
        urj_error_IO_set (_("unable to munmap the GPIO registers"));
    }
    close (p->fd_dev_mem);
    return URJ_STATUS_OK;
}

static int
ts7800_gpio_write (urj_cable_t *cable, uint8_t data)
{
    ts7800_params_t *p = cable->params;

    p->gpio_base[GPIO_OUT] = p->lastout = (p->lastout & GPIO_BITMASK) | data;
    urj_tap_cable_wait (cable);

    return 0;
}

static int
ts7800_gpio_read (urj_cable_t *cable)
{
    ts7800_params_t *p = cable->params;

    return p->gpio_base[GPIO_IN];
}

static int
ts7800_connect (urj_cable_t *cable, const urj_param_t *params[])
{
    ts7800_params_t *cable_params;

    if (urj_param_num (params) > 0)
    {
        urj_error_set (URJ_ERROR_SYNTAX,
                       _("This cable type does not accept parameters"));
        return URJ_STATUS_FAIL;
    }

    urj_log (URJ_LOG_LEVEL_NORMAL,
             _("Initializing TS-7800 Built-in JTAG Chain\n"));

    cable_params = malloc (sizeof *cable_params);
    if (!cable_params)
    {
        urj_error_set (URJ_ERROR_OUT_OF_MEMORY, _("malloc(%zd) fails"),
                       sizeof *cable_params);
        free (cable);
        return URJ_STATUS_FAIL;
    }

    cable->params = cable_params;
    cable->chain = NULL;
    cable->delay = 1000;

    return URJ_STATUS_OK;
}

static void
ts7800_disconnect (urj_cable_t *cable)
{
    ts7800_gpio_close (cable);
    urj_tap_chain_disconnect (cable->chain);
}

static void
ts7800_cable_free (urj_cable_t *cable)
{
    free (cable->params);
    free (cable);
}

static int
ts7800_init (urj_cable_t *cable)
{
    ts7800_params_t *p = cable->params;

    if (ts7800_gpio_open (cable) != URJ_STATUS_OK)
        return URJ_STATUS_FAIL;

    p->signals = URJ_POD_CS_TRST;

    return URJ_STATUS_OK;
}

static void
ts7800_done (urj_cable_t *cable)
{
    ts7800_gpio_close (cable);
}

static void
ts7800_clock (urj_cable_t *cable, int tms, int tdi, int n)
{
    int bit_mask;
    int i;

    tms = tms ? 1 : 0;
    tdi = tdi ? 1 : 0;

    bit_mask = (tms << TMS) | (tdi << TDI);

    for (i = 0; i < n; i++)
    {
        ts7800_gpio_write (cable, (0 << TCK) | bit_mask);
        ts7800_gpio_write (cable, (1 << TCK) | bit_mask);
        ts7800_gpio_write (cable, (0 << TCK) | bit_mask);
    }
}

/**
 * NOTE: This also lowers the TDI and TMS lines; is this intended?
 */
static int
ts7800_get_tdo (urj_cable_t *cable)
{
    ts7800_params_t *p = cable->params;
    ts7800_gpio_write (cable, p->lastout & ~(0 << TCK));

    return (ts7800_gpio_read (cable) >> TDO) & 1;
}

static int
ts7800_current_signals (urj_cable_t *cable)
{
    ts7800_params_t *p = cable->params;
    int sigs = p->signals & ~(URJ_POD_CS_TMS | URJ_POD_CS_TDI | URJ_POD_CS_TCK);

    if (p->lastout & (1 << TCK))
        sigs |= URJ_POD_CS_TCK;
    if (p->lastout & (1 << TDI))
        sigs |= URJ_POD_CS_TDI;
    if (p->lastout & (1 << TMS))
        sigs |= URJ_POD_CS_TMS;

    return sigs;
}

static int
ts7800_set_signal (urj_cable_t *cable, int mask, int val)
{
    int prev_sigs = ts7800_current_signals (cable);

    mask &= (URJ_POD_CS_TDI | URJ_POD_CS_TCK | URJ_POD_CS_TMS); // only these can be modified

    if (mask != 0)
    {
        int sigs = (prev_sigs & ~mask) | (val & mask);
        int tms = (sigs & URJ_POD_CS_TMS) ? (1 << TMS) : 0;
        int tdi = (sigs & URJ_POD_CS_TDI) ? (1 << TDI) : 0;
        int tck = (sigs & URJ_POD_CS_TCK) ? (1 << TCK) : 0;
        ts7800_gpio_write (cable, tms | tdi | tck);
    }

    return prev_sigs;
}

static int
ts7800_get_signal (urj_cable_t *cable, urj_pod_sigsel_t sig)
{
    return (ts7800_current_signals (cable) & sig) ? 1 : 0;
}

static void
ts7800_help (urj_log_level_t ll, const char *cablename)
{
    urj_log (ll,
             _("Usage: cable %s\n" "\n"), cablename);
}

const urj_cable_driver_t urj_tap_cable_ts7800_driver = {
    "ts7800",
    N_("TS-7800 Built-in JTAG Chain"),
    URJ_CABLE_DEVICE_OTHER,
    { .other = ts7800_connect, },
    ts7800_disconnect,
    ts7800_cable_free,
    ts7800_init,
    ts7800_done,
    urj_tap_cable_generic_set_frequency,
    ts7800_clock,
    ts7800_get_tdo,
    urj_tap_cable_generic_transfer,
    ts7800_set_signal,
    ts7800_get_signal,
    urj_tap_cable_generic_flush_one_by_one,
    ts7800_help
};
