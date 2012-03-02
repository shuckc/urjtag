/*
 * $Id$
 *
 * Intel PXA2x0 compatible bus driver via BSR
 * Copyright (C) 2002, 2003 ETC s.r.o.
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
 * Written by Marcel Telka <marcel@telka.sk>, 2002, 2003.
 *
 * 2005-01-29: Cliff Brake <cliff.brake@gmail.com> <http://bec-systems.com>
 *   - added support for PXA270
 *
 * Documentation:
 * [1] Intel Corporation, "Intel PXA250 and PXA210 Application Processors
 *     Developer's Manual", February 2002, Order Number: 278522-001
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
#include <urjtag/tap_state.h>

#include "buses.h"
#include "generic_bus.h"

#include "pxa2x0_mc.h"


/*
 * the following defines are used in proc field of the the
 * bus_params_t structure and are used in various functions
 * below
 */

#define PROC_PXA25x     1       // including px26x series
#define PROC_PXA27x     2


#define nCS_TOTAL 6

typedef struct
{
    char *sig_name;
    int enabled;
    int bus_width;              // set 0 for disabled (or auto-detect)
    char label_buf[81];
} ncs_map_entry;

/*
 * Tables indexed by nCS[index]
 * An array of plain char* would probably do it too, but anyway...
 *
 * Note: the setup of nCS[*] is board-specific, rather than chip-specific!
 * The memory mapping and nCS[*] functions are normally set up by the boot loader.
 * In our JTAG code, we manipulate the outer pins explicitly, without the help
 * of the CPU's memory controller - hence the need to mimick its setup.
 *
 * Note that URJ_BUS_AREA() and URJ_BUS_READ()/URJ_BUS_WRITE() use a window of 64MB
 * per nCS pin (26bit addresses), which seems to be the most common option.
 * For static CS[0] and CS[1] == 128 MB, the algorithms have to be modified...
 */

// Fool-proof basic mapping with only nCS[0] wired.
// nCS[0] doesn't collide with any other GPIO functions.
static const ncs_map_entry pxa25x_ncs_map[nCS_TOTAL] = {
    {"nCS[0]", 1, 0},
    {NULL, 0, 0},
    {NULL, 0, 0},
    {NULL, 0, 0},
    {NULL, 0, 0},
    {NULL, 0, 0}
};

// Default mapping with all nCS[*] GPIO pins used as nCS.
// Note that the same GPIO pins might be used e.g. for PCCard
// service space access or PWM outputs, or some other purpose.
static const ncs_map_entry pxa27x_ncs_map[nCS_TOTAL] = {
    {"nCS[0]", 1, 0},           // nCS[0]
    {"GPIO[15]", 1, 16},        // nCS[1]
    {"GPIO[78]", 1, 16},        // nCS[2]
    {"GPIO[79]", 1, 16},        // nCS[3]
    {"GPIO[80]", 1, 16},        // nCS[4]
    {"GPIO[33]", 1, 16}         // nCS[5]
};


typedef struct
{
    uint32_t last_adr;
    urj_part_signal_t *ma[26];
    urj_part_signal_t *md[32];
    urj_part_signal_t *ncs[nCS_TOTAL];
    urj_part_signal_t *dqm[4];
    urj_part_signal_t *rdnwr;
    urj_part_signal_t *nwe;
    urj_part_signal_t *noe;
    urj_part_signal_t *nsdcas;
    MC_registers_t MC_registers;
    int inited;
    int proc;
    ncs_map_entry ncs_map[nCS_TOTAL];
} bus_params_t;

#define PROC            ((bus_params_t *) bus->params)->proc
#define LAST_ADR        ((bus_params_t *) bus->params)->last_adr
#define MA              ((bus_params_t *) bus->params)->ma
#define MD              ((bus_params_t *) bus->params)->md
#define nCS             ((bus_params_t *) bus->params)->ncs
#define DQM             ((bus_params_t *) bus->params)->dqm
#define RDnWR           ((bus_params_t *) bus->params)->rdnwr
#define nWE             ((bus_params_t *) bus->params)->nwe
#define nOE             ((bus_params_t *) bus->params)->noe
#define nSDCAS          ((bus_params_t *) bus->params)->nsdcas

#define MC_pointer      (&((bus_params_t *) bus->params)->MC_registers)

#define INITED          ((bus_params_t *) bus->params)->inited

#define NCS_MAP         ((bus_params_t *) bus->params)->ncs_map

/**
 * bus->driver->(*new_bus)
 *
 */
static urj_bus_t *
pxa2xx_bus_new (urj_chain_t *chain, const urj_bus_driver_t *driver,
                const urj_param_t *cmd_params[])
{
    urj_part_t *part;
    urj_bus_t *bus;
    const ncs_map_entry *ncs_map;
    char buff[10];
    int i;
    int failed = 0;

    if (!chain || !chain->parts || chain->parts->len <= chain->active_part
        || chain->active_part < 0)
        return NULL;

    bus = urj_bus_generic_new (chain, driver, sizeof (bus_params_t));
    if (bus == NULL)
        return NULL;
    part = bus->part;

    if (strcmp (driver->name, "pxa2x0") == 0)
        PROC = PROC_PXA25x;
    else if (strcmp (driver->name, "pxa27x") == 0)
        PROC = PROC_PXA27x;
    else
    {
        urj_bus_generic_free (bus);
        urj_error_set (URJ_ERROR_SYNTAX,
                       "driver must be 'pxa2x0' or 'pxa27x', not '%s'",
                       driver->name);
        return NULL;
    }

    for (i = 0; i < 26; i++)
    {
        sprintf (buff, "MA[%d]", i);
        failed |= urj_bus_generic_attach_sig (part, &(MA[i]), buff);
    }

    for (i = 0; i < 32; i++)
    {
        sprintf (buff, "MD[%d]", i);
        failed |= urj_bus_generic_attach_sig (part, &(MD[i]), buff);
    }

    if (PROC == PROC_PXA25x)
    {
        ncs_map = pxa25x_ncs_map;
    }
    else if (PROC == PROC_PXA27x)
    {
        ncs_map = pxa27x_ncs_map;
    }
    else
    {
        urj_error_set (URJ_ERROR_INVALID, "processor type %d", PROC);
        ncs_map = pxa25x_ncs_map;       // be dumb by default
    }
    for (i = 0; i < nCS_TOTAL; i++)
    {
        NCS_MAP[i] = ncs_map[i];

        if (ncs_map[i].enabled > 0)
        {
            failed |=
                urj_bus_generic_attach_sig (part, &(nCS[i]),
                                            NCS_MAP[i].sig_name);
        }
        else                    // disabled - this GPIO pin is unused or used for some other function
        {
            nCS[i] = NULL;
        }
    }

    for (i = 0; i < 4; i++)
    {
        sprintf (buff, "DQM[%d]", i);
        failed |= urj_bus_generic_attach_sig (part, &(DQM[i]), buff);
    }

    failed |= urj_bus_generic_attach_sig (part, &(RDnWR), "RDnWR");

    failed |= urj_bus_generic_attach_sig (part, &(nWE), "nWE");

    failed |= urj_bus_generic_attach_sig (part, &(nOE), "nOE");

    failed |= urj_bus_generic_attach_sig (part, &(nSDCAS), "nSDCAS");

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
pxa2xx_bus_printinfo (urj_log_level_t ll, urj_bus_t *bus)
{
    int i;

    for (i = 0; i < bus->chain->parts->len; i++)
        if (bus->part == bus->chain->parts->parts[i])
            break;
    urj_log (ll, _("%s (JTAG part No. %d)\n"), bus->driver->description, i);
}

/**
 * bus->driver->(*init)
 *
 */
static int
pxa2xx_bus_init (urj_bus_t *bus)
{
    urj_chain_t *chain = bus->chain;
    urj_part_t *p = bus->part;

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
    urj_tap_chain_shift_data_registers (chain, 1);

    if (PROC == PROC_PXA25x)
    {
        const urj_part_signal_t *bs_2 = urj_part_find_signal (p, "BOOT_SEL[2]");
        const urj_part_signal_t *bs_1 = urj_part_find_signal (p, "BOOT_SEL[1]");
        const urj_part_signal_t *bs_0 = urj_part_find_signal (p, "BOOT_SEL[0]");

        BOOT_DEF = BOOT_DEF_PKG_TYPE |
            BOOT_DEF_BOOT_SEL (urj_part_get_signal (p, bs_2) << 2
                               | urj_part_get_signal (p, bs_1) << 1
                               | urj_part_get_signal (p, bs_0));
    }
    else if (PROC == PROC_PXA27x)
    {
        const urj_part_signal_t *bs = urj_part_find_signal (p, "BOOT_SEL");

        BOOT_DEF = BOOT_DEF_PKG_TYPE |
            BOOT_DEF_BOOT_SEL (urj_part_get_signal (p, bs));
    }
    else
    {
        urj_error_set (URJ_ERROR_INVALID, "processor type %d", PROC);
        return URJ_STATUS_FAIL;
    }

    urj_part_set_instruction (p, "BYPASS");
    urj_tap_chain_shift_instructions (chain);

    bus->initialized = 1;

    return URJ_STATUS_OK;
}

/**
 * bus->driver->(*area)
 *
 */
static int
pxa2xx_bus_area (urj_bus_t *bus, uint32_t adr, urj_bus_area_t *area)
{
    uint32_t tmp_addr;
    int ncs_index;

    /* Static Chip Select 0 (64 MB) */
    if (adr < UINT32_C (0x04000000))
    {
        area->description = N_("Static Chip Select 0");
        area->start = UINT32_C (0x00000000);
        area->length = UINT64_C (0x04000000);

        if (NCS_MAP[0].bus_width > 0)
        {
            area->width = NCS_MAP[0].bus_width;
        }
        else
        {
            /* see Table 6-36. in [1] */
            switch (get_BOOT_DEF_BOOT_SEL (BOOT_DEF))
            {
            case 0:
                area->width = 32;
                break;
            case 1:
                area->width = 16;
                break;
            case 2:
            case 3:
                area->width = 0;
                break;
            case 4:
            case 5:
            case 6:
            case 7:
                urj_error_set (URJ_ERROR_UNIMPLEMENTED, "TODO - BOOT_SEL: %lu",
                           (long unsigned) get_BOOT_DEF_BOOT_SEL (BOOT_DEF));
                return URJ_STATUS_FAIL;
            default:
                urj_error_set (URJ_ERROR_INVALID, "BOOT_DEF value %lu",
                           (long unsigned) get_BOOT_DEF_BOOT_SEL (BOOT_DEF));
                return URJ_STATUS_FAIL;
            }
        }
        return URJ_STATUS_OK;
    }

    /* Static Chip Select 1..5 (per 64 MB) */
    for (ncs_index = 1, tmp_addr = 0x04000000; ncs_index <= 5;
         ncs_index++, tmp_addr += 0x04000000)
    {
        if ((adr >= tmp_addr) && (adr < tmp_addr + 0x04000000))
        {                       // if the addr is within our window
            sprintf (NCS_MAP[ncs_index].label_buf,
                     "Static Chip Select %d = %s %s", ncs_index,
                     NCS_MAP[ncs_index].sig_name,
                     NCS_MAP[ncs_index].enabled ? "" : "(disabled)");
            area->description = NCS_MAP[ncs_index].label_buf;
            area->start = tmp_addr;
            area->length = UINT64_C (0x04000000);
            area->width = NCS_MAP[ncs_index].bus_width;

            return URJ_STATUS_OK;
        }
    }

    if (adr < UINT32_C (0x48000000))
    {
        area->description = NULL;
        area->start = UINT32_C (0x18000000);
        area->length = UINT64_C (0x30000000);
        area->width = 0;

        return URJ_STATUS_OK;
    }

    if (adr < UINT32_C (0x4C000000))
    {
        area->description = N_("Memory Mapped registers (Memory Ctl)");
        area->start = UINT32_C (0x48000000);
        area->length = UINT64_C (0x04000000);
        area->width = 32;

        return URJ_STATUS_OK;
    }

    area->description = NULL;
    area->start = UINT32_C (0x4C000000);
    area->length = UINT64_C (0xB4000000);
    area->width = 0;

    return URJ_STATUS_OK;
}

/**
 * bus->driver->(*area)
 *
 */
static int
pxa27x_bus_area (urj_bus_t *bus, uint32_t adr, urj_bus_area_t *area)
{
    uint32_t tmp_addr;
    int ncs_index;

    /* Static Chip Select 0 (64 MB) */
    if (adr < UINT32_C (0x04000000))
    {
        area->description = N_("Static Chip Select 0");
        area->start = UINT32_C (0x00000000);
        area->length = UINT64_C (0x04000000);

        if (NCS_MAP[0].bus_width > 0)
        {
            area->width = NCS_MAP[0].bus_width;
        }
        else
        {
            /* see Table 6-36. in [1] */
            switch (get_BOOT_DEF_BOOT_SEL (BOOT_DEF))
            {
            case 0:
                area->width = 32;
                break;
            case 1:
                area->width = 16;
                break;
            case 2:
            case 3:
                area->width = 0;
                break;
            case 4:
            case 5:
            case 6:
            case 7:
                urj_error_set (URJ_ERROR_UNIMPLEMENTED, "TODO - BOOT_SEL: %lu",
                        (long unsigned) get_BOOT_DEF_BOOT_SEL (BOOT_DEF));
                return URJ_STATUS_FAIL;
            default:
                urj_error_set (URJ_ERROR_INVALID, "BOOT_SEL: %lu",
                        (long unsigned) get_BOOT_DEF_BOOT_SEL (BOOT_DEF));
                return URJ_STATUS_FAIL;
            }
        }
        return URJ_STATUS_OK;
    }

    /* Static Chip Select 1..5 (per 64 MB) */
    for (ncs_index = 1, tmp_addr = 0x04000000; ncs_index <= 5;
         ncs_index++, tmp_addr += 0x04000000)
    {
        urj_log (URJ_LOG_LEVEL_DEBUG, "Checking area %08lX - %08lX... ",
                 (unsigned long)tmp_addr,
                 (unsigned long)tmp_addr + 0x04000000 - 1);
        if ((adr >= tmp_addr) && (adr < tmp_addr + 0x04000000))
        {                       // if the addr is within our window
            urj_log (URJ_LOG_LEVEL_DEBUG, "match\n");
            sprintf (NCS_MAP[ncs_index].label_buf,
                     "Static Chip Select %d = %s %s", ncs_index,
                     NCS_MAP[ncs_index].sig_name,
                     NCS_MAP[ncs_index].enabled ? "" : "(disabled)");
            area->description = NCS_MAP[ncs_index].label_buf;
            area->start = tmp_addr;
            area->length = UINT64_C (0x04000000);
            area->width = NCS_MAP[ncs_index].bus_width;

            return URJ_STATUS_OK;
        }
        urj_log (URJ_LOG_LEVEL_DEBUG, "no match\n");
    }

    if (adr < UINT32_C (0x40000000))
    {
        area->description = NULL;
        area->start = UINT32_C (0x18000000);
        area->length = UINT64_C (0x28000000);
        area->width = 0;

        return URJ_STATUS_OK;
    }

    if (adr < UINT32_C (0x60000000))
    {
        area->description = N_("PXA270 internal address space (cfg, SRAM)");
        area->start = UINT32_C (0x40000000);
        area->length = UINT64_C (0x20000000);
        area->width = 32;

        return URJ_STATUS_OK;
    }

    if (adr < UINT32_C (0xA0000000))
    {
        area->description = NULL;
        area->start = UINT32_C (0x60000000);
        area->length = UINT64_C (0x40000000);
        area->width = 0;

        return URJ_STATUS_OK;
    }

    if (adr < UINT32_C (0xB0000000))
    {
        area->description = N_("PXA270 SDRAM space (4x 64MB)");
        area->start = UINT32_C (0xA0000000);
        area->length = UINT64_C (0x10000000);
        area->width = 32;

        return URJ_STATUS_OK;
    }

    area->description = NULL;
    area->start = UINT32_C (0xB0000000);
    area->length = UINT64_C (0x50000000);
    area->width = 0;

    return URJ_STATUS_OK;
}

static void
setup_address (urj_bus_t *bus, uint32_t a)
{
    int i;
    urj_part_t *p = bus->part;

    for (i = 0; i < 26; i++)
        urj_part_set_signal (p, MA[i], 1, (a >> i) & 1);
}

static void
set_data_in (urj_bus_t *bus, uint32_t adr)
{
    int i;
    urj_part_t *p = bus->part;
    urj_bus_area_t area;

    bus->driver->area (bus, adr, &area);

    for (i = 0; i < area.width; i++)
        urj_part_set_signal_input (p, MD[i]);
}

static void
setup_data (urj_bus_t *bus, uint32_t adr, uint32_t d)
{
    int i;
    urj_part_t *p = bus->part;
    urj_bus_area_t area;

    bus->driver->area (bus, adr, &area);

    for (i = 0; i < area.width; i++)
        urj_part_set_signal (p, MD[i], 1, (d >> i) & 1);
}

/**
 * bus->driver->(*read_start)
 *
 */
static int
pxa2xx_bus_read_start (urj_bus_t *bus, uint32_t adr)
{
    int cs_index = 0;

    urj_chain_t *chain = bus->chain;
    urj_part_t *p = bus->part;

    LAST_ADR = adr;
    if (adr >= 0x18000000)
    {
        urj_error_set (URJ_ERROR_OUT_OF_BOUNDS, "adr 0x%lx",
                       (long unsigned) adr);
        return URJ_STATUS_FAIL;
    }

    cs_index = adr >> 26;
    if (nCS[cs_index] == NULL)
    {
        urj_error_set (URJ_ERROR_OUT_OF_BOUNDS, "nCS[%d] null", cs_index);
        return URJ_STATUS_FAIL;
    }

    /* see Figure 6-13 in [1] */
    urj_part_set_signal_low (p, nCS[cs_index]);
    urj_part_set_signal_low (p, DQM[0]);
    urj_part_set_signal_low (p, DQM[1]);
    urj_part_set_signal_low (p, DQM[2]);
    urj_part_set_signal_low (p, DQM[3]);
    urj_part_set_signal_high (p, RDnWR);
    urj_part_set_signal_high (p, nWE);
    urj_part_set_signal_low (p, nOE);
    urj_part_set_signal_low (p, nSDCAS);

    setup_address (bus, adr);
    set_data_in (bus, adr);

    urj_tap_chain_shift_data_registers (chain, 0);

    return URJ_STATUS_OK;
}

/**
 * bus->driver->(*read_next)
 *
 */
static uint32_t
pxa2xx_bus_read_next (urj_bus_t *bus, uint32_t adr)
{
    urj_part_t *p = bus->part;
    urj_chain_t *chain = bus->chain;
    uint32_t d;
    uint32_t old_last_adr = LAST_ADR;

    LAST_ADR = adr;

    if (adr < UINT32_C (0x18000000))
    {
        int i;
        urj_bus_area_t area;

        if (nCS[adr >> 26] == NULL)     // avoid undefined nCS windows
            return 0;

        bus->driver->area (bus, adr, &area);

        /* see Figure 6-13 in [1] */
        setup_address (bus, adr);
        urj_tap_chain_shift_data_registers (chain, 1);

        d = 0;
        for (i = 0; i < area.width; i++)
            d |= (uint32_t) (urj_part_get_signal (p, MD[i]) << i);

        return d;
    }

    // anything above 0x18000000 is essentially unreachable...
    if (adr < UINT32_C (0x48000000))
        return 0;

    if (adr < UINT32_C (0x4C000000))
    {
        if (old_last_adr == (MC_BASE + BOOT_DEF_OFFSET))
            return BOOT_DEF;

        return 0;
    }

    return 0;
}

/**
 * bus->driver->(*read_end)
 *
 */
static uint32_t
pxa2xx_bus_read_end (urj_bus_t *bus)
{
    urj_part_t *p = bus->part;
    urj_chain_t *chain = bus->chain;

    if (LAST_ADR < UINT32_C (0x18000000))
    {
        int i;
        uint32_t d = 0;
        urj_bus_area_t area;

        if (nCS[LAST_ADR >> 26] == NULL)        // avoid undefined nCS windows
            return 0;

        bus->driver->area (bus, LAST_ADR, &area);

        /* see Figure 6-13 in [1] */
        urj_part_set_signal_high (p, nCS[0]);
        urj_part_set_signal_high (p, nOE);
        urj_part_set_signal_high (p, nSDCAS);

        urj_tap_chain_shift_data_registers (chain, 1);

        for (i = 0; i < area.width; i++)
            d |= (uint32_t) (urj_part_get_signal (p, MD[i]) << i);

        return d;
    }

    // anything above 0x18000000 is essentially unreachable...
    if (LAST_ADR < UINT32_C (0x48000000))
        return 0;

    if (LAST_ADR < UINT32_C (0x4C000000))
    {
        if (LAST_ADR == (MC_BASE + BOOT_DEF_OFFSET))
            return BOOT_DEF;

        return 0;
    }

    return 0;
}

/**
 * bus->driver->(*write)
 *
 */
static void
pxa2xx_bus_write (urj_bus_t *bus, uint32_t adr, uint32_t data)
{
    int cs_index = 0;

    /* see Figure 6-17 in [1] */
    urj_part_t *p = bus->part;
    urj_chain_t *chain = bus->chain;

    if (adr >= 0x18000000)
        return;

    cs_index = adr >> 26;
    if (nCS[cs_index] == NULL)
        return;

    urj_part_set_signal_low (p, nCS[cs_index]);
    urj_part_set_signal_low (p, DQM[0]);
    urj_part_set_signal_low (p, DQM[1]);
    urj_part_set_signal_low (p, DQM[2]);
    urj_part_set_signal_low (p, DQM[3]);
    urj_part_set_signal_low (p, RDnWR);
    urj_part_set_signal_high (p, nWE);
    urj_part_set_signal_high (p, nOE);
    urj_part_set_signal_low (p, nSDCAS);

    setup_address (bus, adr);
    setup_data (bus, adr, data);

    urj_tap_chain_shift_data_registers (chain, 0);

    urj_part_set_signal_low (p, nWE);
    urj_tap_chain_shift_data_registers (chain, 0);
    urj_part_set_signal_high (p, nWE);
    urj_tap_chain_shift_data_registers (chain, 0);
}

const urj_bus_driver_t urj_bus_pxa2x0_bus = {
    "pxa2x0",
    N_("Intel PXA2x0 compatible bus driver via BSR"),
    pxa2xx_bus_new,
    urj_bus_generic_free,
    pxa2xx_bus_printinfo,
    urj_bus_generic_prepare_extest,
    pxa2xx_bus_area,
    pxa2xx_bus_read_start,
    pxa2xx_bus_read_next,
    pxa2xx_bus_read_end,
    urj_bus_generic_read,
    urj_bus_generic_write_start,
    pxa2xx_bus_write,
    pxa2xx_bus_init,
    urj_bus_generic_no_enable,
    urj_bus_generic_no_disable,
    URJ_BUS_TYPE_PARALLEL,
};

const urj_bus_driver_t urj_bus_pxa27x_bus = {
    "pxa27x",
    N_("Intel PXA27x compatible bus driver via BSR"),
    pxa2xx_bus_new,
    urj_bus_generic_free,
    pxa2xx_bus_printinfo,
    urj_bus_generic_prepare_extest,
    pxa27x_bus_area,
    pxa2xx_bus_read_start,
    pxa2xx_bus_read_next,
    pxa2xx_bus_read_end,
    urj_bus_generic_read,
    urj_bus_generic_write_start,
    pxa2xx_bus_write,
    pxa2xx_bus_init,
    urj_bus_generic_no_enable,
    urj_bus_generic_no_disable,
    URJ_BUS_TYPE_PARALLEL,
};
