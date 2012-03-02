/*
 * $Id$
 *
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
 * Modified by Wojtek Kaniewski <wojtekka@toxygen.net>, 2004.
 * Modified from ppc405ep.c by Detrick Martin <jtag@detrickmartin.net>, 2008.
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
    urj_part_signal_t *a[32];
    urj_part_signal_t *d[32];
    urj_part_signal_t *cs;
    urj_part_signal_t *we;
    urj_part_signal_t *oe;
    int alsbi, amsbi, ai, aw, dlsbi, dmsbi, di, dw, csa, wea, oea;
    int ashift;
} bus_params_t;

#define A       ((bus_params_t *) bus->params)->a
#define D       ((bus_params_t *) bus->params)->d
#define CS      ((bus_params_t *) bus->params)->cs
#define WE      ((bus_params_t *) bus->params)->we
#define OE      ((bus_params_t *) bus->params)->oe

#define ALSBI   ((bus_params_t *) bus->params)->alsbi
#define AMSBI   ((bus_params_t *) bus->params)->amsbi
#define AI      ((bus_params_t *) bus->params)->ai
#define AW      ((bus_params_t *) bus->params)->aw
#define DLSBI   ((bus_params_t *) bus->params)->dlsbi
#define DMSBI   ((bus_params_t *) bus->params)->dmsbi
#define DI      ((bus_params_t *) bus->params)->di
#define DW      ((bus_params_t *) bus->params)->dw
#define CSA     ((bus_params_t *) bus->params)->csa
#define WEA     ((bus_params_t *) bus->params)->wea
#define OEA     ((bus_params_t *) bus->params)->oea

#define ASHIFT ((bus_params_t *) bus->params)->ashift

static void
prototype_bus_signal_parse (const char *str, char *fmt, int *inst)
{
    char pre[16], suf[16];

    switch (sscanf (str, "%[^0-9]%d%s", pre, inst, suf))
    {
    case 1:
        strcpy (fmt, str);
        break;
    case 2:
        sprintf (fmt, "%s%s", pre, "%d");
        break;
    case 3:
        sprintf (fmt, "%s%s%s", pre, "%d", suf);
    }
    // @@@@ RFHH what about failure?
}

/**
 * bus->driver->(*new_bus)
 *
 */
static urj_bus_t *
prototype_bus_new (urj_chain_t *chain, const urj_bus_driver_t *driver,
                   const urj_param_t *cmd_params[])
{
    urj_bus_t *bus;
    urj_part_signal_t *sig;
    char buff[16], fmt[16], afmt[16], dfmt[16];
    int i, j, inst, max, min;
    int failed = 0;
    int ashift = -1;
    const char *value;

    bus = urj_bus_generic_new (chain, driver, sizeof (bus_params_t));
    if (bus == NULL)
        return NULL;

    CS = OE = WE = NULL;
    ALSBI = AMSBI = DLSBI = DMSBI = -1;
    for (i = 0; cmd_params[i] != NULL; i++)
    {
        if (cmd_params[i]->key == URJ_BUS_PARAM_KEY_AMODE ||
            cmd_params[i]->key == URJ_BUS_PARAM_KEY_WIDTH)
        {
            switch (cmd_params[i]->value.lu)
            {
            case 8:
                ashift = 0;
                break;
            case 16:
                ashift = 1;
                break;
            case 32:
                ashift = 2;
                break;
            case 0:     // "auto"
                break;

            default:
                urj_error_set (URJ_ERROR_INVALID,
                               _("value %lu not defined for parameter %s"),
                               cmd_params[i]->value.lu,
                               urj_param_string(&urj_bus_param_list,
                                                cmd_params[i]));
                failed = 1;     // @@@@ RFHH
                break;
            }

        }
        else
        {
            if (cmd_params[i]->type != URJ_PARAM_TYPE_STRING)
            {
                urj_error_set (URJ_ERROR_SYNTAX,
                               "parameter must be of type string");
                failed = 1;
                continue;
            }

            value = cmd_params[i]->value.string;

            inst = 32;
            prototype_bus_signal_parse (value, fmt, &inst);
            // @@@@ RFHH Flag error?
            // @@@@ RFHH If it is mandatory for a signal to have an int inst
            // number, why does prototype_bus_signal_parse() accept values
            // without an int?
            if (inst > 31 || inst < 0)
                continue;

            sig = urj_part_find_signal (bus->part, value);
            if (!sig)
            {
                urj_error_set (URJ_ERROR_NOTFOUND, _("signal '%s' not found"),
                               value);
                failed = 1;
                continue;
            }

            switch (cmd_params[i]->key)
            {
            case URJ_BUS_PARAM_KEY_ALSB:
                ALSBI = inst;
                A[inst] = sig;
                strcpy (afmt, fmt);
                break;
            case URJ_BUS_PARAM_KEY_AMSB:
                AMSBI = inst;
                A[inst] = sig;
                strcpy (afmt, fmt);
                break;
            case URJ_BUS_PARAM_KEY_DLSB:
                DLSBI = inst;
                D[inst] = sig;
                strcpy (dfmt, fmt);
                break;
            case URJ_BUS_PARAM_KEY_DMSB:
                DMSBI = inst;
                D[inst] = sig;
                strcpy (dfmt, fmt);
                break;
            case URJ_BUS_PARAM_KEY_CS:
            case URJ_BUS_PARAM_KEY_NCS:
                CS = sig;
                CSA = (cmd_params[i]->key == URJ_BUS_PARAM_KEY_CS);
                break;
            case URJ_BUS_PARAM_KEY_OE:
            case URJ_BUS_PARAM_KEY_NOE:
                OE = sig;
                OEA = (cmd_params[i]->key == URJ_BUS_PARAM_KEY_OE);
                break;
            case URJ_BUS_PARAM_KEY_WE:
            case URJ_BUS_PARAM_KEY_NWE:
                WE = sig;
                WEA = (cmd_params[i]->key == URJ_BUS_PARAM_KEY_WE);
                break;
            default:
                urj_error_set (URJ_ERROR_INVALID, _("parameter %s is unknown"),
                               urj_param_string(&urj_bus_param_list,
                                                cmd_params[i]));
                failed = 1;
                break;
            }
        }
    }

    if (ALSBI >= 0 || AMSBI >= 0)
    {
        if (ALSBI == -1 || AMSBI == -1)
        {
            for (min = 0; min <= 31; min++)
            {
                sprintf (buff, afmt, min);
                A[min] = urj_part_find_signal (bus->part, buff);
                if (A[min])
                    break;
            }
            for (max = 31; max >= 0; max--)
            {
                sprintf (buff, afmt, max);
                A[max] = urj_part_find_signal (bus->part, buff);
                if (A[max])
                    break;
            }
            if (ALSBI == -1)
                ALSBI = (max - AMSBI < AMSBI - min) ? min : max;
            else
                AMSBI = (max - ALSBI < ALSBI - min) ? min : max;
        }
        AI = (AMSBI > ALSBI ? 1 : -1);
        AW = (AMSBI > ALSBI ? AMSBI - ALSBI : ALSBI - AMSBI) + 1;
        for (i = 0, j = ALSBI; i < AW; i++, j += AI)
        {
            sprintf (buff, afmt, j);
            // @@@@ RFHH check result
            A[j] = urj_part_find_signal (bus->part, buff);
        }
    }
    else
    {
        urj_error_set (URJ_ERROR_INVALID,
            _("parameters alsb=<signal> and/or amsb=<signal> are not defined"));
        failed = 1;
    }

    if (DLSBI >= 0 || DMSBI >= 0)
    {
        if (DLSBI == -1 || DMSBI == -1)
        {
            for (min = 0; min <= 31; min++)
            {
                sprintf (buff, dfmt, min);
                D[min] = urj_part_find_signal (bus->part, buff);
                if (D[min])
                    break;
            }
            for (max = 31; max >= 0; max--)
            {
                sprintf (buff, dfmt, max);
                D[max] = urj_part_find_signal (bus->part, buff);
                if (D[max])
                    break;
            }
            if (DLSBI == -1)
                DLSBI = (max - DMSBI < DMSBI - min) ? min : max;
            else
                DMSBI = (max - DLSBI < DLSBI - min) ? min : max;
        }
        DI = (DMSBI > DLSBI ? 1 : -1);
        DW = (DMSBI > DLSBI ? DMSBI - DLSBI : DLSBI - DMSBI) + 1;
        for (i = 0, j = DLSBI; i < DW; i++, j += DI)
        {
            sprintf (buff, dfmt, j);
            D[j] = urj_part_find_signal (bus->part, buff);
        }

        /* bus drivers are called with a byte address
           this address needs to be adjusted by setup_address() to the memory data width */
        if (ashift < 0)
        {
            int nbytes;

            /* parameter 'amode' wasn't specified, derive the address shift from the
               data bus width */
            nbytes = DW / 8;
            if (DW % 8 > 0)
                nbytes++;

            ashift = 0;
            while (nbytes != 1)
            {
                nbytes >>= 1;
                ashift++;
            }
        }
        ASHIFT = ashift;

    }
    else
    {
        urj_error_set (URJ_ERROR_INVALID,
                       _("parameters dlsb=<signal> and/or dmsb=<signal> are not defined\n"));
        failed = 1;
    }

    if (!CS)
    {
        urj_error_set (URJ_ERROR_INVALID,
                       _("parameter cs=<signal> or ncs=<signal> is not defined\n"));
        failed = 1;
    }

    if (!OE)
    {
        urj_error_set (URJ_ERROR_INVALID,
                       _("parameter oe=<signal> or noe=<signal> is not defined\n"));
        failed = 1;
    }

    if (!WE)
    {
        urj_error_set (URJ_ERROR_INVALID,
                       _("parameter we=<signal> or nwe=<signal> is not defined\n"));
        failed = 1;
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
prototype_bus_printinfo (urj_log_level_t ll, urj_bus_t *bus)
{
    int i;

    for (i = 0; i < bus->chain->parts->len; i++)
        if (bus->part == bus->chain->parts->parts[i])
            break;
    // @@@@ RFHH check for error
    urj_log (ll, _("Configurable prototype bus driver via BSR (JTAG part No. %d)\n"),
            i);
}

/**
 * bus->driver->(*area)
 *
 */
static int
prototype_bus_area (urj_bus_t *bus, uint32_t adr, urj_bus_area_t *area)
{
    area->description = NULL;
    area->start = UINT32_C (0x00000000);
    area->length = UINT64_C (0x100000000);
    area->width = DW;

    return URJ_STATUS_OK;
}

static void
setup_address (urj_bus_t *bus, uint32_t a)
{
    int i, j;
    urj_part_t *p = bus->part;

    a >>= ASHIFT;

    for (i = 0, j = ALSBI; i < AW; i++, j += AI)
        urj_part_set_signal (p, A[j], 1, (a >> i) & 1);
}

static void
set_data_in (urj_bus_t *bus)
{
    int i, j;
    urj_part_t *p = bus->part;
    urj_bus_area_t area;

    prototype_bus_area (bus, 0, &area);

    for (i = 0, j = DLSBI; i < DW; i++, j += DI)
        urj_part_set_signal_input (p, D[j]);
}

static void
setup_data (urj_bus_t *bus, uint32_t d)
{
    int i, j;
    urj_part_t *p = bus->part;
    urj_bus_area_t area;

    prototype_bus_area (bus, 0, &area);

    for (i = 0, j = DLSBI; i < DW; i++, j += DI)
        urj_part_set_signal (p, D[j], 1, (d >> i) & 1);
}

/**
 * bus->driver->(*read_start)
 *
 */
static int
prototype_bus_read_start (urj_bus_t *bus, uint32_t adr)
{
    urj_part_t *p = bus->part;
    urj_chain_t *chain = bus->chain;

    urj_part_set_signal (p, CS, 1, CSA);
    urj_part_set_signal (p, WE, 1, WEA ? 0 : 1);
    urj_part_set_signal (p, OE, 1, OEA);

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
prototype_bus_read_next (urj_bus_t *bus, uint32_t adr)
{
    urj_part_t *p = bus->part;
    urj_chain_t *chain = bus->chain;
    int i, j;
    uint32_t d = 0;
    urj_bus_area_t area;

    prototype_bus_area (bus, adr, &area);

    setup_address (bus, adr);
    urj_tap_chain_shift_data_registers (chain, 1);

    for (i = 0, j = DLSBI; i < DW; i++, j += DI)
        d |= (uint32_t) (urj_part_get_signal (p, D[j]) << i);

    return d;
}

/**
 * bus->driver->(*read_end)
 *
 */
static uint32_t
prototype_bus_read_end (urj_bus_t *bus)
{
    urj_part_t *p = bus->part;
    urj_chain_t *chain = bus->chain;
    int i, j;
    uint32_t d = 0;
    urj_bus_area_t area;

    prototype_bus_area (bus, 0, &area);

    urj_part_set_signal (p, CS, 1, CSA ? 0 : 1);
    urj_part_set_signal (p, OE, 1, OEA ? 0 : 1);
    urj_tap_chain_shift_data_registers (chain, 1);

    for (i = 0, j = DLSBI; i < DW; i++, j += DI)
        d |= (uint32_t) (urj_part_get_signal (p, D[j]) << i);

    return d;
}

/**
 * bus->driver->(*write)
 *
 */
static void
prototype_bus_write (urj_bus_t *bus, uint32_t adr, uint32_t data)
{
    urj_part_t *p = bus->part;
    urj_chain_t *chain = bus->chain;

    urj_part_set_signal (p, CS, 1, CSA);
    urj_part_set_signal (p, WE, 1, WEA ? 0 : 1);
    urj_part_set_signal (p, OE, 1, OEA ? 0 : 1);

    setup_address (bus, adr);
    setup_data (bus, data);

    urj_tap_chain_shift_data_registers (chain, 0);

    urj_part_set_signal (p, WE, 1, WEA);
    urj_tap_chain_shift_data_registers (chain, 0);
    urj_part_set_signal (p, WE, 1, WEA ? 0 : 1);
    urj_part_set_signal (p, CS, 1, CSA ? 0 : 1);
    urj_tap_chain_shift_data_registers (chain, 0);
}

const urj_bus_driver_t urj_bus_prototype_bus = {
    "prototype",
    N_("Configurable prototype bus driver via BSR, requires parameters:\n"
       "           amsb=<addr MSB> alsb=<addr LSB> dmsb=<data MSB> dlsb=<data LSB>\n"
       "           ncs=<CS#>|cs=<CS> noe=<OE#>|oe=<OE> nwe=<WE#>|we=<WE> [amode=auto|x8|x16|x32]"),
    prototype_bus_new,
    urj_bus_generic_free,
    prototype_bus_printinfo,
    urj_bus_generic_prepare_extest,
    prototype_bus_area,
    prototype_bus_read_start,
    prototype_bus_read_next,
    prototype_bus_read_end,
    urj_bus_generic_read,
    urj_bus_generic_write_start,
    prototype_bus_write,
    urj_bus_generic_no_init,
    urj_bus_generic_no_enable,
    urj_bus_generic_no_disable,
    URJ_BUS_TYPE_PARALLEL,
};
