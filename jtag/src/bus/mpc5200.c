/*
 * $Id$
 *
 * Freescale MPC5200 compatible bus driver via BSR
 * Copyright (C) 2003 Marcel Telka
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
 * Written by Asier Llano <a.llano@usyscom.com>, 2004.
 *
 * Documentation:
 * [1] Freescale, "Freescale MPC5200 Users Guide", Rev. 2, 08/2004
 *     Order Number: MPC5200UG
 *
 */

#include "sysdep.h"

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "part.h"
#include "bus.h"
#include "bssignal.h"
#include "jtag.h"
#include "buses.h"
#include "generic_bus.h"

typedef struct {
	chain_t *chain;
	part_t *part;
	uint32_t last_adr;
	signal_t *ad[24];
	signal_t *ncs[4];
	signal_t *nwe;
	signal_t *noe;
	signal_t *d[8];
} bus_params_t;

#define	CHAIN		((bus_params_t *) bus->params)->chain
#define	PART		((bus_params_t *) bus->params)->part
#define	LAST_ADR	((bus_params_t *) bus->params)->last_adr
#define	AD		((bus_params_t *) bus->params)->ad
#define	nCS		((bus_params_t *) bus->params)->ncs
#define	nWE		((bus_params_t *) bus->params)->nwe
#define	nOE		((bus_params_t *) bus->params)->noe
#define	D		((bus_params_t *) bus->params)->d

/**
 * bus->driver->(*new_bus)
 *
 */
static bus_t *
mpc5200_bus_new( chain_t *chain, char *cmd_params[] )
{
	bus_t *bus;
	char buff[10];
	int i;
	int failed = 0;
	part_t *part;

	if (!chain || !chain->parts || chain->parts->len <= chain->active_part || chain->active_part < 0)
		return NULL;

	bus = calloc( 1, sizeof (bus_t) );
	if (!bus)
		return NULL;

	bus->driver = &mpc5200_bus;
	bus->params = calloc( 1, sizeof (bus_params_t) );
	if (!bus->params) {
		free( bus );
		return NULL;
	}

	CHAIN = chain;
	PART = part = chain->parts->parts[chain->active_part];

	/* Get the signals */
	for (i = 0; i < 24; i++) {
		sprintf( buff, "EXT_AD_%d", i );
		AD[i] = part_find_signal( part, buff );
		if (!AD[i]) {
			printf( _("signal '%s' not found\n"), buff );
			failed = 1;
			break;
		}
	}
	for (i = 0; i < 4; i++) {
		sprintf( buff, "LP_CS%d_B", i );
		nCS[i] = part_find_signal( part, buff );
		if (!nCS[i]) {
			printf( _("signal '%s' not found\n"), buff );
			failed = 1;
			break;
		}
	}
	nWE = part_find_signal( part, "LP_RW" );
	if (!nWE) {
		printf( _("signal '%s' not found\n"), "LP_RW" );
		failed = 1;
	}
	nOE = part_find_signal( part, "LP_OE" );
	if (!nOE) {
		printf( _("signal '%s' not found\n"), "LP_OE" );
		failed = 1;
	}
	for (i = 0; i < 8; i++) {
		sprintf( buff, "EXT_AD_%d", i+24 );
		D[i] = part_find_signal( part, buff );
		if (!D[i]) {
			printf( _("signal '%s' not found\n"), buff );
			failed = 1;
			break;
		}
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
mpc5200_bus_printinfo( bus_t *bus )
{
	int i;

	for (i = 0; i < CHAIN->parts->len; i++)
		if (PART == CHAIN->parts->parts[i])
			break;
	printf( _("Freescale MPC5200 compatible bus driver via BSR (JTAG part No. %d)\n"), i );
}

/**
 * bus->driver->(*prepare)
 *
 */
static void
mpc5200_bus_prepare( bus_t *bus )
{
	part_set_instruction( PART, "EXTEST" );
	chain_shift_instructions( CHAIN );
}

/**
 * bus->driver->(*area)
 *
 */
static int
mpc5200_bus_area( bus_t *bus, uint32_t adr, bus_area_t *area )
{
	if( adr < UINT32_C(0x01000000) )
	{
		area->description = N_("LocalPlus Bus");
		area->start = UINT32_C(0x00000000);
		area->length = UINT64_C(0x01000000);
		area->width = 8;
		return 0;
	}

	area->description = NULL;
	area->start = 0x01000000;
	area->length = 0xFF000000;
	area->width = 0;
	return 0;
}

static void
setup_address( bus_t *bus, uint32_t a )
{
	int i;
	part_t *p = PART;

	for (i = 0; i < 24; i++)
		part_set_signal( p, AD[i], 1, (a >> i) & 1 );
}

static void
set_data_in( bus_t *bus, uint32_t adr )
{
	int i;
	part_t *p = PART;
	bus_area_t area;

	mpc5200_bus_area( bus, adr, &area );
	if (area.width > 8)
		return;

	for (i = 0; i < area.width; i++)
		part_set_signal( p, D[i], 0, 0 );
}

static void
setup_data( bus_t *bus, uint32_t adr, uint32_t d )
{
	int i;
	part_t *p = PART;
	bus_area_t area;

	mpc5200_bus_area( bus, adr, &area );
	if (area.width > 8)
		return;

	for (i = 0; i < area.width; i++)
		part_set_signal( p, D[i], 1, (d >> i) & 1 );
}

static uint32_t
get_data( bus_t *bus, uint32_t adr )
{
	bus_area_t area;
	int i;
	uint32_t d = 0;
	part_t *p = PART;

	mpc5200_bus_area( bus, adr, &area );
	if (area.width > 8)
		return 0;

	for (i = 0; i < area.width; i++)
		d |= (uint32_t) (part_get_signal( p, D[i] ) << i);

	return d;
}

/**
 * bus->driver->(*read_start)
 *
 */
static void
mpc5200_bus_read_start( bus_t *bus, uint32_t adr )
{
	part_t *p = PART;

	LAST_ADR = adr;

	/* see Figure 6-45 in [1] */
	part_set_signal( p, nCS[0], 1, 0 );
	part_set_signal( p, nCS[1], 1, 1 );
	part_set_signal( p, nCS[2], 1, 1 );
	part_set_signal( p, nCS[3], 1, 1 );
	part_set_signal( p, nWE, 1, 1 );
	part_set_signal( p, nOE, 1, 0 );

	setup_address( bus, adr );
	set_data_in( bus, adr );

	chain_shift_data_registers( CHAIN, 0 );
}

/**
 * bus->driver->(*read_next)
 *
 */
static uint32_t
mpc5200_bus_read_next( bus_t *bus, uint32_t adr )
{
	uint32_t d;

	setup_address( bus, adr );
	chain_shift_data_registers( CHAIN, 1 );

	d = get_data( bus, LAST_ADR );
	LAST_ADR = adr;
	return d;
}

/**
 * bus->driver->(*read_end)
 *
 */
static uint32_t
mpc5200_bus_read_end( bus_t *bus )
{
	part_t *p = PART;

	part_set_signal( p, nCS[0], 1, 1 );
	part_set_signal( p, nOE, 1, 1 );

	chain_shift_data_registers( CHAIN, 1 );

	return get_data( bus, LAST_ADR );
}

/**
 * bus->driver->(*read)
 *
 */
static uint32_t
mpc5200_bus_read( bus_t *bus, uint32_t adr )
{
	mpc5200_bus_read_start( bus, adr );
	return mpc5200_bus_read_end( bus );
}

/**
 * bus->driver->(*write)
 *
 */
static void
mpc5200_bus_write( bus_t *bus, uint32_t adr, uint32_t data )
{
	/* see Figure 6-47 in [1] */
	part_t *p = PART;
	chain_t *chain = CHAIN;

	part_set_signal( p, nCS[0], 1, 0 );
	part_set_signal( p, nCS[1], 1, 1 );
	part_set_signal( p, nCS[2], 1, 1 );
	part_set_signal( p, nCS[3], 1, 1 );
	part_set_signal( p, nWE, 1, 1 );
	part_set_signal( p, nOE, 1, 1 );

	setup_address( bus, adr );
	setup_data( bus, adr, data );

	chain_shift_data_registers( chain, 0 );

	part_set_signal( p, nWE, 1, 0 );
	chain_shift_data_registers( chain, 0 );
	part_set_signal( p, nWE, 1, 1 );
	chain_shift_data_registers( chain, 0 );
}

const bus_driver_t mpc5200_bus = {
	"mpc5200",
	N_("Freescale MPC5200 compatible bus driver via BSR"),
	mpc5200_bus_new,
	generic_bus_free,
	mpc5200_bus_printinfo,
	mpc5200_bus_prepare,
	mpc5200_bus_area,
	mpc5200_bus_read_start,
	mpc5200_bus_read_next,
	mpc5200_bus_read_end,
	mpc5200_bus_read,
	mpc5200_bus_write,
    NULL
};
