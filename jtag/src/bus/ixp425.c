/*
 * $Id$
 *
 * Copyright (C) 2002 ETC s.r.o.
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
 * Written by Christian Pellegrin <chri@ascensit.com>, 2003.
 * Modified by Marcel Telka <marcel@telka.sk>, 2003.
 *
 */

#include "sysdep.h"

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "part.h"
#include "bus.h"
#include "chain.h"
#include "bssignal.h"
#include "jtag.h"
#include "buses.h"

typedef struct {
	chain_t *chain;
	part_t *part;
	signal_t *ex_cs[8];
	signal_t *ex_addr[24];
	signal_t *ex_data[16];
	signal_t *ex_wr;
	signal_t *ex_rd;
} bus_params_t;

#define	CHAIN	((bus_params_t *) bus->params)->chain
#define	PART	((bus_params_t *) bus->params)->part
#define	EX_CS	((bus_params_t *) bus->params)->ex_cs
#define	EX_ADDR	((bus_params_t *) bus->params)->ex_addr
#define	EX_DATA	((bus_params_t *) bus->params)->ex_data
#define	EX_WR	((bus_params_t *) bus->params)->ex_wr
#define	EX_RD	((bus_params_t *) bus->params)->ex_rd

static void 
select_flash( bus_t *bus )
{
	part_t *p = PART;

	part_set_signal( p, EX_CS[0], 1, 0 );
	part_set_signal( p, EX_CS[1], 1, 1 );
	part_set_signal( p, EX_CS[2], 1, 1 );
	part_set_signal( p, EX_CS[3], 1, 1 );
	part_set_signal( p, EX_CS[4], 1, 1 );
	part_set_signal( p, EX_CS[5], 1, 1 );
	part_set_signal( p, EX_CS[6], 1, 1 );
	part_set_signal( p, EX_CS[7], 1, 1 );
}

static void 
unselect_flash( bus_t *bus )
{
	part_t *p = PART;

	part_set_signal( p, EX_CS[0], 1, 1 );
	part_set_signal( p, EX_CS[1], 1, 1 );
	part_set_signal( p, EX_CS[2], 1, 1 );
	part_set_signal( p, EX_CS[3], 1, 1 );
	part_set_signal( p, EX_CS[4], 1, 1 );
	part_set_signal( p, EX_CS[5], 1, 1 );
	part_set_signal( p, EX_CS[6], 1, 1 );
	part_set_signal( p, EX_CS[7], 1, 1 );
}

static void
setup_address( bus_t *bus, uint32_t a )
{
	int i;
	part_t *p = PART;

	for (i = 0; i < 24; i++)
		part_set_signal( p, EX_ADDR[i], 1, (a >> i) & 1 );
}

static void
set_data_in( bus_t *bus )
{
	int i;
	part_t *p = PART;

	for (i = 0; i < 16; i++)
		part_set_signal( p, EX_DATA[i], 0, 0 );
}

static void
setup_data( bus_t *bus, uint32_t d )
{
	int i;
	part_t *p = PART;

	for (i = 0; i < 16; i++)
		part_set_signal( p, EX_DATA[i], 1, (d >> i) & 1 );
}

static void
ixp425_bus_printinfo( bus_t *bus )
{
	int i;

	for (i = 0; i < CHAIN->parts->len; i++)
		if (PART == CHAIN->parts->parts[i])
			break;
	printf( _("Intel IXP425 compatible bus driver via BSR (JTAG part No. %d)\n"), i );
}

static void
ixp425_bus_prepare( bus_t *bus )
{
	part_set_instruction( PART, "EXTEST" );
	chain_shift_instructions( CHAIN );
}

static void
ixp425_bus_read_start( bus_t *bus, uint32_t adr )
{
	part_t *p = PART;
	chain_t *chain = CHAIN;

	select_flash( bus );
	part_set_signal( p, EX_RD, 1, 0 );
	part_set_signal( p, EX_WR, 1, 1 );

	setup_address( bus, adr );
	set_data_in( bus );

	chain_shift_data_registers( chain, 0 );
}

static uint32_t
ixp425_bus_read_next( bus_t *bus, uint32_t adr )
{
	part_t *p = PART;
	chain_t *chain = CHAIN;
	int i;
	uint32_t d = 0;

	setup_address( bus, adr );
	chain_shift_data_registers( chain, 1 );

	for (i = 0; i < 16; i++)
		d |= (uint32_t) (part_get_signal( p, EX_DATA[i] ) << i);

	return d;
}

static uint32_t
ixp425_bus_read_end( bus_t *bus )
{
	part_t *p = PART;
	chain_t *chain = CHAIN;
	int i;
	uint32_t d = 0;

	unselect_flash( bus );
	part_set_signal( p, EX_RD, 1, 1 );
	part_set_signal( p, EX_WR, 1, 1 );

	chain_shift_data_registers( chain, 1 );

	for (i = 0; i < 16; i++)
		d |= (uint32_t) (part_get_signal( p, EX_DATA[i] ) << i);

	return d;
}

static uint32_t
ixp425_bus_read( bus_t *bus, uint32_t adr )
{
	ixp425_bus_read_start( bus, adr );
	return ixp425_bus_read_end( bus );
}

static void
ixp425_bus_write( bus_t *bus, uint32_t adr, uint32_t data )
{
	part_t *p = PART;
	chain_t *chain = CHAIN;

	select_flash( bus );
	part_set_signal( p, EX_RD, 1, 1 );

	setup_address( bus, adr );
	setup_data( bus, data );

	chain_shift_data_registers( chain, 0 );

	part_set_signal( p, EX_WR, 1, 0 );
	chain_shift_data_registers( chain, 0 );
	part_set_signal( p, EX_WR, 1, 1 );
	unselect_flash( bus );
	chain_shift_data_registers( chain, 0 );
}

static int
ixp425_bus_area( bus_t *bus, uint32_t adr, bus_area_t *area )
{
	area->description = NULL;
	area->start = UINT32_C(0x00000000);
	area->length = UINT64_C(0x100000000);
	area->width = 16;

	return 0;
}

static void
ixp425_bus_free( bus_t *bus )
{
	free( bus->params );
	free( bus );
}

static bus_t *ixp425_bus_new( void );

const bus_driver_t ixp425_bus = {
	"ixp425",
	N_("Intel IXP425 compatible bus driver via BSR"),
	ixp425_bus_new,
	ixp425_bus_free,
	ixp425_bus_printinfo,
	ixp425_bus_prepare,
	ixp425_bus_area,
	ixp425_bus_read_start,
	ixp425_bus_read_next,
	ixp425_bus_read_end,
	ixp425_bus_read,
	ixp425_bus_write
};

static bus_t *
ixp425_bus_new( void )
{
	bus_t *bus;
	char buff[15];
	int i;
	int failed = 0;

	if (!chain || !chain->parts || chain->parts->len <= chain->active_part || chain->active_part < 0)
		return NULL;

	bus = malloc( sizeof (bus_t) );
	if (!bus)
		return NULL;

	bus->driver = &ixp425_bus;
	bus->params = malloc( sizeof (bus_params_t) );
	if (!bus->params) {
		free( bus );
		return NULL;
	}

	CHAIN = chain;
	PART = chain->parts->parts[chain->active_part];

	for (i = 0; i < 8; i++) {
		sprintf( buff, "EX_CS[%d]", i );
		EX_CS[i] = part_find_signal( PART, buff );
		if (!EX_CS[i]) {
			printf( _("signal '%s' not found\n"), buff );
			failed = 1;
			break;
		}
	}
	for (i = 0; i < 24; i++) {
		sprintf( buff, "EX_ADDR[%d]", i );
		EX_ADDR[i] = part_find_signal( PART, buff );
		if (!EX_ADDR[i]) {
			printf( _("signal '%s' not found\n"), buff );
			failed = 1;
			break;
		}
	}
	for (i = 0; i < 16; i++) {
		sprintf( buff, "EX_DATA[%d]", i );
		EX_DATA[i] = part_find_signal( PART, buff );
		if (!EX_DATA[i]) {
			printf( _("signal '%s' not found\n"), buff );
			failed = 1;
			break;
		}
	}
	EX_WR = part_find_signal( PART, "EX_WR" );
	if (!EX_WR) {
		printf( _("signal '%s' not found\n"), "EX_WR" );
		failed = 1;
	}
	EX_RD = part_find_signal( PART, "EX_RD" );
	if (!EX_RD) {
		printf( _("signal '%s' not found\n"), "EX_RD" );
		failed = 1;
	}

	if (failed) {
		free( bus->params );
		free( bus );
		return NULL;
	}

	return bus;
}
