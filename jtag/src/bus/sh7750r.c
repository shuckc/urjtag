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
 * Written by Matan Ziv-Av <matan@svgalib.org>, 2003.
 * Modified by Marcel Telka <marcel@telka.sk>, 2003.
 *
 */

#include "sysdep.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "part.h"
#include "bus.h"
#include "chain.h"
#include "bssignal.h"
#include "jtag.h"
#include "buses.h"
#include "generic_bus.h"

typedef struct {
	signal_t *a[26];
	signal_t *d[64];
	signal_t *cs[7];
	signal_t *we[4];
	signal_t *rdwr;
	signal_t *rd;
	signal_t *rdwr2;
	signal_t *rd2;
} bus_params_t;

#define	A	((bus_params_t *) bus->params)->a
#define	D	((bus_params_t *) bus->params)->d
#define	CS	((bus_params_t *) bus->params)->cs
#define	WE	((bus_params_t *) bus->params)->we
#define	RDWR	((bus_params_t *) bus->params)->rdwr
#define	RD	((bus_params_t *) bus->params)->rd
#define	RDWR2	((bus_params_t *) bus->params)->rdwr2
#define	RD2	((bus_params_t *) bus->params)->rd2

/**
 * bus->driver->(*new_bus)
 *
 */
static bus_t *
sh7750r_bus_new( chain_t *chain, const bus_driver_t *driver, char *cmd_params[] )
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

	for (i = 0; i < 26; i++) {
		sprintf( buff, "A%d", i );
		failed |= generic_bus_attach_sig( part, &(A[i]), buff );
	}

	for (i = 0; i < 32; i++) {
		sprintf( buff, "D%d", i );
		failed |= generic_bus_attach_sig( part, &(D[i]), buff );
	}

	for (i = 0; i < 7; i++) {
		sprintf( buff, "CS%d", i );
		failed |= generic_bus_attach_sig( part, &(CS[i]), buff );
	}

	for (i = 0; i < 8; i++) {
		sprintf( buff, "WE%d", i );
		failed |= generic_bus_attach_sig( part, &(WE[i]), buff );
	}

	failed |= generic_bus_attach_sig( part, &(RDWR),  "RDWR"  );

	failed |= generic_bus_attach_sig( part, &(RD),    "RD"    );

	failed |= generic_bus_attach_sig( part, &(RDWR2), "RDWR2" );

	failed |= generic_bus_attach_sig( part, &(RD2),   "RD2"   );

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
sh7750r_bus_printinfo( bus_t *bus )
{
	int i;

	for (i = 0; i < CHAIN->parts->len; i++)
		if (PART == CHAIN->parts->parts[i])
			break;
	printf( _("Hitachi SH7750R compatible bus driver via BSR (JTAG part No. %d)\n"), i );
}

/**
 * bus->driver->(*area)
 *
 */
static int
sh7750r_bus_area( bus_t *bus, uint32_t adr, bus_area_t *area )
{
	area->description = NULL;
	area->start = UINT32_C(0x00000000);
	area->length = UINT64_C(0x100000000);
	area->width = 32;

	return URJTAG_STATUS_OK;
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

	for (i = 0; i < 32; i++)
		part_set_signal( p, D[i], 0, 0 );
}

static void
setup_data( bus_t *bus, uint32_t d )
{
	int i;
	part_t *p = PART;

	for (i = 0; i < 32; i++)
		part_set_signal( p, D[i], 1, (d >> i) & 1 );
}

/**
 * bus->driver->(*read_start)
 *
 */
static void
sh7750r_bus_read_start( bus_t *bus, uint32_t adr )
{
	part_t *p = PART;
	int cs[8];
	int i;

	for (i = 0; i < 8; i++)
		cs[i] = 1;
	cs[(adr & 0x1C000000) >> 26] = 0;

	part_set_signal( p, CS[0], 1, cs[0] );
	part_set_signal( p, CS[1], 1, cs[1] );
	part_set_signal( p, CS[2], 1, cs[2] );
	part_set_signal( p, CS[3], 1, cs[3] );
	part_set_signal( p, CS[4], 1, cs[4] );
	part_set_signal( p, CS[5], 1, cs[5] );
	part_set_signal( p, CS[6], 1, cs[6] );
	part_set_signal( p, RDWR, 1, 1 );
	part_set_signal( p, RDWR2, 1, 1 );
	part_set_signal( p, WE[0], 1, 1 );
	part_set_signal( p, WE[1], 1, 1 );
	part_set_signal( p, WE[2], 1, 1 );
	part_set_signal( p, WE[3], 1, 1 );
	part_set_signal( p, RD, 1, 0 );
	part_set_signal( p, RD2, 1, 0 );

	setup_address( bus, adr );
	set_data_in( bus );
	chain_shift_data_registers( CHAIN, 0 );
}

/**
 * bus->driver->(*read_next)
 *
 */
static uint32_t
sh7750r_bus_read_next( bus_t *bus, uint32_t adr )
{
	part_t *p = PART;
	int i;
	uint32_t d = 0;

	setup_address( bus, adr );
	chain_shift_data_registers( CHAIN, 1 );

	for (i = 0; i < 32; i++)
		d |= (uint32_t) (part_get_signal( p, D[i] ) << i);

	return d;
}

/**
 * bus->driver->(*read_end)
 *
 */
static uint32_t
sh7750r_bus_read_end( bus_t *bus )
{
	part_t *p = PART;
	int cs[8];
	int i;
	uint32_t d = 0;

	for (i = 0; i < 8; i++)
		cs[i] = 1;

	part_set_signal( p, CS[0], 1, cs[0] );
	part_set_signal( p, CS[1], 1, cs[1] );
	part_set_signal( p, CS[2], 1, cs[2] );
	part_set_signal( p, CS[3], 1, cs[3] );
	part_set_signal( p, CS[4], 1, cs[4] );
	part_set_signal( p, CS[5], 1, cs[5] );
	part_set_signal( p, CS[6], 1, cs[6] );

	part_set_signal( p, RD, 1, 1 );
	part_set_signal( p, RD2, 1, 1 );
	chain_shift_data_registers( CHAIN, 1 );

	for (i = 0; i < 32; i++)
		d |= (uint32_t) (part_get_signal( p, D[i] ) << i);

	return d;
}

/**
 * bus->driver->(*write)
 *
 */
static void
sh7750r_bus_write( bus_t *bus, uint32_t adr, uint32_t data )
{
	chain_t *chain = CHAIN;
	part_t *p = PART;
	int cs[8];
	int i;

	for (i = 0; i < 8 ; i++)
		cs[i] = 1;
	cs[(adr & 0x1C000000) >> 26] = 0;

	part_set_signal( p, CS[0], 1, cs[0] );
	part_set_signal( p, CS[1], 1, cs[1] );
	part_set_signal( p, CS[2], 1, cs[2] );
	part_set_signal( p, CS[3], 1, cs[3] );
	part_set_signal( p, CS[4], 1, cs[4] );
	part_set_signal( p, CS[5], 1, cs[5] );
	part_set_signal( p, CS[6], 1, cs[6] );

	part_set_signal( p, RDWR, 1, 0 );
	part_set_signal( p, RDWR2, 1, 0 );
	part_set_signal( p, WE[0], 1, 1 );
	part_set_signal( p, WE[1], 1, 1 );
	part_set_signal( p, WE[2], 1, 1 );
	part_set_signal( p, WE[3], 1, 1 );
	part_set_signal( p, RD, 1, 1 );
	part_set_signal( p, RD2, 1, 1 );

	setup_address( bus, adr );
	setup_data( bus, data );

	chain_shift_data_registers( chain, 0 );

	part_set_signal( p, WE[0], 1, 0 );
	part_set_signal( p, WE[1], 1, 0 );
	part_set_signal( p, WE[2], 1, 0 );
	part_set_signal( p, WE[3], 1, 0 );

	chain_shift_data_registers( chain, 0 );

	part_set_signal( p, WE[0], 1, 1 );
	part_set_signal( p, WE[1], 1, 1 );
	part_set_signal( p, WE[2], 1, 1 );
	part_set_signal( p, WE[3], 1, 1 );

	chain_shift_data_registers( chain, 0 );
}

const bus_driver_t sh7750r_bus = {
	"sh7750r",
	N_("Hitachi SH7750R compatible bus driver via BSR"),
	sh7750r_bus_new,
	generic_bus_free,
	sh7750r_bus_printinfo,
	generic_bus_prepare_extest,
	sh7750r_bus_area,
	sh7750r_bus_read_start,
	sh7750r_bus_read_next,
	sh7750r_bus_read_end,
	generic_bus_read,
	sh7750r_bus_write,
	generic_bus_no_init
};
