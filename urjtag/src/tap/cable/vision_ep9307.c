/*
 * $Id$
 *
 * Vision EP9307 SoM GPIO JTAG Cable Driver
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
 * Written by H Hartley Sweeten <hsweeten@visionengravers.com>, 2007, 2008.
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

#include <urjtag/cmd.h>

#define SYSCON_BASE             0x80930000
#define SYSCON_DEVICE_CONFIG    0x80
#define SYSCON_SWLOCK           0xC0

#define SYSCON_DEVCFG_HonIDE    (1 << 11)

#define GPIO_BASE               0x80840000
#define GPIO_PHDR               0x40
#define GPIO_PHDDR              0x44

#define TDO                     4
#define TDI                     6
#define TMS                     5
#define TCK                     7
#define TRST                    3

#define HGPIO(b)                (1 << (b))
#define EP9307_TDO              HGPIO(TDO)
#define EP9307_TDI              HGPIO(TDI)
#define EP9307_TMS              HGPIO(TMS)
#define EP9307_TCK              HGPIO(TCK)
#define EP9307_TRST             HGPIO(TRST)

#define GPIO_INPUT_MASK         ((EP9307_TCK)|(EP9307_TMS)|(EP9307_TDI)|(EP9307_TRST))
#define GPIO_OUTPUT_MASK        (~(EP9307_TDO))
#define GPIO_BITMASK            (~((EP9307_TDO)|(EP9307_TDI)|(EP9307_TMS)|(EP9307_TCK)|(EP9307_TRST)))

typedef struct
{
    int fd_dev_mem;
    void *map_base;
    size_t map_size;
    uint32_t *gpio_PHDR;
    uint32_t lastout;
    int signals;
} ep9307_params_t;

static int
ep9307_gpio_open (urj_cable_t *cable)
{
    ep9307_params_t *p = cable->params;
    off_t map_mask;
    uint32_t *syscon_devcfg;
    uint32_t *syscon_sysswlock;
    uint32_t *gpio_PHDDR;
    uint32_t tmp;

    /* Open the memory device so we can access the hardware registers */
    p->fd_dev_mem = open ("/dev/mem", O_RDWR | O_SYNC);
    if (p->fd_dev_mem == -1)
    {
        urj_error_IO_set (_("unable to open /dev/mem"));
        return URJ_STATUS_FAIL;
    }

    p->map_size = getpagesize ();
    map_mask = p->map_size - 1;

    /* Map the System Controller registers */
    p->map_base = mmap (0, p->map_size, PROT_READ | PROT_WRITE, MAP_SHARED,
                        p->fd_dev_mem, SYSCON_BASE & ~map_mask);
    if (p->map_base == MAP_FAILED)
    {
        urj_error_IO_set (_("unable to mmap the System Control registers"));
        close (p->fd_dev_mem);
        return URJ_STATUS_FAIL;
    }

    /* Create the pointers to access the DeviceCfg and SysSWLock registers */
    syscon_devcfg = (uint32_t *) ((char *) p->map_base + SYSCON_DEVICE_CONFIG);
    syscon_sysswlock = (uint32_t *) ((char *) p->map_base + SYSCON_SWLOCK);

    /* Set the HonIDE bit in the DeviceCfg register so we can use Port H as GPIO */
    tmp = *((uint32_t *) syscon_devcfg);
    tmp |= SYSCON_DEVCFG_HonIDE;

    /* The DeviceCfg register has a SoftwareLock; unlock it first */
    *((uint32_t *) syscon_sysswlock) = 0xAA;
    *((uint32_t *) syscon_devcfg) = tmp;

    /* Unmap the System Controller registers */
    if (munmap (p->map_base, p->map_size) == -1)
    {
        urj_error_IO_set (_("unable to munmap the System Controller registers"));
        close (p->fd_dev_mem);
        return URJ_STATUS_FAIL;
    }

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

    /* Create the pointers to access the PHDR and PHDDR registers */
    p->gpio_PHDR = (uint32_t *) ((char *) p->map_base + GPIO_PHDR);
    gpio_PHDDR = (uint32_t *) ((char *) p->map_base + GPIO_PHDDR);

    /* Set the GPIO pins as inputs/outputs as needed for the JTAG interface */
    tmp = *((uint32_t *) gpio_PHDDR);
    tmp |= GPIO_INPUT_MASK;
    tmp &= GPIO_OUTPUT_MASK;
    *((uint32_t *) gpio_PHDDR) = tmp;

    return URJ_STATUS_OK;
}

static int
ep9307_gpio_close (urj_cable_t *cable)
{
    ep9307_params_t *p = cable->params;

    /* Unmap the GPIO registers */
    if (munmap (p->map_base, p->map_size) == -1)
    {
        urj_error_IO_set (_("unable to munmap the GPIO registers"));
    }
    close (p->fd_dev_mem);
    return URJ_STATUS_OK;
}

static int
ep9307_gpio_write (urj_cable_t *cable, uint8_t data)
{
    ep9307_params_t *p = cable->params;
    uint32_t tmp;

    tmp = *p->gpio_PHDR;
    tmp &= ~GPIO_OUTPUT_MASK;
    tmp |= data;
    *p->gpio_PHDR = tmp;
    p->lastout = tmp;

    return 0;
}

static int
ep9307_gpio_read (urj_cable_t *cable)
{
    ep9307_params_t *p = cable->params;
    uint32_t tmp;

    tmp = *p->gpio_PHDR;

    return tmp;
}

static int
ep9307_connect (urj_cable_t *cable, const urj_param_t *params[])
{
    ep9307_params_t *cable_params;

    if (urj_param_num (params) != 0)
    {
        urj_error_set (URJ_ERROR_SYNTAX,
                       _("This cable type does not accept parameters"));
        return URJ_STATUS_FAIL;
    }

    urj_log (URJ_LOG_LEVEL_NORMAL,
             _("Initializing Vision EP9307 SoM GPIO JTAG Cable\n"));

    cable_params = malloc (sizeof *cable_params);
    if (!cable_params)
    {
        urj_error_set (URJ_ERROR_OUT_OF_MEMORY, _("malloc(%zd) fails"),
                       sizeof *cable_params);
        return URJ_STATUS_FAIL;
    }

    cable->link.port = NULL;
    cable->params = cable_params;
    cable->chain = NULL;

    return URJ_STATUS_OK;
}

static void
ep9307_cable_free (urj_cable_t *cable)
{
    free (cable->params);
    free (cable);
}

static int
ep9307_init (urj_cable_t *cable)
{
    ep9307_params_t *p = cable->params;

    if (ep9307_gpio_open (cable) != URJ_STATUS_OK)
        return URJ_STATUS_FAIL;

    ep9307_gpio_write (cable, 1 << TRST);
    urj_tap_cable_wait (cable);
    p->signals = URJ_POD_CS_TRST;

    return URJ_STATUS_OK;
}

static void
ep9307_done (urj_cable_t *cable)
{
    ep9307_gpio_close (cable);
}

static void
ep9307_clock (urj_cable_t *cable, int tms, int tdi, int n)
{
    ep9307_params_t *p = cable->params;
    int bit_mask;
    int i;
    int trst;

    tms = tms ? 1 : 0;
    tdi = tdi ? 1 : 0;
    trst = (p->signals & URJ_POD_CS_TRST) ? 1 : 0;

    bit_mask = (tms << TMS) | (tdi << TDI) | (trst << TRST);

    for (i = 0; i < n; i++)
    {
        ep9307_gpio_write (cable, (0 << TCK) | bit_mask);
        urj_tap_cable_wait (cable);
        ep9307_gpio_write (cable, (1 << TCK) | bit_mask);
        urj_tap_cable_wait (cable);
    }
}

/**
 * NOTE: This also lowers the TDI and TMS lines; is this intended?
 */
static int
ep9307_get_tdo (urj_cable_t *cable)
{
    ep9307_params_t *p = cable->params;
    int trst;

    trst = (p->signals & URJ_POD_CS_TRST) ? 1 : 0;

    ep9307_gpio_write (cable, (0 << TCK) | (trst << TRST));
    urj_tap_cable_wait (cable);

    return (ep9307_gpio_read (cable) >> TDO) & 1;
}

static int
ep9307_current_signals (urj_cable_t *cable)
{
    ep9307_params_t *p = cable->params;

    int sigs = p->signals & ~(URJ_POD_CS_TMS | URJ_POD_CS_TDI | URJ_POD_CS_TCK
                              | URJ_POD_CS_TRST);
    if (p->lastout & (1 << TCK))
        sigs |= URJ_POD_CS_TCK;
    if (p->lastout & (1 << TDI))
        sigs |= URJ_POD_CS_TDI;
    if (p->lastout & (1 << TMS))
        sigs |= URJ_POD_CS_TMS;
    if (p->lastout & (1 << TRST))
        sigs |= URJ_POD_CS_TRST;

    return sigs;
}

static int
ep9307_set_signal (urj_cable_t *cable, int mask, int val)
{
    int prev_sigs = ep9307_current_signals (cable);

    mask &= (URJ_POD_CS_TMS | URJ_POD_CS_TDI | URJ_POD_CS_TCK | URJ_POD_CS_TRST);   // only these can be modified

    if (mask != 0)
    {
        int sigs = (prev_sigs & ~mask) | (val & mask);
        int tms = (sigs & URJ_POD_CS_TMS) ? (1 << TMS) : 0;
        int tdi = (sigs & URJ_POD_CS_TDI) ? (1 << TDI) : 0;
        int tck = (sigs & URJ_POD_CS_TCK) ? (1 << TCK) : 0;
        int trst = (sigs & URJ_POD_CS_TRST) ? (1 << TRST) : 0;
        ep9307_gpio_write (cable, tms | tdi | tck | trst);
    }

    return prev_sigs;
}

static int
ep9307_get_signal (urj_cable_t *cable, urj_pod_sigsel_t sig)
{
    return (ep9307_current_signals (cable) & sig) ? 1 : 0;
}

static void
ep9307_help (urj_log_level_t ll, const char *cablename)
{
    urj_log (ll, _("Usage: cable %s\n" "\n"), cablename);
}

const urj_cable_driver_t urj_tap_cable_ep9307_driver = {
    "EP9307",
    N_("Vision EP9307 SoM GPIO JTAG Cable"),
    URJ_CABLE_DEVICE_OTHER,
    { .other = ep9307_connect, },
    urj_tap_cable_generic_disconnect,
    ep9307_cable_free,
    ep9307_init,
    ep9307_done,
    urj_tap_cable_generic_set_frequency,
    ep9307_clock,
    ep9307_get_tdo,
    urj_tap_cable_generic_transfer,
    ep9307_set_signal,
    ep9307_get_signal,
    urj_tap_cable_generic_flush_one_by_one,
    ep9307_help
};
