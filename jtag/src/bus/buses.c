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
	&au1500_bus,
	&bcm1250_bus,
	&bf533_stamp_bus,
	&bf533_ezkit_bus,
	&ixp425_bus,
	&lh7a400_bus,
	&mpc824x_bus,
	&mpc5200_bus,
	&ppc440gx_ebc8_bus,
	&pxa2x0_bus,
	&pxa27x_bus,
	&s3c4510_bus,
	&sa1110_bus,
	&sh7727_bus,
	&sh7750r_bus,
	&sh7751r_bus,
	&slsup3_bus,
	&tx4925_bus,
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
