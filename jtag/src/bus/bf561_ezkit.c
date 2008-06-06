/*
 * bf561_ezkit.c
 *
 * Analog Devices ADSP-BF561 EZ-KIT Lite bus driver
 * Copyright (C) 2008 Analog Devices, Inc.
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
 * Written by Jie Zhang <jie.zhang@analog.com>, 2008.
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
#include "generic_bus.h"

typedef struct {
	chain_t *chain;
	part_t *part;
	signal_t *ams[4];
	signal_t *addr[24];
	signal_t *abe[4];
	signal_t *data[32];
	signal_t *awe;
	signal_t *aoe;
	signal_t *sras;
	signal_t *scas;
	signal_t *sms[4];
	signal_t *swe;
} bus_params_t;

#define	CHAIN	((bus_params_t *) bus->params)->chain
#define	PART	((bus_params_t *) bus->params)->part
#define	AMS	((bus_params_t *) bus->params)->ams
#define	ADDR	((bus_params_t *) bus->params)->addr
#define	ABE	((bus_params_t *) bus->params)->abe
#define	DATA	((bus_params_t *) bus->params)->data
#define	AWE	((bus_params_t *) bus->params)->awe
#define	AOE	((bus_params_t *) bus->params)->aoe
#define	SRAS	((bus_params_t *) bus->params)->sras
#define	SCAS	((bus_params_t *) bus->params)->scas
#define	SMS	((bus_params_t *) bus->params)->sms
#define	SWE	((bus_params_t *) bus->params)->swe

static void
select_flash( bus_t *bus )
{
	part_t *p = PART;

	part_set_signal( p, AMS[0], 1, 0 );
	part_set_signal( p, AMS[1], 1, 1 );
	part_set_signal( p, AMS[2], 1, 1 );
	part_set_signal( p, AMS[3], 1, 1 );

	part_set_signal( p, ABE[0], 1, 0 );
	part_set_signal( p, ABE[1], 1, 0 );
	part_set_signal( p, ABE[2], 1, 0 );
	part_set_signal( p, ABE[3], 1, 0 );

	part_set_signal( p, SRAS, 1, 1 );
	part_set_signal( p, SCAS, 1, 1 );
	part_set_signal( p, SWE, 1, 1 );
	part_set_signal( p, SMS[0], 1, 1 );
	part_set_signal( p, SMS[1], 1, 1 );
	part_set_signal( p, SMS[2], 1, 1 );
	part_set_signal( p, SMS[3], 1, 1 );
}

static void
unselect_flash( bus_t *bus )
{
	part_t *p = PART;

	part_set_signal( p, AMS[0], 1, 1 );
	part_set_signal( p, AMS[1], 1, 1 );
	part_set_signal( p, AMS[2], 1, 1 );
	part_set_signal( p, AMS[3], 1, 1 );

	part_set_signal( p, ABE[0], 1, 1 );
	part_set_signal( p, ABE[1], 1, 1 );
	part_set_signal( p, ABE[2], 1, 1 );
	part_set_signal( p, ABE[3], 1, 1 );

	part_set_signal( p, SRAS, 1, 1 );
	part_set_signal( p, SCAS, 1, 1 );
	part_set_signal( p, SWE, 1, 1 );
	part_set_signal( p, SMS[0], 1, 1 );
	part_set_signal( p, SMS[1], 1, 1 );
	part_set_signal( p, SMS[2], 1, 1 );
	part_set_signal( p, SMS[3], 1, 1 );
}

static void
setup_address( bus_t *bus, uint32_t a )
{
	int i;
	part_t *p = PART;

	for (i = 0; i < 24; i++)
		part_set_signal( p, ADDR[i], 1, (a >> (i + 2)) & 1 );
	part_set_signal( p, ABE[3], 1, (a >> 1) & 1 );
}

static void
set_data_in( bus_t *bus )
{
	int i;
	part_t *p = PART;

	for (i = 0; i < 16; i++)
		part_set_signal( p, DATA[i], 0, 0 );
}

static void
setup_data( bus_t *bus, uint32_t d )
{
	int i;
	part_t *p = PART;

	for (i = 0; i < 16; i++)
		part_set_signal( p, DATA[i], 1, (d >> i) & 1 );

}

static void
bf561_ezkit_bus_printinfo( bus_t *bus )
{
	int i;

	for (i = 0; i < CHAIN->parts->len; i++)
		if (PART == CHAIN->parts->parts[i])
			break;
	printf( _("Blackfin BF561 EZ-KIT compatible bus driver via BSR (JTAG part No. %d)\n"), i );
}

static void
bf561_ezkit_bus_prepare( bus_t *bus )
{
	part_set_instruction( PART, "EXTEST" );
	chain_shift_instructions( CHAIN );
}

static void
bf561_ezkit_bus_read_start( bus_t *bus, uint32_t adr )
{
	part_t *p = PART;
	chain_t *chain = CHAIN;

	select_flash( bus );
	part_set_signal( p, AOE, 1, 0 );
	part_set_signal( p, AWE, 1, 1 );

	setup_address( bus, adr );
	set_data_in( bus );

	chain_shift_data_registers( chain, 0 );
}

static uint32_t
bf561_ezkit_bus_read_next( bus_t *bus, uint32_t adr )
{
	part_t *p = PART;
	chain_t *chain = CHAIN;
	int i;
	uint32_t d = 0;

	setup_address( bus, adr );
	chain_shift_data_registers( chain, 1 );

	for (i = 0; i < 16; i++)
		d |= (uint32_t) (part_get_signal( p, DATA[i] ) << i);

	return d;
}

static uint32_t
bf561_ezkit_bus_read_end( bus_t *bus )
{
	part_t *p = PART;
	chain_t *chain = CHAIN;
	int i;
	uint32_t d = 0;

	unselect_flash( bus );
	part_set_signal( p, AOE, 1, 1 );
	part_set_signal( p, AWE, 1, 1 );

	chain_shift_data_registers( chain, 1 );

	for (i = 0; i < 16; i++)
		d |= (uint32_t) (part_get_signal( p, DATA[i] ) << i);

	return d;
}

static uint32_t
bf561_ezkit_bus_read( bus_t *bus, uint32_t adr )
{
	bf561_ezkit_bus_read_start( bus, adr );
	return bf561_ezkit_bus_read_end( bus );
}

static void
bf561_ezkit_bus_write( bus_t *bus, uint32_t adr, uint32_t data )
{
	part_t *p = PART;
	chain_t *chain = CHAIN;

	select_flash( bus );
	part_set_signal( p, AOE, 1, 1 );

	setup_address( bus, adr );
	setup_data( bus, data );

	chain_shift_data_registers( chain, 0 );

	part_set_signal( p, AWE, 1, 0 );
	chain_shift_data_registers( chain, 0 );
	part_set_signal( p, AWE, 1, 1 );
	unselect_flash( bus );
	chain_shift_data_registers( chain, 0 );
}

static int
bf561_ezkit_bus_area( bus_t *bus, uint32_t addr, bus_area_t *area )
{
	area->description = NULL;
	area->start = UINT32_C(0x00000000);
	area->length = UINT64_C(0x100000000);
	area->width = 16;
	return 0;
}

static bus_t *
bf561_ezkit_bus_new( chain_t *chain, char *cmd_params[] )
{
	bus_t *bus;
	char buff[15];
	int i;
	int failed = 0;

	if (!chain || !chain->parts || chain->parts->len <= chain->active_part || chain->active_part < 0)
		return NULL;

	bus = calloc( 1, sizeof (bus_t) );
	if (!bus)
		return NULL;

	bus->driver = &bf561_ezkit_bus;
	bus->params = calloc( 1, sizeof (bus_params_t) );
	if (!bus->params) {
		free( bus );
		return NULL;
	}

	CHAIN = chain;
	PART = chain->parts->parts[chain->active_part];

	for (i = 0; i < 4; i++) {
		sprintf( buff, "AMS_B%d", i );
		AMS[i] = part_find_signal( PART, buff );
		if (!AMS[i]) {
			printf( _("signal '%s' not found\n"), buff );
			failed = 1;
			break;
		}
	}

	for (i = 0; i < 24; i++) {
		sprintf( buff, "ADDR%d", i + 2);
		ADDR[i] = part_find_signal( PART, buff );
		if (!ADDR[i]) {
			printf( _("signal '%s' not found\n"), buff );
			failed = 1;
			break;
		}
	}

	for (i = 0; i < 4; i++) {
		sprintf( buff, "ABE_B%d", i);
		ABE[i] = part_find_signal( PART, buff );
		if (!ABE[i]) {
			printf( _("signal '%s' not found\n"), buff );
			failed = 1;
			break;
		}
	}

	for (i = 0; i < 32; i++) {
		sprintf( buff, "DATA%d", i);
		DATA[i] = part_find_signal( PART, buff );
		if (!DATA[i]) {
			printf( _("signal '%s' not found\n"), buff );
			failed = 1;
			break;
		}
	}

	AWE = part_find_signal( PART, "AWE_B" );
	if (!AWE) {
		printf( _("signal '%s' not found\n"), "AWE_B" );
		failed = 1;
	}

	AOE = part_find_signal( PART, "AOE_B" );
	if (!AOE) {
		printf( _("signal '%s' not found\n"), "AOE_B" );
		failed = 1;
	}

	SRAS = part_find_signal( PART, "SRAS_B" );
	if (!SRAS) {
		printf( _("signal '%s' not found\n"), "SRAS_B" );
		failed = 1;
	}

	SCAS = part_find_signal( PART, "SCAS_B" );
	if (!SCAS) {
		printf( _("signal '%s' not found\n"), "SCAS_B" );
		failed = 1;
	}

	SWE = part_find_signal( PART, "SWE_B" );
	if (!SWE) {
		printf( _("signal '%s' not found\n"), "SWE_B" );
		failed = 1;
	}

	for (i = 0; i < 4; i++) {
		sprintf( buff, "SMS_B%d", i );
		SMS[i] = part_find_signal( PART, buff );
		if (!SMS[i]) {
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

const bus_driver_t bf561_ezkit_bus = {
	"bf561_ezkit",
	N_("Blackfin BF561 EZ-KIT board bus driver"),
	bf561_ezkit_bus_new,
	generic_bus_free,
	bf561_ezkit_bus_printinfo,
	bf561_ezkit_bus_prepare,
	bf561_ezkit_bus_area,
	bf561_ezkit_bus_read_start,
	bf561_ezkit_bus_read_next,
	bf561_ezkit_bus_read_end,
	bf561_ezkit_bus_read,
	bf561_ezkit_bus_write,
	NULL
};
