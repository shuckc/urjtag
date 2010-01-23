/*
 * $Id$
 *
 * Altera ByteBlaster/ByteBlaster II/ByteBlasterMV Parallel Port Download Cable Driver
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
 * [1] Altera Corporation, "ByteBlaster Parallel Port Download Cable Data Sheet",
 *     February 1998, ver. 2.01, Order Number: A-DS-BYTE-02.01
 * [2] Altera Corporation, "ByteBlasterMV Parallel Port Download Cable Data Sheet",
 *     July 2002, Version 3.3, Order Number: DS-BYTBLMV-3.3
 * [3] Altera Corporation, "ByteBlaster II Parallel Port Download Cable Data Sheet",
 *     December 2002, Version 1.0, Order Number: DS-BYTEBLSTRII-1.0 L01-08739-00
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
#define TDI             6
#define TCK             0
#define TMS             1
#define BB_CHECK        5

/*
 * 7 - BUSY (pin 11)
 * 6 - ACK (pin 10)
 * 5 - PE (pin 12)
 * 4 - SEL (pin 13)
 * 3 - ERROR (pin 15)
 */
#define TDO             7
#define BB_PRESENT      6
#define VCC_OK_N        3

/*
 * 0 - STROBE (pin 1)
 * 1 - AUTOFD (pin 14)
 * 2 - INIT (pin 16)
 * 3 - SELECT (pin 17)
*/
#define BB_ENABLE       0xC

static int
byteblaster_init (urj_cable_t *cable)
{
    int BB_II = 0;
    int status;

    if (urj_tap_parport_open (cable->link.port) != URJ_STATUS_OK)
        return URJ_STATUS_FAIL;

    PARAM_SIGNALS (cable) = URJ_POD_CS_TRST;

    /* check if a ByteBlaster or ByteBlasterMV is connected */
    urj_tap_parport_set_data (cable->link.port, 1 << BB_CHECK);
    status = urj_tap_parport_get_status (cable->link.port);
    if (status == -1)
        return URJ_STATUS_FAIL;
    if (!((status >> BB_PRESENT) & 1))
        BB_II = 1;
    urj_tap_parport_set_data (cable->link.port, 0);
    status = urj_tap_parport_get_status (cable->link.port);
    if (status == -1)
        return URJ_STATUS_FAIL;
    if ((status >> BB_PRESENT) & 1)
        BB_II = 1;

    /* check if the power supply is ok (only for ByteBlaster II) */
    /* if no ByteBlaster at all is connected this check will fail, too */
    if (BB_II)
    {
        status = urj_tap_parport_get_status (cable->link.port);
        if (status == -1)
            return URJ_STATUS_FAIL;
        if ((status >> VCC_OK_N) & 1)
        {
            urj_error_set (URJ_ERROR_ILLEGAL_STATE, "Power supply not OK");
            return URJ_STATUS_FAIL;
        }
    }

    /* Enable ByteBlaster */
    if (urj_tap_parport_set_control (cable->link.port,
                                     BB_ENABLE) != URJ_STATUS_OK)
        return URJ_STATUS_FAIL;

    return URJ_STATUS_OK;
}

static void
byteblaster_clock (urj_cable_t *cable, int tms, int tdi, int n)
{
    int i;

    tms = tms ? 1 : 0;
    tdi = tdi ? 1 : 0;

    for (i = 0; i < n; i++)
    {
        urj_tap_parport_set_data (cable->link.port,
                                  (0 << TCK) | (tms << TMS) | (tdi << TDI));
        urj_tap_cable_wait (cable);
        urj_tap_parport_set_data (cable->link.port,
                                  (1 << TCK) | (tms << TMS) | (tdi << TDI));
        urj_tap_cable_wait (cable);
    }

    PARAM_SIGNALS (cable) &= URJ_POD_CS_TRST;
    PARAM_SIGNALS (cable) |= URJ_POD_CS_TCK;
    PARAM_SIGNALS (cable) |= tms ? URJ_POD_CS_TMS : 0;
    PARAM_SIGNALS (cable) |= tdi ? URJ_POD_CS_TDI : 0;
}

static int
byteblaster_get_tdo (urj_cable_t *cable)
{
    urj_tap_parport_set_data (cable->link.port, 0 << TCK);
    int status;

    PARAM_SIGNALS (cable) &=
        ~(URJ_POD_CS_TDI | URJ_POD_CS_TCK | URJ_POD_CS_TMS);

    urj_tap_cable_wait (cable);

    status = urj_tap_parport_get_status (cable->link.port);
    if (status == -1)
        return status;

    return (status >> TDO) & 1;
}

static int
byteblaster_set_signal (urj_cable_t *cable, int mask, int val)
{
    int prev_sigs = PARAM_SIGNALS (cable);

    mask &= (URJ_POD_CS_TDI | URJ_POD_CS_TCK | URJ_POD_CS_TMS); // only these can be modified

    if (mask != 0)
    {
        int data = 0;
        int sigs = (prev_sigs & ~mask) | (val & mask);
        data |= (sigs & URJ_POD_CS_TDI) ? (1 << TDI) : 0;
        data |= (sigs & URJ_POD_CS_TCK) ? (1 << TCK) : 0;
        data |= (sigs & URJ_POD_CS_TMS) ? (1 << TMS) : 0;
        urj_tap_parport_set_data (cable->link.port, data);
        PARAM_SIGNALS (cable) = sigs;
    }

    return prev_sigs;
}

const const urj_cable_driver_t urj_tap_cable_byteblaster_driver = {
    "ByteBlaster",
    N_("Altera ByteBlaster/ByteBlaster II/ByteBlasterMV Parallel Port Download Cable"),
    URJ_CABLE_DEVICE_PARPORT,
    { .parport = urj_tap_cable_generic_parport_connect, },
    urj_tap_cable_generic_disconnect,
    urj_tap_cable_generic_parport_free,
    byteblaster_init,
    urj_tap_cable_generic_parport_done,
    urj_tap_cable_generic_set_frequency,
    byteblaster_clock,
    byteblaster_get_tdo,
    urj_tap_cable_generic_transfer,
    byteblaster_set_signal,
    urj_tap_cable_generic_get_signal,
    urj_tap_cable_generic_flush_one_by_one,
    urj_tap_cable_generic_parport_help
};
