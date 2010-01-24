/*
 * $Id$
 *
 * Ka-Ro TRITON Starterkit II (PXA255/250) JTAG Cable
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
 * Modified for TRITON by Andreas Mohr <andi@lisas.de>, 2003
 *
 */

/*
 * Ka-Ro electronics GmbH (http://www.karo-electronics.de)
 * TRITON Starterkit II (PXA255/250) JTAG Parallel Cable Driver
 * (boards probably produced by www.mite.cz)
 * Other vendors: www.strategic-test.com, www.fsforth.de (www.es-usa.com),
 *                www.directinsight.co.uk, www.quantum.com.pl,
 *
 * This code has been verified to work with a Starterkit II,
 * but a Starterkit I might also work (however it has a differing JTAG cable
 * interface circuit, so all bets are off).
 */

#include <sysdep.h>

#include <urjtag/cable.h>
#include <urjtag/parport.h>
#include <urjtag/chain.h>

#include "generic.h"
#include "generic_parport.h"

/*
 * data D[7:0] (pins 9:2)
 */
#define TDI     1
#define TCK     0
#define TMS     2
#define TRST    3
#define SRESET  4
#define ENAB    5               /* not programmed, since it's always 0 */

/*
 * status
 *
 * 7 - BUSY (pin 11)
 * 6 - ACK (pin 10)
 * 5 - PE (pin 12)
 * 4 - SEL (pin 13)
 * 3 - ERROR (pin 15)
 */
#define TDO     7

static int
triton_init (urj_cable_t *cable)
{
    if (urj_tap_parport_open (cable->link.port) != URJ_STATUS_OK)
        return URJ_STATUS_FAIL;

    PARAM_SIGNALS (cable) = URJ_POD_CS_TRST | URJ_POD_CS_RESET;
    urj_tap_parport_set_data (cable->link.port, (1 << TRST) | (1 << SRESET));

    return URJ_STATUS_OK;
}

static void
triton_clock (urj_cable_t *cable, int tms, int tdi, int n)
{
    int i;
    int trst = (PARAM_SIGNALS (cable) & URJ_POD_CS_TRST) ? 1 : 0;
    int sreset = (PARAM_SIGNALS (cable) & URJ_POD_CS_RESET) ? 1 : 0;

    tms = tms ? 1 : 0;
    tdi = tdi ? 1 : 0;

    for (i = 0; i < n; i++)
    {
        urj_tap_parport_set_data (cable->link.port,
                                  (trst << TRST) | (sreset << SRESET)
                                  | (0 << TCK) | (tms << TMS) | (tdi << TDI));
        urj_tap_cable_wait (cable);
        urj_tap_parport_set_data (cable->link.port,
                                  (trst << TRST) | (sreset << SRESET)
                                  | (1 << TCK) | (tms << TMS) | (tdi << TDI));
        urj_tap_cable_wait (cable);
    }

    PARAM_SIGNALS (cable) &= (URJ_POD_CS_TRST | URJ_POD_CS_RESET);
    PARAM_SIGNALS (cable) |= URJ_POD_CS_TCK;
    PARAM_SIGNALS (cable) |= tms ? URJ_POD_CS_TMS : 0;
    PARAM_SIGNALS (cable) |= tdi ? URJ_POD_CS_TDI : 0;
}

static int
triton_get_tdo (urj_cable_t *cable)
{
    int trst = (PARAM_SIGNALS (cable) & URJ_POD_CS_TRST) ? 1 : 0;
    int sreset = (PARAM_SIGNALS (cable) & URJ_POD_CS_RESET) ? 1 : 0;
    int status;

    urj_tap_parport_set_data (cable->link.port,
                              (trst << TRST) | (sreset << SRESET) | (0 << TCK));
    PARAM_SIGNALS (cable) &=
        ~(URJ_POD_CS_TDI | URJ_POD_CS_TCK | URJ_POD_CS_TMS);

    urj_tap_cable_wait (cable);

    status = urj_tap_parport_get_status (cable->link.port);
    if (status == -1)
        return status;

    return (status >> TDO) & 1;
}

static int
triton_set_signal (urj_cable_t *cable, int mask, int val)
{
    int prev_sigs = PARAM_SIGNALS (cable);

    mask &= (URJ_POD_CS_TDI | URJ_POD_CS_TCK | URJ_POD_CS_TMS | URJ_POD_CS_TRST | URJ_POD_CS_RESET);    // only these can be modified

    if (mask != 0)
    {
        int data = 0;
        int sigs = (PARAM_SIGNALS (cable) & ~mask) | (val & mask);
        data |= (sigs & URJ_POD_CS_TDI) ? (1 << TDI) : 0;
        data |= (sigs & URJ_POD_CS_TCK) ? (1 << TCK) : 0;
        data |= (sigs & URJ_POD_CS_TMS) ? (1 << TMS) : 0;
        data |= (sigs & URJ_POD_CS_TRST) ? (1 << TRST) : 0;
        data |= (sigs & URJ_POD_CS_RESET) ? (1 << SRESET) : 0;
        urj_tap_parport_set_data (cable->link.port, data);
        PARAM_SIGNALS (cable) = sigs;
    }

    return prev_sigs;
}

const urj_cable_driver_t urj_tap_cable_triton_driver = {
    "TRITON",
    N_("Ka-Ro TRITON Starterkit II (PXA255/250) JTAG Cable"),
    URJ_CABLE_DEVICE_PARPORT,
    { .parport = urj_tap_cable_generic_parport_connect, },
    urj_tap_cable_generic_disconnect,
    urj_tap_cable_generic_parport_free,
    triton_init,
    urj_tap_cable_generic_parport_done,
    urj_tap_cable_generic_set_frequency,
    triton_clock,
    triton_get_tdo,
    urj_tap_cable_generic_transfer,
    triton_set_signal,
    urj_tap_cable_generic_get_signal,
    urj_tap_cable_generic_flush_one_by_one,
    urj_tap_cable_generic_parport_help
};
