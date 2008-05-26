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

#include "sysdep.h"

#include <stdlib.h>

#include "bus.h"
#include "buses.h"

const bus_driver_t *bus_drivers[] = {
#ifdef ENABLE_BUS_AU1500
	&au1500_bus,
#endif
#ifdef ENABLE_BUS_AVR32
	&avr32_bus_driver,
#endif
#ifdef ENABLE_BUS_BCM1250
	&bcm1250_bus,
#endif
#ifdef ENABLE_BUS_BF527_EZKIT
	&bf527_ezkit_bus,
#endif
#ifdef ENABLE_BUS_BF533_STAMP
	&bf533_stamp_bus,
#endif
#ifdef ENABLE_BUS_BF533_EZKIT
	&bf533_ezkit_bus,
#endif
#ifdef ENABLE_BUS_BF537_STAMP
	&bf537_stamp_bus,
#endif
#ifdef ENABLE_BUS_BF537_EZKIT
	&bf537_ezkit_bus,
#endif
#ifdef ENABLE_BUS_BF548_EZKIT
	&bf548_ezkit_bus,
#endif
#ifdef ENABLE_BUS_BF561_EZKIT
	&bf561_ezkit_bus,
#endif
#ifdef ENABLE_BUS_EJTAG
	&ejtag_bus,
#endif
#ifdef ENABLE_BUS_FJMEM
	&fjmem_bus,
#endif
#ifdef ENABLE_BUS_IXP425
	&ixp425_bus,
#endif
#ifdef ENABLE_BUS_JOPCYC
	&jopcyc_bus,
#endif
#ifdef ENABLE_BUS_H7202
	&h7202_bus,
#endif
#ifdef ENABLE_BUS_LH7A400
	&lh7a400_bus,
#endif
#ifdef ENABLE_BUS_MPC5200
	&mpc5200_bus,
#endif
#ifdef ENABLE_BUS_MPC824X
	&mpc824x_bus,
#endif
#ifdef ENABLE_BUS_PPC405EP
	&ppc405ep_bus,
#endif
#ifdef ENABLE_BUS_PPC440GX_EBC8
	&ppc440gx_ebc8_bus,
#endif
#ifdef ENABLE_BUS_PROTOTYPE
	&prototype_bus,
#endif
#ifdef ENABLE_BUS_PXA2X0
	&pxa2x0_bus,
#endif
#ifdef ENABLE_BUS_PXA27X
	&pxa27x_bus,
#endif
#ifdef ENABLE_BUS_S3C4510
	&s3c4510_bus,
#endif
#ifdef ENABLE_BUS_SA1110
	&sa1110_bus,
#endif
#ifdef ENABLE_BUS_SH7727
	&sh7727_bus,
#endif
#ifdef ENABLE_BUS_SH7750R
	&sh7750r_bus,
#endif
#ifdef ENABLE_BUS_SH7751R
	&sh7751r_bus,
#endif
#ifdef ENABLE_BUS_SHARC_21065L
	&sharc_21065L_bus,
#endif
#ifdef ENABLE_BUS_SLSUP3
	&slsup3_bus,
#endif
#ifdef ENABLE_BUS_TX4925
	&tx4925_bus,
#endif
#ifdef ENABLE_BUS_ZEFANT_XS3
	&zefant_xs3_bus,
#endif
	NULL			/* last must be NULL */
};

bus_t *bus = NULL;
buses_t buses = {0, NULL};

void buses_free( void )
{
	int i;

	for (i = 0; i < buses.len; i++)
		bus_free( buses.buses[i] );

	free( buses.buses );
	buses.len = 0;
	buses.buses = NULL;
	bus = NULL;
}

void buses_add( bus_t *abus )
{
	bus_t **b;

	if (abus == NULL)
		return;

	b = realloc( buses.buses, (buses.len + 1) * sizeof (bus_t *) );
	if (b == NULL) {
		printf( _("Out of memory\n") );
		return;
	}
	buses.buses = b;
	buses.buses[buses.len++] = abus;
	if (bus == NULL)
		bus = abus;
}

void buses_delete( bus_t *abus )
{
	int i;
	bus_t **b;

	for (i = 0; i < buses.len; i++)
		if (abus == buses.buses[i])
			break;
	if (i >= buses.len)
		return;

	while (i + 1 < buses.len) {
		buses.buses[i] = buses.buses[i + 1];
		i++;
	}
	buses.len--;
	b = realloc( buses.buses, buses.len * sizeof (bus_t *) );
	if ((b != NULL) || (buses.len == 0))
		buses.buses = b;

	if (bus != abus)
		return;

	if (buses.len > 0)
		bus = buses.buses[0];
}
