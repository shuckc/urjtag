/*
 * $Id$
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
 * Written by Marko Roessler <marko.roessler@indakom.de>, 2004
 *
 * based on sa1110.c
 * Written by Marcel Telka <marcel@telka.sk>, 2002, 2003.
 *
 * Documentation:
 *
 * [1] Sharp Microelectronics, "LH7A400 Universal SOC Preliminary 
 *     Users's Guide", May 2003, Reference No. SMA02010
 *
 *
 * Notes:
 *        - this bus driver ONLY works for the asynchronous boot mode!
 *        - use only to access flash devices
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

#define ADR_NUM 24
#define D_NUM   32
#define nCS_NUM 4
#define WIDTH_NUM 2

typedef struct {
	chain_t *chain;
	part_t *part;
	signal_t *a[ADR_NUM];
	signal_t *d[D_NUM];
	signal_t *ncs[nCS_NUM];
	signal_t *nwe;
	signal_t *noe;
	signal_t *width[WIDTH_NUM];
} bus_params_t;

#define	CHAIN	((bus_params_t *) bus->params)->chain
#define	PART	((bus_params_t *) bus->params)->part
#define	A	((bus_params_t *) bus->params)->a
#define	D	((bus_params_t *) bus->params)->d
#define	nCS	((bus_params_t *) bus->params)->ncs
#define	nWE	((bus_params_t *) bus->params)->nwe
#define	nOE	((bus_params_t *) bus->params)->noe
#define WIDTH   ((bus_params_t *) bus->params)->width

static void
setup_address( bus_t *bus, uint32_t a )
{
	int i;
	part_t *p = PART;

	for (i = 0; i < ADR_NUM; i++)
		part_set_signal( p, A[i], 1, (a >> i) & 1 );
}

static int lh7a400_bus_area( bus_t *bus, uint32_t adr, bus_area_t *area );

static void
set_data_in( bus_t *bus )
{
	int i;
	part_t *p = PART;
	bus_area_t area;

	lh7a400_bus_area( bus, 0, &area );

	for (i = 0; i < area.width; i++)
		part_set_signal( p, D[i], 0, 0 );
	
}

static void
setup_data( bus_t *bus, uint32_t d )
{
	int i;
	part_t *p = PART;
	bus_area_t area;

	lh7a400_bus_area( bus, 0, &area );

	for (i = 0; i < area.width; i++)
		part_set_signal( p, D[i], 1, (d >> i) & 1 );
}

static void
lh7a400_bus_printinfo( bus_t *bus )
{
	int i;

	for (i = 0; i < CHAIN->parts->len; i++)
		if (PART == CHAIN->parts->parts[i])
			break;
	printf( _("Sharp LH7A400 compatible bus driver via BSR (JTAG part No. %d)\n"), i );
}

static void
lh7a400_bus_prepare( bus_t *bus )
{       
	part_set_instruction( PART, "EXTEST" );
	chain_shift_instructions( CHAIN );
}

static void
lh7a400_bus_read_start( bus_t *bus, uint32_t adr )
{
	/* see Figure 3-3 in [1] */
	part_t *p = PART;
	chain_t *chain = CHAIN;

	part_set_signal( p, nCS[0], 1, (adr >> 27) != 0 );
	part_set_signal( p, nWE, 1, 1 );
	part_set_signal( p, nOE, 1, 0 );

	setup_address( bus, adr );
	set_data_in( bus );

	chain_shift_data_registers( chain, 0 );
}

static uint32_t
lh7a400_bus_read_next( bus_t *bus, uint32_t adr )
{
	/* see Figure 3-3 in [1] */
	part_t *p = PART;
	chain_t *chain = CHAIN;
	int i;
	uint32_t d = 0;
	bus_area_t area;

	lh7a400_bus_area( bus, adr, &area );

	setup_address( bus, adr );
	chain_shift_data_registers( chain, 1 );

	for (i = 0; i < area.width; i++)
		d |= (uint32_t) (part_get_signal( p, D[i] ) << i);

	return d;
}

static uint32_t 
lh7a400_bus_read_end( bus_t *bus )
{
	/* see Figure 3-3 in [1] */
	part_t *p = PART;
	chain_t *chain = CHAIN;
	int i;
	uint32_t d = 0;
	bus_area_t area;

	lh7a400_bus_area( bus, 0, &area );

	part_set_signal( p, nCS[0], 1, 1 );
	part_set_signal( p, nOE, 1, 1 );

	chain_shift_data_registers( chain, 1 );

	for (i = 0; i < area.width; i++)
		d |= (uint32_t) (part_get_signal( p, D[i] ) << i);

	return d;
}

static uint32_t
lh7a400_bus_read( bus_t *bus, uint32_t adr )
{
	lh7a400_bus_read_start( bus, adr );
	return lh7a400_bus_read_end( bus );
}

static void
lh7a400_bus_write( bus_t *bus, uint32_t adr, uint32_t data )
{
	/* see Figure 3-3 in [1] */
	part_t *p = PART;
	chain_t *chain = CHAIN;

	part_set_signal( p, nCS[0], 1, (adr >> 27) != 0 );
	part_set_signal( p, nWE, 1, 1 );
	part_set_signal( p, nOE, 1, 1 );

	setup_address( bus, adr );
	setup_data( bus, data );

	chain_shift_data_registers( chain, 0 );

	part_set_signal( p, nWE, 1, 0 );
	chain_shift_data_registers( chain, 0 );

	part_set_signal( p, nWE, 1, 1 );
	part_set_signal( p, nCS[0], 1, 1 );
	chain_shift_data_registers( chain, 0 );
}

static int
lh7a400_bus_area ( bus_t *bus, uint32_t adr, bus_area_t *area )
{
  unsigned int width;

	area->description = NULL;
	area->start = UINT32_C(0x00000000);
	area->length = UINT64_C(0x10000000);

	/* we determine the size of the flash that was booted from [1] table 3.1 */
	width = part_get_signal( PART, part_find_signal( PART, "WIDTH0" ) );	
	width |= part_get_signal( PART, part_find_signal( PART, "WIDTH1" ) ) << 1;

	if (width < 0)
	  return -1;

	switch (width) {
	case 0:
	  area->width = 8;
	  break;
	case 1:
	  area->width = 16;
	  break;
	case 2:
	case 3:
	  area->width = 32;
	}

	return 0;
}

static void
lh7a400_bus_free( bus_t *bus )
{
	free( bus->params );
	free( bus );
}

static bus_t *
lh7a400_bus_new( chain_t *chain, char *cmd_params[] )
{
	bus_t *bus;
	char buff[10];
	int i;
	int failed = 0;

	if (!chain || !chain->parts || chain->parts->len <= chain->active_part || chain->active_part < 0)
		return NULL;

	bus = calloc( 1, sizeof (bus_t) );
	if (!bus)
		return NULL;

	bus->driver = &lh7a400_bus;
	bus->params = calloc( 1, sizeof (bus_params_t) );
	if (!bus->params) {
		free( bus );
		return NULL;
	}

	CHAIN = chain;
	PART = chain->parts->parts[chain->active_part];

	for (i = 0; i < ADR_NUM; i++) {
		sprintf( buff, "A%d", i );
		A[i] = part_find_signal( PART, buff );
		if (!A[i]) {
			printf( _("signal '%s' not found\n"), buff );
			failed = 1;
			break;
		}
	}
	for (i = 0; i < D_NUM; i++) {
		sprintf( buff, "D%d", i );
		D[i] = part_find_signal( PART, buff );
		if (!D[i]) {
			printf( _("signal '%s' not found\n"), buff );
			failed = 1;
			break;
		}
	}
	for (i = 0; i < nCS_NUM; i++) {
		sprintf( buff, "nCS%d", i );
		nCS[i] = part_find_signal( PART, buff );
		if (!nCS[i]) {
			printf( _("signal '%s' not found\n"), buff );
			failed = 1;
			break;
		}
	}
	for (i = 0; i < WIDTH_NUM; i++) {
		sprintf( buff, "WIDTH%d", i );
		WIDTH[i] = part_find_signal( PART, buff );
		if (!WIDTH[i]) {
			printf( _("signal '%s' not found\n"), buff );
			failed = 1;
			break;
		}
	}
	nWE = part_find_signal( PART, "nWE0" );
	if (!nWE) {
		printf( _("signal '%s' not found\n"), "nWE" );
		failed = 1;
	}
	nOE = part_find_signal( PART, "nOE" );
	if (!nOE) {
		printf( _("signal '%s' not found\n"), "nOE" );
		failed = 1;
	}

	if (failed) {
		free( bus->params );
		free( bus );
		return NULL;
	}

	return bus;
}

const bus_driver_t lh7a400_bus = {
	"lh7a400",
	N_("Sharp LH7A400 compatible bus driver via BSR (flash access only!)"),
	lh7a400_bus_new,
	lh7a400_bus_free,
	lh7a400_bus_printinfo,
	lh7a400_bus_prepare,
	lh7a400_bus_area,
	lh7a400_bus_read_start,
	lh7a400_bus_read_next,
	lh7a400_bus_read_end,
	lh7a400_bus_read,
	lh7a400_bus_write,
    NULL
};
