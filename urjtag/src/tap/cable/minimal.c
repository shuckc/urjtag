/*
 * $Id$
 *
 * Minimal Parallel Port JTAG Cable Driver
 * Copyright © 2009 Yen Rui
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
 * The Minimal Parallel Port JTAG Cable is a cheap and easy-to-make cable
 * that only uses the four fundamental JTAG signals (TMS, TCD, TDI and TDO),
 * i.e. it does NOT use the TRST signal. The Minimal Parallel Port JTAG
 * Cable is a strict subset of the Macraigor Wiggler JTAG Cable, i.e. the
 * more expensive Macraigor Wiggler JTAG Cable can be a drop-in replacement
 * of the Minimal Parallel Port JTAG Cable.
 *
 * http://oldwiki.openwrt.org/OpenWrtDocs(2f)Customizing(2f)Hardware(2f)JTAG_Cable.html
 *
 * This driver does not modify the state of any other parallel port pins.
 *
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
#define TMS 1
#define TCK 2
#define TDI 3

/*
 * 7 - BUSY (pin 11)
 * 6 - ACK (pin 10)
 * 5 - PE (pin 12)
 * 4 - SEL (pin 13)
 * 3 - ERROR (pin 15)
 */
#define TDO 7

/* FIXME: having a static variable like this is probably not thread safe */
static unsigned char unused_bits;

static int
minimal_init (urj_cable_t *cable)
{
    int data;

    if (urj_tap_parport_open (cable->link.port))
        return URJ_STATUS_FAIL;

    if ((data = urj_tap_parport_get_data (cable->link.port)) < 0)
        return URJ_STATUS_FAIL;

    /* remember state of all parallel port pins except TDI, TCK and TMS */
    unused_bits = data & (~((1 << TDI) | (1 << TCK) | (1 << TMS)));

    /* copy data into PARAM_SIGNALS (cable) while faking URJ_POD_CS_TRST */
    PARAM_SIGNALS (cable) = URJ_POD_CS_TRST;
    PARAM_SIGNALS (cable) |= (data & TDI) ? URJ_POD_CS_TDI : 0;
    PARAM_SIGNALS (cable) |= (data & TCK) ? URJ_POD_CS_TCK : 0;
    PARAM_SIGNALS (cable) |= (data & TMS) ? URJ_POD_CS_TMS : 0;

    return URJ_STATUS_OK;
}

static void
minimal_clock (urj_cable_t *cable, int tms, int tdi, int n)
{
    int i;

    tms = tms ? 1 : 0;
    tdi = tdi ? 1 : 0;

    for (i = 0; i < n; i++)
    {
        urj_tap_parport_set_data (cable->link.port,
                                  (tms << TMS) | (0 << TCK) | (tdi << TDI) |
                                  unused_bits);
        urj_tap_cable_wait (cable);
        urj_tap_parport_set_data (cable->link.port,
                                  (tms << TMS) | (1 << TCK) | (tdi << TDI) |
                                  unused_bits);
        urj_tap_cable_wait (cable);
    }

    /* do not touch URJ_POD_CS_TRST in PARAM_SIGNALS (cable) */
    PARAM_SIGNALS (cable) &= ~(URJ_POD_CS_TMS | URJ_POD_CS_TDI);
    PARAM_SIGNALS (cable) |= URJ_POD_CS_TCK;
    PARAM_SIGNALS (cable) |= tms ? URJ_POD_CS_TMS : 0;
    PARAM_SIGNALS (cable) |= tdi ? URJ_POD_CS_TDI : 0;
}

static int
minimal_get_tdo (urj_cable_t *cable)
{
    urj_tap_parport_set_data (cable->link.port, (0 << TCK) | unused_bits);

    PARAM_SIGNALS (cable) &=
        ~(URJ_POD_CS_TDI | URJ_POD_CS_TCK | URJ_POD_CS_TMS);

    urj_tap_cable_wait (cable);

    return (urj_tap_parport_get_status (cable->link.port) >> TDO) & 1;
}

static int
minimal_set_signal (urj_cable_t *cable, int mask, int val)
{
    int prev_sigs = PARAM_SIGNALS (cable);

    /* only these signals can be modified */
    mask &= (URJ_POD_CS_TDI | URJ_POD_CS_TCK | URJ_POD_CS_TMS | URJ_POD_CS_TRST);

    if (mask != 0)
    {
        unsigned char data = 0;
        int sigs = (prev_sigs & ~mask) | (val & mask);
        data |= (sigs & URJ_POD_CS_TDI) ? (1 << TDI) : 0;
        data |= (sigs & URJ_POD_CS_TCK) ? (1 << TCK) : 0;
        data |= (sigs & URJ_POD_CS_TMS) ? (1 << TMS) : 0;
        /* do not actually set TRST on parallel port ... */
        //data |= (sigs & URJ_POD_CS_TRST) ? (1 << TRST) : 0;
        urj_tap_parport_set_data (cable->link.port, data | unused_bits);
        /* ... although TRST can be marked as set in PARAM_SIGNALS (cable) */
        PARAM_SIGNALS (cable) = sigs;
    }

    return prev_sigs;
}

const urj_cable_driver_t urj_tap_cable_minimal_driver = {
    "Minimal",
    N_("Minimal Parallel Port JTAG Cable"),
    URJ_CABLE_DEVICE_PARPORT,
    { .parport = urj_tap_cable_generic_parport_connect, },
    urj_tap_cable_generic_disconnect,
    urj_tap_cable_generic_parport_free,
    minimal_init,
    urj_tap_cable_generic_parport_done,
    urj_tap_cable_generic_set_frequency,
    minimal_clock,
    minimal_get_tdo,
    urj_tap_cable_generic_transfer,
    minimal_set_signal,
    urj_tap_cable_generic_get_signal,
    urj_tap_cable_generic_flush_one_by_one,
    urj_tap_cable_generic_parport_help
};
