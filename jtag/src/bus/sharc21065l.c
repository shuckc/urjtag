/*
 * $Id: sharc21065l.c,v 1.0 20/09/2006 12:38:01  $
 *
 * Analog Device's SHARC 21065L compatible bus driver via BSR
 * Copyright (C) 2006 Kila Medical Systems.
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
 * Written by Ajith Kumar P.C <ajithpc@kila.com>
 *
 * Documentation:
 *      [1] Analog Devices Inc.,"ADSP-21065L SHARC Technical Reference", September 1998
 *      [2] Analog Devices Inc.,"ADSP-21065L SHARC User's Manual", September 1998
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

//no SDRAM access

typedef struct
{
	chain_t *chain;
	part_t *part;
	uint32_t last_adr;
	signal_t *ma[19];	//19 - 512K flash address are used
	signal_t *md[8];	//8 bit data bus connected to Flash are used
	signal_t *bms;	//boot memory select
	signal_t *nwe;
	signal_t *noe;
} bus_params_t;


#define	CHAIN	((bus_params_t *) bus->params)->chain
#define	PART	((bus_params_t *) bus->params)->part
#define	LAST_ADR	((bus_params_t *) bus->params)->last_adr
#define	MA		((bus_params_t *) bus->params)->ma
#define	MD		((bus_params_t *) bus->params)->md
#define	BMS		((bus_params_t *) bus->params)->bms
#define	nWE		((bus_params_t *) bus->params)->nwe
#define	nOE		((bus_params_t *) bus->params)->noe

/**
 * bus->driver->(*new_bus)
 *
 */
static bus_t *sharc_21065L_bus_new( chain_t *chain, char *cmd_params[] )
{
	bus_t *bus;
	part_t *part;
	char buff[15];
	int i;
	int failed = 0;

	bus = calloc( 1, sizeof (bus_t) );
	if (!bus)
		return NULL;

	bus->driver = &sharc_21065L_bus;
	bus->params = calloc( 1, sizeof (bus_params_t) );
	if (!bus->params) {
		free( bus );
		return NULL;
	}

	CHAIN = chain;
	PART = part = chain->parts->parts[chain->active_part];

	for (i = 0; i < 19; i++) {
		sprintf( buff, "ADDR%d", i );
		failed |= generic_bus_attach_sig( part, &(MA[i]), buff );
	}

	for (i = 0; i < 8; i++) {
		sprintf( buff, "DATA%d", i );
		failed |= generic_bus_attach_sig( part, &(MD[i]), buff );
	}

	failed |= generic_bus_attach_sig( part, &(BMS), "BMS_B" );

	failed |= generic_bus_attach_sig( part, &(nWE), "WR_B"  );

	failed |= generic_bus_attach_sig( part, &(nOE), "RD_B"  );

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
static void sharc_21065L_bus_printinfo( bus_t *bus )
{
	int i;

	for (i = 0; i < CHAIN->parts->len; i++)
		if (PART == CHAIN->parts->parts[i])
			break;
	printf( _("Analog Device's SHARC 21065L compatible bus driver via BSR (JTAG part No. %d)\n"), i );
}

/**
 * bus->driver->(*prepare)
 *
 */
static void
sharc_21065L_bus_prepare( bus_t *bus )
{
	part_set_instruction( PART, "EXTEST" );
	chain_shift_instructions( CHAIN );
}

/**
 * bus->driver->(*area)
 *
 */
static int
sharc_21065L_bus_area( bus_t *bus, uint32_t adr, bus_area_t *area )
{
	/* BMS  (512 KB) */
	if (adr < UINT32_C(0x080000)) {
		area->description = N_("Boot Memory Select");
		area->start = UINT32_C(0x000000);
		area->length = UINT64_C(0x080000);
		area->width = 8;

		return 0;
	}

	area->description = NULL;
	area->start = UINT32_C(0xffffffff);
	area->length = UINT64_C(0x080000);
	area->width = 0;
	return 0;
}

static void
setup_address( bus_t *bus, uint32_t a )
{
	int i;
	part_t *p = PART;

	for (i = 0; i < 19; i++)
		part_set_signal( p, MA[i], 1, (a >> i) & 1 );
}

static void
set_data_in( bus_t *bus, uint32_t adr )
{
	int i;
	part_t *p = PART;
	bus_area_t area;

	sharc_21065L_bus_area( bus, adr, &area );

	for (i = 0; i < area.width; i++)
		part_set_signal( p, MD[i], 0, 0 );
}


static void
setup_data( bus_t *bus, uint32_t adr, uint32_t d )
{
	int i;
	part_t *p = PART;
	bus_area_t area;

	sharc_21065L_bus_area( bus, adr, &area );

	for (i = 0; i < area.width; i++)
		part_set_signal( p, MD[i], 1, (d >> i) & 1 );
}

/**
 * bus->driver->(*read_start)
 *
 */
static void
sharc_21065L_bus_read_start( bus_t *bus, uint32_t adr )
{
	chain_t *chain = CHAIN;
	part_t *p = PART;

	LAST_ADR = adr;
	if (adr >= 0x080000)
		return;


	part_set_signal( p, BMS, 1, 0 );
	part_set_signal( p, nWE, 1, 1 );
	part_set_signal( p, nOE, 1, 0 );

	setup_address( bus, adr );
	set_data_in( bus, adr );

	chain_shift_data_registers( chain, 0 );
}

/**
 * bus->driver->(*read_next)
 *
 */
static uint32_t
sharc_21065L_bus_read_next( bus_t *bus, uint32_t adr )
{
	part_t *p = PART;
	chain_t *chain = CHAIN;
	uint32_t d;
	//uint32_t old_last_adr = LAST_ADR;

	LAST_ADR = adr;

	if (adr < UINT32_C(0x080000)) {
		int i;
		bus_area_t area;

		sharc_21065L_bus_area( bus, adr, &area );


		setup_address( bus, adr );
		chain_shift_data_registers( chain, 1 );

		d = 0;
		for (i = 0; i < area.width; i++)
			d |= (uint32_t) (part_get_signal( p, MD[i] ) << i);

		return d;
	}
	return 0;
}

/**
 * bus->driver->(*read_end)
 *
 */
static uint32_t
sharc_21065L_bus_read_end( bus_t *bus )
{
	part_t *p = PART;
	chain_t *chain = CHAIN;

	if (LAST_ADR < UINT32_C(0x080000)) {
		int i;
		uint32_t d = 0;
		bus_area_t area;

		sharc_21065L_bus_area( bus, LAST_ADR, &area );


		part_set_signal( p, BMS, 1, 1 );
		part_set_signal( p, nWE, 1, 1 );
		part_set_signal( p, nOE, 1, 1 );

		chain_shift_data_registers( chain, 1 );

		for (i = 0; i < area.width; i++)
			d |= (uint32_t) (part_get_signal( p, MD[i] ) << i);

		return d;
	}

	return 0;
}

/**
 * bus->driver->(*write)
 *
 */
static void
sharc_21065L_bus_write( bus_t *bus, uint32_t adr, uint32_t data )
{
	part_t *p = PART;
	chain_t *chain = CHAIN;

	if (adr >= 0x080000)
		return;


	part_set_signal( p, BMS, 1, 0 );
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

const bus_driver_t sharc_21065L_bus = {
	"SHARC_21065L",
	N_("SHARC_21065L compatible bus driver via BSR"),
	sharc_21065L_bus_new,
	generic_bus_free,
	sharc_21065L_bus_printinfo,
	sharc_21065L_bus_prepare,
	sharc_21065L_bus_area,
	sharc_21065L_bus_read_start,
	sharc_21065L_bus_read_next,
	sharc_21065L_bus_read_end,
	generic_bus_read,
	sharc_21065L_bus_write,
	NULL
};
