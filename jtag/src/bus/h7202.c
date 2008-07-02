/*
 * Copyright (C) 2005, Raphael Mack
 * Work heavily based on file sa1110.c
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
 * Written by Raphael Mack <mail AT raphael-mack DOT de>
 *
 * Documentation:
 * [1] MagnaChip Semiconductor Ltd. "HMS30C7202"
 *
 */

//#include "sysdep.h"

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "part.h"
#include "bus.h"
#include "chain.h"
#include "bssignal.h"
#include "jtag.h"
#include "buses.h"
#include "generic_bus.h"

typedef struct {
	signal_t *a[25];
	signal_t *d[32];
	signal_t *nRCS[4];
	signal_t *nRWE[4];
	signal_t *nROE;
} bus_params_t;

#define	A	((bus_params_t *) bus->params)->a
#define	D	((bus_params_t *) bus->params)->d
#define	nRCS	((bus_params_t *) bus->params)->nRCS
#define	nRWE	((bus_params_t *) bus->params)->nRWE
#define	nROE	((bus_params_t *) bus->params)->nROE

/**
 * bus->driver->(*new_bus)
 *
 */
static bus_t *
h7202_bus_new( chain_t *chain, const bus_driver_t *driver, char *cmd_params[] )
{
	bus_t *bus;
	part_t *part;
	char buff[10];
	int i;
	int failed = 0;

	bus = calloc( 1, sizeof (bus_t) );
	if (!bus)
		return NULL;

	bus->driver = driver;
	bus->params = calloc( 1, sizeof (bus_params_t) );
	if (!bus->params) {
		free( bus );
		return NULL;
	}

	CHAIN = chain;
	PART = part = chain->parts->parts[chain->active_part];

	for (i = 0; i < 25; i++) {
		sprintf( buff, "RA%d", i );
		failed |= generic_bus_attach_sig( part, &(A[i]), buff );
	}

	for (i = 0; i < 32; i++) {
		sprintf( buff, "RD%d", i );
		failed |= generic_bus_attach_sig( part, &(D[i]), buff );
	}

	for (i = 0; i < 4; i++) {
		sprintf( buff, "nRCS%d", i );
		failed |= generic_bus_attach_sig( part, &(nRCS[i]), buff );
	}

	failed |= generic_bus_attach_sig( part, &(nROE), "nROE" );

	for (i = 0; i < 4; i++){
	  sprintf( buff, "nRWE%d", i);
		failed |= generic_bus_attach_sig( part, &(nRWE[i]), buff );
	}

	if (failed) {
		free( bus->params );
		free( bus );
		return NULL;
	}

	return bus;
}

/**
 * bus->driver->(*printinfo)
 *
 */
static void
h7202_bus_printinfo( bus_t *bus )
{
	int i;

	for (i = 0; i < CHAIN->parts->len; i++)
		if (PART == CHAIN->parts->parts[i])
			break;
	printf( "H7202 compatible bus driver via BSR (JTAG part No. %d)\n", i );
}

/**
 * bus->driver->(*area)
 *
 */
static int
h7202_bus_area( bus_t *bus, uint32_t adr, bus_area_t *area )
{
	area->description = NULL;
	area->start = UINT32_C(0x00000000);
	area->length = UINT64_C(0x100000000);
	area->width = 16; //part_get_signal( PART, part_find_signal( PART, "ROM_SEL" ) ) ? 32 : 16;

	return 0;
}

static void
setup_address( bus_t *bus, uint32_t a )
{
	int i;
	part_t *p = PART;

	for (i = 0; i < 26; i++)
		part_set_signal( p, A[i], 1, (a >> i) & 1 );
}

static void
set_data_in( bus_t *bus )
{
	int i;
	part_t *p = PART;
	bus_area_t area;

	h7202_bus_area( bus, 0, &area );

	for (i = 0; i < area.width; i++)
		part_set_signal( p, D[i], 0, 0 );
}

static void
setup_data( bus_t *bus, uint32_t d )
{
	int i;
	part_t *p = PART;
	bus_area_t area;

	h7202_bus_area( bus, 0, &area );

	for (i = 0; i < area.width; i++)
		part_set_signal( p, D[i], 1, (d >> i) & 1 );
}

/**
 * bus->driver->(*read_start)
 *
 */
static void
h7202_bus_read_start( bus_t *bus, uint32_t adr )
{
	/* see Figure 10-12 in [1] */
	part_t *p = PART;
	chain_t *chain = CHAIN;

	part_set_signal( p, nRCS[0], 1, 0 );
	part_set_signal( p, nRCS[1], 1, 1 );
	part_set_signal( p, nRCS[2], 1, 1 );
	part_set_signal( p, nRCS[3], 1, 1 );
	part_set_signal( p, nRWE[0], 1, 1 );
	part_set_signal( p, nROE, 1, 0 );

	setup_address( bus, adr );
	set_data_in( bus );

	chain_shift_data_registers( chain, 0 );
}

/**
 * bus->driver->(*read_next)
 *
 */
static uint32_t
h7202_bus_read_next( bus_t *bus, uint32_t adr )
{
	/* see Figure 10-12 in [1] */
	part_t *p = PART;
	chain_t *chain = CHAIN;
	int i;
	uint32_t d = 0;
	bus_area_t area;

	h7202_bus_area( bus, adr, &area );

	setup_address( bus, adr );
	chain_shift_data_registers( chain, 1 );

	for (i = 0; i < area.width; i++)
		d |= (uint32_t) (part_get_signal( p, D[i] ) << i);

	return d;
}

/**
 * bus->driver->(*read_end)
 *
 */
static uint32_t
h7202_bus_read_end( bus_t *bus )
{
	/* see Figure 10-12 in [1] */
	part_t *p = PART;
	chain_t *chain = CHAIN;
	int i;
	uint32_t d = 0;
	bus_area_t area;

	h7202_bus_area( bus, 0, &area );

	part_set_signal( p, nRCS[0], 1, 1 );
	part_set_signal( p, nRCS[1], 1, 1 );
	part_set_signal( p, nRCS[2], 1, 1 );
	part_set_signal( p, nRCS[3], 1, 1 );
	part_set_signal( p, nROE, 1, 1 );
	chain_shift_data_registers( chain, 1 );

	for (i = 0; i < area.width; i++)
		d |= (uint32_t) (part_get_signal( p, D[i] ) << i);

	return d;
}

/**
 * bus->driver->(*write)
 *
 */
static void
h7202_bus_write( bus_t *bus, uint32_t adr, uint32_t data )
{
	/* see Figure 10-16 in [1] */
	part_t *p = PART;
	chain_t *chain = CHAIN;

	//	part_set_signal( p, nRCS[0], 1, (adr >> 27) != 0 );
	//part_set_signal( p, nRCS[1], 1, (adr >> 27) != 1 );
	//part_set_signal( p, nRCS[2], 1, (adr >> 27) != 2 );
	//part_set_signal( p, nRCS[3], 1, (adr >> 27) != 3 );
	part_set_signal( p, nRCS[0], 1, 0 );
	part_set_signal( p, nRCS[1], 1, 1 );
	part_set_signal( p, nRCS[2], 1, 1 );
	part_set_signal( p, nRCS[3], 1, 1 );

	part_set_signal( p, nRWE[0], 1, 0 );
	part_set_signal( p, nRWE[1], 1, 1 );
	part_set_signal( p, nRWE[2], 1, 1 );
	part_set_signal( p, nRWE[3], 1, 1 );
	part_set_signal( p, nROE, 1, 1 );

	setup_address( bus, adr );
	setup_data( bus, data );

	chain_shift_data_registers( chain, 0 );

	part_set_signal( p, nRWE[0], 1, 1 );
	part_set_signal( p, nRCS[0], 1, 1 );
	part_set_signal( p, nRCS[1], 1, 1 );
	part_set_signal( p, nRCS[2], 1, 1 );
	part_set_signal( p, nRCS[3], 1, 1 );
	chain_shift_data_registers( chain, 0 );
}

const bus_driver_t h7202_bus = {
	"h7202",
	"H7202 compatible bus driver via BSR",
	h7202_bus_new,
	generic_bus_free,
	h7202_bus_printinfo,
	generic_bus_prepare_extest,
	h7202_bus_area,
	h7202_bus_read_start,
	h7202_bus_read_next,
	h7202_bus_read_end,
	generic_bus_read,
	h7202_bus_write,
	generic_bus_no_init
};
