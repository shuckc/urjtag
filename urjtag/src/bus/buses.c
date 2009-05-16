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
 * Written by Marcel Telka <marcel@telka.sk>, 2003.
 *
 */

#include <sysdep.h>

#include <stdio.h>
#include <stdlib.h>

#include <urjtag/error.h>
#include <urjtag/bus.h>

#include "buses.h"

const urj_bus_driver_t *urj_bus_drivers[] = {
#ifdef ENABLE_BUS_AU1500
    &urj_bus_au1500_bus,
#endif
#ifdef ENABLE_BUS_AVR32
    &urj_bus_avr32_bus_driver,
#endif
#ifdef ENABLE_BUS_BCM1250
    &urj_bus_bcm1250_bus,
#endif
#ifdef ENABLE_BUS_BF526_EZKIT
    &urj_bus_bf526_ezkit_bus,
#endif
#ifdef ENABLE_BUS_BF527_EZKIT
    &urj_bus_bf527_ezkit_bus,
#endif
#ifdef ENABLE_BUS_BF533_STAMP
    &urj_bus_bf533_stamp_bus,
#endif
#ifdef ENABLE_BUS_BF533_EZKIT
    &urj_bus_bf533_ezkit_bus,
#endif
#ifdef ENABLE_BUS_BF537_STAMP
    &urj_bus_bf537_stamp_bus,
#endif
#ifdef ENABLE_BUS_BF537_EZKIT
    &urj_bus_bf537_ezkit_bus,
#endif
#ifdef ENABLE_BUS_BF538F_EZKIT
    &urj_bus_bf538f_ezkit_bus,
#endif
#ifdef ENABLE_BUS_BF548_EZKIT
    &urj_bus_bf548_ezkit_bus,
#endif
#ifdef ENABLE_BUS_BF561_EZKIT
    &urj_bus_bf561_ezkit_bus,
#endif
#ifdef ENABLE_BUS_BSCOACH
    &urj_bus_bscoach_bus,
#endif
#ifdef ENABLE_BUS_EJTAG
    &urj_bus_ejtag_bus,
    &urj_bus_ejtag_dma_bus,
#endif
#ifdef ENABLE_BUS_FJMEM
    &urj_bus_fjmem_bus,
#endif
#ifdef ENABLE_BUS_IXP425
    &urj_bus_ixp425_bus,
#endif
#ifdef ENABLE_BUS_IXP435
    &urj_bus_ixp435_bus,
#endif
#ifdef ENABLE_BUS_JOPCYC
    &urj_bus_jopcyc_bus,
#endif
#ifdef ENABLE_BUS_H7202
    &urj_bus_h7202_bus,
#endif
#ifdef ENABLE_BUS_LH7A400
    &urj_bus_lh7a400_bus,
#endif
#ifdef ENABLE_BUS_MPC5200
    &urj_bus_mpc5200_bus,
#endif
#ifdef ENABLE_BUS_MPC824X
    &urj_bus_mpc824x_bus,
#endif
#ifdef ENABLE_BUS_PPC405EP
    &urj_bus_ppc405ep_bus,
#endif
#ifdef ENABLE_BUS_PPC440GX_EBC8
    &urj_bus_ppc440gx_ebc8_bus,
#endif
#ifdef ENABLE_BUS_PROTOTYPE
    &urj_bus_prototype_bus,
#endif
#ifdef ENABLE_BUS_PXA2X0
    &urj_bus_pxa2x0_bus,
#endif
#ifdef ENABLE_BUS_PXA27X
    &urj_bus_pxa27x_bus,
#endif
#ifdef ENABLE_BUS_S3C4510
    &urj_bus_s3c4510_bus,
#endif
#ifdef ENABLE_BUS_SA1110
    &urj_bus_sa1110_bus,
#endif
#ifdef ENABLE_BUS_SH7727
    &urj_bus_sh7727_bus,
#endif
#ifdef ENABLE_BUS_SH7750R
    &urj_bus_sh7750r_bus,
#endif
#ifdef ENABLE_BUS_SH7751R
    &urj_bus_sh7751r_bus,
#endif
#ifdef ENABLE_BUS_SHARC_21065L
    &urj_bus_sharc_21065L_bus,
#endif
#ifdef ENABLE_BUS_SLSUP3
    &urj_bus_slsup3_bus,
#endif
#ifdef ENABLE_BUS_TX4925
    &urj_bus_tx4925_bus,
#endif
#ifdef ENABLE_BUS_ZEFANT_XS3
    &urj_bus_zefant_xs3_bus,
#endif
    NULL                        /* last must be NULL */
};

urj_bus_t *urj_bus = NULL;
urj_buses_t urj_buses = { 0, NULL };

void
urj_bus_buses_free (void)
{
    int i;

    for (i = 0; i < urj_buses.len; i++)
        URJ_BUS_FREE (urj_buses.buses[i]);

    free (urj_buses.buses);
    urj_buses.len = 0;
    urj_buses.buses = NULL;
    urj_bus = NULL;
}

void
urj_bus_buses_add (urj_bus_t *abus)
{
    urj_bus_t **b;

    if (abus == NULL)
        /* @@@@ RFHH add status return */
        return;

    b = realloc (urj_buses.buses, (urj_buses.len + 1) * sizeof (urj_bus_t *));
    if (b == NULL)
    {
        /* @@@@ RFHH add status return */
        urj_error_set (URJ_ERROR_OUT_OF_MEMORY, "calloc(%zd,%zd) fails",
                       1, sizeof (urj_bus_t));
        printf (_("Out of memory\n"));
        return;
    }
    urj_buses.buses = b;
    urj_buses.buses[urj_buses.len++] = abus;
    if (urj_bus == NULL)
        urj_bus = abus;
}

void
urj_bus_buses_delete (urj_bus_t *abus)
{
    int i;
    urj_bus_t **b;

    for (i = 0; i < urj_buses.len; i++)
        if (abus == urj_buses.buses[i])
            break;
    if (i >= urj_buses.len)
        /* @@@@ RFHH add status return */
        return;

    while (i + 1 < urj_buses.len)
    {
        urj_buses.buses[i] = urj_buses.buses[i + 1];
        i++;
    }
    urj_buses.len--;
    b = realloc (urj_buses.buses, urj_buses.len * sizeof (urj_bus_t *));
    if ((b != NULL) || (urj_buses.len == 0))
        urj_buses.buses = b;

    if (urj_bus != abus)
        /* @@@@ RFHH add status return */
        return;

    if (urj_buses.len > 0)
        urj_bus = urj_buses.buses[0];
}

int
urj_bus_buses_set (int n)
{
    if (n >= urj_buses.len)
    {
        urj_error_set(URJ_ERROR_INVALID, _("invalid bus number"));
        return URJ_STATUS_FAIL;
    }

    urj_bus = urj_buses.buses[n];

    return URJ_STATUS_OK;
}
