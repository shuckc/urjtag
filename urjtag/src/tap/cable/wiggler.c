/*
 * $Id$
 *
 * Macraigor Wiggler JTAG Cable Driver
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
 * Documentation:
 * [1] http://www.ocdemon.net/
 * [2] http://jtag-arm9.sourceforge.net/hardware.html
 *
 */

#include <sysdep.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <urjtag/cable.h>
#include <urjtag/parport.h>
#include <urjtag/chain.h>

#include "generic.h"
#include "generic_parport.h"

#include <urjtag/cmd.h>

/*
 * Bit <-> pin mapping of an original Wiggler
 * This is the default when no mapping is specified for wiggler_connect( )
 *
 * data D[7:0] (pins 9:2)
 */
#define nTRST   4               /* nTRST is not inverted in the cable */
#define TDI     3
#define TCK     2
#define TMS     1
#define nSRESET 0               /* sRESET is inverted in the cable */

/*
 * 7 - BUSY (pin 11)
 * 6 - ACK (pin 10)
 * 5 - PE (pin 12)
 * 4 - SEL (pin 13)
 * 3 - ERROR (pin 15)
 */
#define TDO     7


/* macros used to stringify the defines above */
#define xstr(s) str(s)
#define str(s) #s
static const char *std_wgl_map = xstr (TDO) ","
                                 xstr (nTRST) ","
                                 xstr (TDI) ","
                                 xstr (TCK) ","
                                 xstr (TMS) ","
                                 "#" xstr (nSRESET);


/* private parameters of this cable driver */
typedef struct
{
    int signals;
    int trst_lvl;
    int srst_act, srst_inact;
    int tms_act, tms_inact;
    int tck_act, tck_inact;
    int tdi_act, tdi_inact;
    int tdo_act, tdo_inact;
    int trst_act, trst_inact;
    int unused_bits;
} wiggler_params_t;


/* access macros for the parameters */
#define PRM_SIGNALS(cable)     ((wiggler_params_t *) (cable)->params)->signals
#define PRM_TRST_LVL(cable)    ((wiggler_params_t *) (cable)->params)->trst_lvl
#define PRM_SRST_ACT(cable)    ((wiggler_params_t *) (cable)->params)->srst_act
#define PRM_SRST_INACT(cable)  ((wiggler_params_t *) (cable)->params)->srst_inact
#define PRM_TMS_ACT(cable)     ((wiggler_params_t *) (cable)->params)->tms_act
#define PRM_TMS_INACT(cable)   ((wiggler_params_t *) (cable)->params)->tms_inact
#define PRM_TCK_ACT(cable)     ((wiggler_params_t *) (cable)->params)->tck_act
#define PRM_TCK_INACT(cable)   ((wiggler_params_t *) (cable)->params)->tck_inact
#define PRM_TDI_ACT(cable)     ((wiggler_params_t *) (cable)->params)->tdi_act
#define PRM_TDI_INACT(cable)   ((wiggler_params_t *) (cable)->params)->tdi_inact
#define PRM_TDO_ACT(cable)     ((wiggler_params_t *) (cable)->params)->tdo_act
#define PRM_TDO_INACT(cable)   ((wiggler_params_t *) (cable)->params)->tdo_inact
#define PRM_TRST_ACT(cable)    ((wiggler_params_t *) (cable)->params)->trst_act
#define PRM_TRST_INACT(cable)  ((wiggler_params_t *) (cable)->params)->trst_inact
#define PRM_UNUSED_BITS(cable) ((wiggler_params_t *) (cable)->params)->unused_bits



static int map_pin (const char *pin, int *act, int *inact)
{
    int bitnum;
    int inverted = 0;

    if (*pin == '#')
    {
        inverted = 1;
        pin++;
    }

    if (!isdigit (*pin))
    {
        urj_error_set (URJ_ERROR_SYNTAX, "should be digit: '%s'", pin);
        return -1;
    }

    bitnum = atoi (pin) % 8;

    bitnum = 1 << bitnum;

    *act = inverted ? 0 : bitnum;
    *inact = inverted ? bitnum : 0;

    return 0;
}


static int
set_mapping (const char *bitmap, urj_cable_t *cable)
{
    const char delim = ',';
    int syntax = 0;
    const char *tdo, *trst, *tdi, *tck, *tms, *srst;

    /* assign mappings for each pin */
    if ((tdo = bitmap))
        if ((trst = strchr (tdo, delim)))
        {
            trst++;
            if ((tdi = strchr (trst, delim)))
            {
                tdi++;
                if ((tck = strchr (tdi, delim)))
                {
                    tck++;
                    if ((tms = strchr (tck, delim)))
                    {
                        tms++;
                        if ((srst = strchr (tms, delim)))
                        {
                            srst++;
                            syntax = 1;
                        }
                    }
                }
            }
        }

    if (!syntax)
    {
        urj_error_set (URJ_ERROR_SYNTAX, "pin mapping");
        return -1;
    }

    if (map_pin (tdo, &(PRM_TDO_ACT (cable)), &(PRM_TDO_INACT (cable))) != 0)
        return -1;
    if (map_pin (trst, &(PRM_TRST_ACT (cable)), &(PRM_TRST_INACT (cable))) != 0)
        return -1;
    if (map_pin (tdi, &(PRM_TDI_ACT (cable)), &(PRM_TDI_INACT (cable))) != 0)
        return -1;
    if (map_pin (tck, &(PRM_TCK_ACT (cable)), &(PRM_TCK_INACT (cable))) != 0)
        return -1;
    if (map_pin (tms, &(PRM_TMS_ACT (cable)), &(PRM_TMS_INACT (cable))) != 0)
        return -1;
    if (map_pin (srst, &(PRM_SRST_ACT (cable)), &(PRM_SRST_INACT (cable))) != 0)
        return -1;

    return 0;
}


static int
wiggler_connect (urj_cable_t *cable, urj_cable_parport_devtype_t devtype,
                 const char *devname, const urj_param_t *params[])
{
    const urj_param_t *param_bitmap = NULL;
    const char *bitmap = NULL;
    wiggler_params_t *wiggler_params;

    if (urj_param_num (params) > 0)
    {
        /* acquire optional parameter for bit<->pin mapping */
        param_bitmap = params[0];
        if (params[0]->type != URJ_PARAM_TYPE_STRING)
        {
            urj_error_set (URJ_ERROR_SYNTAX, "mapping name should be a string");
            return URJ_STATUS_FAIL;
        }
        /* urj_tap_cable_generic_parport_connect() shouldn't see this parameter */
        params[0] = NULL;
    }

    if (urj_tap_cable_generic_parport_connect (cable, devtype, devname,
                                               params) != URJ_STATUS_OK)
        return URJ_STATUS_FAIL;

    if (param_bitmap)
        params[0] = param_bitmap;

    wiggler_params = malloc (sizeof *wiggler_params);
    if (!wiggler_params)
    {
        urj_error_set (URJ_ERROR_OUT_OF_MEMORY, _("malloc(%zd) fails"),
                       sizeof *wiggler_params);
        /* NOTE:
         * Call the underlying parport driver (*free) routine directly
         * not generic_parconn_free() since it also free's cable->params
         * (which is not established) and cable (which the caller will do)
         */
        cable->link.port->driver->parport_free (cable->link.port);
        return 4;
    }

    /* set new wiggler-specific params */
    free (cable->params);
    cable->params = wiggler_params;

    if (!param_bitmap)
        bitmap = (char *) std_wgl_map;
    else
        bitmap = param_bitmap->value.string;

    if (set_mapping (bitmap, cable) != 0)
    {
        urj_log (URJ_LOG_LEVEL_ERROR, _("Pin mapping failed\n"));
        /* NOTE:
         * Call the underlying parport driver (*free) routine directly
         * not generic_parconn_free() since it also free's cable (which
         * the caller will do)
         */
        cable->link.port->driver->parport_free (cable->link.port);
        free (cable->params);
        return URJ_STATUS_FAIL;
    }

    /* Certain Macraigor Wigglers appear to use one of the unused data lines as a
       power line so set all unused bits high. */
    PRM_UNUSED_BITS (cable) =
        ~(PRM_SRST_ACT (cable) | PRM_SRST_INACT (cable) | PRM_TMS_ACT (cable)
          | PRM_TMS_INACT (cable) | PRM_TCK_ACT (cable) | PRM_TCK_INACT (cable)
          | PRM_TDI_ACT (cable) | PRM_TDI_INACT (cable) | PRM_TRST_ACT (cable)
          | PRM_TRST_INACT (cable)) & 0xff;

    return 0;
}

static int
wiggler_init (urj_cable_t *cable)
{
    int data;

    if (urj_tap_parport_open (cable->link.port) != URJ_STATUS_OK)
        return -1;

    if ((data = urj_tap_parport_get_data (cable->link.port)) < 0)
    {
        if (urj_tap_parport_set_data (cable->link.port,
                                      PRM_TRST_ACT (cable)
                                      | PRM_TRST_INACT (cable)
                                      | PRM_UNUSED_BITS (cable))
            != URJ_STATUS_OK)
            return URJ_STATUS_FAIL;
        PRM_TRST_LVL (cable) = PRM_TRST_ACT (cable) | PRM_TRST_INACT (cable);
    }
    else
        PRM_TRST_LVL (cable) =
            data & (PRM_TRST_ACT (cable) | PRM_TRST_INACT (cable));

    PRM_SIGNALS (cable) =
        (PRM_TRST_LVL (cable) == PRM_TRST_ACT (cable)) ? URJ_POD_CS_TRST : 0;

    return URJ_STATUS_OK;
}

static void
wiggler_clock (urj_cable_t *cable, int tms, int tdi, int n)
{
    int i;

    tms = tms ? 1 : 0;
    tdi = tdi ? 1 : 0;

    for (i = 0; i < n; i++)
    {
        urj_tap_parport_set_data (cable->link.port,
                                  PRM_TRST_LVL (cable)
                                  | PRM_TCK_INACT (cable)
                                  | (tms ? PRM_TMS_ACT (cable)
                                         : PRM_TMS_INACT (cable))
                                  | (tdi ? PRM_TDI_ACT (cable)
                                         : PRM_TDI_INACT (cable))
                                  | PRM_UNUSED_BITS (cable));
        urj_tap_cable_wait (cable);
        urj_tap_parport_set_data (cable->link.port,
                                  PRM_TRST_LVL (cable)
                                  | PRM_TCK_ACT (cable)
                                  | (tms ? PRM_TMS_ACT (cable)
                                         : PRM_TMS_INACT (cable))
                                  | (tdi ? PRM_TDI_ACT (cable)
                                         : PRM_TDI_INACT (cable))
                                  | PRM_UNUSED_BITS (cable));
        urj_tap_cable_wait (cable);
    }

    PRM_SIGNALS (cable) &= ~(URJ_POD_CS_TDI | URJ_POD_CS_TMS);
    if (tms)
        PRM_SIGNALS (cable) |= URJ_POD_CS_TMS;
    if (tdi)
        PRM_SIGNALS (cable) |= URJ_POD_CS_TDI;
}

static int
wiggler_get_tdo (urj_cable_t *cable)
{
    int status;

    urj_tap_parport_set_data (cable->link.port, PRM_TRST_LVL (cable) |
                              PRM_TCK_INACT (cable) |
                              PRM_UNUSED_BITS (cable));
    urj_tap_cable_wait (cable);

    status = urj_tap_parport_get_status (cable->link.port);
    if (status == -1)
        return -1;

    return (status & (PRM_TDO_ACT (cable) | PRM_TDO_INACT (cable)))
             ^ PRM_TDO_ACT (cable) ? 0 : 1;
}

static int
wiggler_set_signal (urj_cable_t *cable, int mask, int val)
{
    int prev_sigs = PRM_SIGNALS (cable);

    mask &= (URJ_POD_CS_TMS | URJ_POD_CS_TDI | URJ_POD_CS_TCK | URJ_POD_CS_TRST);       // Only these can be modified

    if (mask != 0)
    {
        int sigs = (prev_sigs & ~mask) | (val & mask);
        PRM_TRST_LVL (cable) =
            ((sigs & URJ_POD_CS_TRST) ? PRM_TRST_ACT (cable) :
             PRM_TRST_INACT (cable));
        int data = PRM_UNUSED_BITS (cable) | PRM_TRST_LVL (cable);
        data |=
            ((sigs & URJ_POD_CS_TCK) ? PRM_TCK_ACT (cable) :
             PRM_TCK_INACT (cable));
        data |=
            ((sigs & URJ_POD_CS_TMS) ? PRM_TMS_ACT (cable) :
             PRM_TMS_INACT (cable));
        data |=
            ((sigs & URJ_POD_CS_TDI) ? PRM_TDI_ACT (cable) :
             PRM_TDI_INACT (cable));
        urj_tap_parport_set_data (cable->link.port, data);
        PRM_SIGNALS (cable) = sigs;
    }

    return prev_sigs;
}

static int
wiggler_get_signal (urj_cable_t *cable, urj_pod_sigsel_t sig)
{
    return (PRM_SIGNALS (cable) & sig) ? 1 : 0;
}

static void
wiggler_help (urj_log_level_t ll, const char *cablename)
{
    urj_log (ll,
             _("Usage: cable %s parallel PORTADDR [TDO,TRST,TDI,TCK,TMS,SRESET]\n"
#if ENABLE_LOWLEVEL_PPDEV
               "   or: cable %s ppdev PPDEV [TDO,TRST,TDI,TCK,TMS,SRESET]\n"
#endif
#if HAVE_DEV_PPBUS_PPI_H
               "   or: cable %s ppi PPIDEV [TDO,TRST,TDI,TCK,TMS,SRESET]\n"
#endif
               "\n" "PORTADDR   parallel port address (e.g. 0x378)\n"
#if ENABLE_LOWLEVEL_PPDEV
               "PPDEV      ppdev device (e.g. /dev/parport0)\n"
#endif
#if HAVE_DEV_PPBUS_PPI_H
               "PPIDEF     ppi device (e.g. /dev/ppi0)\n"
#endif
               "TDO, ...   parallel port bit number, prepend '#' for inversion\n"
               "           default is '%s'\n" "\n"),
#if ENABLE_LOWLEVEL_PPDEV
             cablename,
#endif
#if HAVE_DEV_PPBUS_PPI_H
             cablename,
#endif
             cablename, std_wgl_map);
}

const urj_cable_driver_t urj_tap_cable_wiggler_driver = {
    "WIGGLER",
    N_("Macraigor Wiggler JTAG Cable"),
    URJ_CABLE_DEVICE_PARPORT,
    { .parport = wiggler_connect, },
    urj_tap_cable_generic_disconnect,
    urj_tap_cable_generic_parport_free,
    wiggler_init,
    urj_tap_cable_generic_parport_done,
    urj_tap_cable_generic_set_frequency,
    wiggler_clock,
    wiggler_get_tdo,
    urj_tap_cable_generic_transfer,
    wiggler_set_signal,
    wiggler_get_signal,
    urj_tap_cable_generic_flush_one_by_one,
    wiggler_help
};

const urj_cable_driver_t urj_tap_cable_igloo_driver = {
    "IGLOO",
    N_("Excelpoint IGLOO JTAG Cable"),
    URJ_CABLE_DEVICE_PARPORT,
    { .parport = wiggler_connect, },
    urj_tap_cable_generic_disconnect,
    urj_tap_cable_generic_parport_free,
    wiggler_init,
    urj_tap_cable_generic_parport_done,
    urj_tap_cable_generic_set_frequency,
    wiggler_clock,
    wiggler_get_tdo,
    urj_tap_cable_generic_transfer,
    wiggler_set_signal,
    wiggler_get_signal,
    urj_tap_cable_generic_flush_one_by_one,
    wiggler_help
};
