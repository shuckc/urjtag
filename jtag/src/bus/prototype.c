/*
 * $Id$
 *
 * Copyright (C) 2002, 2003 ETC s.r.o.
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
 * Written by Marcel Telka <marcel@telka.sk>, 2002, 2003.
 * Modified by Wojtek Kaniewski <wojtekka@toxygen.net>, 2004.
 * Modified from ppc405ep.c by Detrick Martin <jtag@detrickmartin.net>, 2008.
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
	signal_t *a[32];
	signal_t *d[32];
	signal_t *cs;
	signal_t *we;
	signal_t *oe;
	int alsbi, amsbi, ai, aw, dlsbi, dmsbi, di, dw, csa, wea, oea;
	int ashift;
} bus_params_t;

#define	CHAIN	((bus_params_t *) bus->params)->chain
#define	PART	((bus_params_t *) bus->params)->part
#define	A	((bus_params_t *) bus->params)->a
#define	D	((bus_params_t *) bus->params)->d
#define	CS	((bus_params_t *) bus->params)->cs
#define	WE	((bus_params_t *) bus->params)->we
#define	OE	((bus_params_t *) bus->params)->oe

#define	ALSBI	((bus_params_t *) bus->params)->alsbi
#define	AMSBI	((bus_params_t *) bus->params)->amsbi
#define	AI	((bus_params_t *) bus->params)->ai
#define	AW	((bus_params_t *) bus->params)->aw
#define	DLSBI	((bus_params_t *) bus->params)->dlsbi
#define	DMSBI	((bus_params_t *) bus->params)->dmsbi
#define	DI	((bus_params_t *) bus->params)->di
#define	DW	((bus_params_t *) bus->params)->dw
#define	CSA	((bus_params_t *) bus->params)->csa
#define	WEA	((bus_params_t *) bus->params)->wea
#define	OEA	((bus_params_t *) bus->params)->oea

#define ASHIFT ((bus_params_t *) bus->params)->ashift


static void
setup_address( bus_t *bus, uint32_t a )
{
	int i, j;
	part_t *p = PART;

	a >>= ASHIFT;

	for ( i = 0, j = ALSBI; i < AW; i++, j += AI )
		part_set_signal( p, A[j], 1, (a >> i) & 1 );
}

static int
prototype_bus_area( bus_t *bus, uint32_t adr, bus_area_t *area )
{
	area->description = NULL;
	area->start = UINT32_C(0x00000000);
	area->length = UINT64_C(0x100000000);
	area->width = DW;

	return 0;
}

static void
set_data_in( bus_t *bus )
{
	int i, j;
	part_t *p = PART;
	bus_area_t area;

	prototype_bus_area( bus, 0, &area );

	for ( i = 0, j = DLSBI; i < DW; i++, j += DI )
		part_set_signal( p, D[j], 0, 0 );
}

static void
setup_data( bus_t *bus, uint32_t d )
{
	int i, j;
	part_t *p = PART;
	bus_area_t area;

	prototype_bus_area( bus, 0, &area );

	for ( i = 0, j = DLSBI; i < DW; i++, j += DI )
		part_set_signal( p, D[j], 1, (d >> i) & 1 );
}

static void
prototype_bus_printinfo( bus_t *bus )
{
	int i;

	for ( i = 0; i < CHAIN->parts->len; i++ )
		if ( PART == CHAIN->parts->parts[i] )
			break;
	printf( _("Configurable prototype bus driver via BSR (JTAG part No. %d)\n"), i );
}

static void
prototype_bus_prepare( bus_t *bus )
{
	part_set_instruction( PART, "EXTEST" );
	chain_shift_instructions( CHAIN );
}

static void
prototype_bus_read_start( bus_t *bus, uint32_t adr )
{
	part_t *p = PART;
	chain_t *chain = CHAIN;

	part_set_signal( p, CS, 1, CSA );
	part_set_signal( p, WE, 1, WEA ? 0 : 1 );
	part_set_signal( p, OE, 1, OEA );

	setup_address( bus, adr );
	set_data_in( bus );

	chain_shift_data_registers( chain, 0 );
}

static uint32_t
prototype_bus_read_next( bus_t *bus, uint32_t adr )
{
	part_t *p = PART;
	chain_t *chain = CHAIN;
	int i, j;
	uint32_t d = 0;
	bus_area_t area;

	prototype_bus_area( bus, adr, &area );

	setup_address( bus, adr );
	chain_shift_data_registers( chain, 1 );

	for ( i = 0, j = DLSBI; i < DW; i++, j += DI )
		d |= (uint32_t) (part_get_signal( p, D[j] ) << i);

	return d;
}

static uint32_t
prototype_bus_read_end( bus_t *bus )
{
	part_t *p = PART;
	chain_t *chain = CHAIN;
	int i, j;
	uint32_t d = 0;
	bus_area_t area;

	prototype_bus_area( bus, 0, &area );

	part_set_signal( p, CS, 1, CSA ? 0 : 1 );
	part_set_signal( p, OE, 1, OEA ? 0 : 1 );
	chain_shift_data_registers( chain, 1 );

	for ( i = 0, j = DLSBI; i < DW; i++, j += DI )
		d |= (uint32_t) (part_get_signal( p, D[j] ) << i);

	return d;
}

static uint32_t
prototype_bus_read( bus_t *bus, uint32_t adr )
{
	int res;
	
	prototype_bus_read_start( bus, adr );
	res = prototype_bus_read_end( bus );
	
	return res;
}

static void
prototype_bus_write( bus_t *bus, uint32_t adr, uint32_t data )
{
	part_t *p = PART;
	chain_t *chain = CHAIN;

	part_set_signal( p, CS, 1, CSA );
	part_set_signal( p, WE, 1, WEA ? 0 : 1 );
	part_set_signal( p, OE, 1, OEA ? 0 : 1 );

	setup_address( bus, adr );
	setup_data( bus, data );

	chain_shift_data_registers( chain, 0 );

	part_set_signal( p, WE, 1, WEA );
	chain_shift_data_registers( chain, 0 );
	part_set_signal( p, WE, 1, WEA ? 0 : 1 );
	part_set_signal( p, CS, 1, CSA ? 0 : 1 );
	chain_shift_data_registers( chain, 0 );
}

static void
prototype_bus_free( bus_t *bus )
{
	free( bus->params );
	free( bus );
}

static void
prototype_bus_signal_parse( char *str, char *fmt, int *inst )
{
	char pre[16], suf[16];

	switch( sscanf( str, "%[^0-9]%d%s", pre, inst, suf ) ) {
		case 1:
			strcpy( fmt, str );
			break;
		case 2:
			sprintf( fmt, "%s%s", pre, "%d" );
			break;
		case 3:
			sprintf( fmt, "%s%s%s", pre, "%d", suf );
	}
}

static bus_t *
prototype_bus_new( char *cmd_params[] )
{
	bus_t *bus;
	signal_t *sig;
	char buff[16], fmt[16], afmt[16], dfmt[16], param[16], value[16];
	int i, j, inst, max, min;
	int failed = 0;
	int ashift = -1;

	if (!chain || !chain->parts || (chain->parts->len <= chain->active_part) || (chain->active_part < 0))
		return NULL;

	bus = malloc( sizeof (bus_t) );
	if (!bus)
		return NULL;

	bus->driver = &prototype_bus;
	bus->params = malloc( sizeof (bus_params_t) );
	if (!bus->params) {
		free( bus );
		return NULL;
	}

	CHAIN = chain;
	PART = chain->parts->parts[chain->active_part];

	CS = OE = WE = NULL;
	ALSBI = AMSBI = DLSBI = DMSBI = -1;
	for ( i = 2; cmd_params[i]; i++ ) {
		if (!strstr( cmd_params[i], "=")) continue;
		sscanf( cmd_params[i], "%[^=]%*c%s", param, value );

		if (!strcmp( "amode", param )) {
			if (!strcmp( "x8", value ))
				ashift = 0;
			else if (!strcmp( "x16", value ))
				ashift = 1;
			else if (!strcmp( "x32", value ))
				ashift = 2;
			else if (strcmp( "auto", value ))
				printf( _("value %s not defined for parameter %s\n"), value, param );
			continue;
		}

		prototype_bus_signal_parse( value, fmt, &inst );
		if( inst > 31 ) continue;

		sig = part_find_signal( PART, value );
		if (!sig) {
			printf( _("signal '%s' is not found\n"), value );
			failed = 1;
		} else if (!strcmp( "alsb", param )) {
			ALSBI = inst;
			A[inst] = sig;
			strcpy( afmt, fmt );
		} else if (!strcmp( "amsb", param )) {
			AMSBI = inst;
			A[inst] = sig;
			strcpy( afmt, fmt );
		} else if (!strcmp( "dlsb", param )) {
			DLSBI = inst;
			D[inst] = sig;
			strcpy( dfmt, fmt );
		} else if (!strcmp( "dmsb", param )) {
			DMSBI = inst;
			D[inst] = sig;
			strcpy( dfmt, fmt );
		} else if (!(strcmp( "cs", param ) && strcmp( "ncs", param ))) {
			CS = sig;
			CSA = ( *param == 'n' ) ? 0 : 1;
		} else if (!(strcmp( "oe", param ) && strcmp( "noe", param ))) {
			OE = sig;
			OEA = ( *param == 'n' ) ? 0 : 1;
		} else if (!(strcmp( "we", param ) && strcmp( "nwe", param ))) {
			WE = sig;
			WEA = ( *param == 'n' ? 0 : 1 );
		} else {
			printf( _("parameter %s is unknown\n"), param );
			failed = 1;
		}
	}

	if ( ALSBI >= 0 || AMSBI >= 0 ) {
		if ( ALSBI == -1 || AMSBI == -1 ) {
			for ( min = 0; min <= 31; min++ ) {
				sprintf( buff, afmt, min );
				A[min] = part_find_signal( PART, buff );
				if(A[min]) break;
			}
			for ( max = 31; max >= 0; max-- ) {
				sprintf( buff, afmt, max );
				A[max] = part_find_signal( PART, buff );
				if(A[max]) break;
			}
			if ( ALSBI == -1 )
				ALSBI = ( max-AMSBI < AMSBI-min ) ? min : max;
			else
				AMSBI = ( max-ALSBI < ALSBI-min ) ? min : max;
		}
		AI = ( AMSBI > ALSBI ? 1 : -1 );
		AW = ( AMSBI > ALSBI ? AMSBI - ALSBI : ALSBI - AMSBI ) + 1;
		for (i = 0, j = ALSBI; i < AW; i++, j += AI ) {
			sprintf( buff, afmt, j );
			A[j] = part_find_signal( PART, buff );
		}	
	} else {
		printf( _("parameters alsb=<signal> and/or amsb=<signal> are not defined\n") );
		failed = 1;
	}

	if ( DLSBI >= 0 || DMSBI >= 0 ) {
		if ( DLSBI == -1 || DMSBI == -1 ) {
			for ( min = 0; min <= 31; min++ ) {
				sprintf( buff, dfmt, min );
				D[min] = part_find_signal( PART, buff );
				if(D[min]) break;
			}
			for ( max = 31; max >= 0; max-- ) {
				sprintf( buff, dfmt, max );
				D[max] = part_find_signal( PART, buff );
				if(D[max]) break;
			}
			if ( DLSBI == -1 )
				DLSBI = ( max - DMSBI < DMSBI - min ) ? min : max;
			else
				DMSBI = ( max - DLSBI < DLSBI - min ) ? min : max;
		}
		DI = ( DMSBI > DLSBI ? 1 : -1 );
		DW = ( DMSBI > DLSBI ? DMSBI - DLSBI : DLSBI - DMSBI ) + 1;
		for ( i = 0, j = DLSBI; i < DW; i++, j += DI ) {
			sprintf( buff, dfmt, j );
			D[j] = part_find_signal( PART, buff );
		}	

		/* bus drivers are called with a byte address
		   this address needs to be adjusted by setup_address() to the memory data width */
		if (ashift < 0) {
			int nbytes;

			/* parameter 'amode' wasn't specified, derive the address shift from the
			   data bus width */
			nbytes = DW / 8;
			if ( DW % 8 > 0 )
				nbytes++;

			ashift = 0;
			while (nbytes != 1) {
				nbytes >>= 1;
				ashift++;
			}
			ASHIFT = ashift;
		}

	} else {
		printf( _("parameters dlsb=<signal> and/or dmsb=<signal> are not defined\n") );
		failed = 1;
	}

	if (!CS) {
		printf( _("parameter cs=<signal> or ncs=<signal> is not defined\n") );
		failed = 1;
	}

	if (!OE) {
		printf( _("parameter oe=<signal> or noe=<signal> is not defined\n") );
		failed = 1;
	}

	if (!WE) {
		printf( _("parameter we=<signal> or nwe=<signal> is not defined\n") );
		failed = 1;
	}

	if (failed) {
		free( bus->params );
		free( bus );
		return NULL;
	}

	return bus;
}


const bus_driver_t prototype_bus = {
	"prototype",
	N_("Configurable prototype bus driver via BSR, requires parameters:\n"
	"           amsb=<addr MSB> alsb=<addr LSB> dmsb=<data MSB> dlsb=<data LSB>\n"
	"           ncs=<CS#>|cs=<CS> noe=<OE#>|oe=<OE> nwe=<WE#>|we=<WE> [amode=auto|x8|x16|x32]"),
	prototype_bus_new,
	prototype_bus_free,
	prototype_bus_printinfo,
	prototype_bus_prepare,
	prototype_bus_area,
	prototype_bus_read_start,
	prototype_bus_read_next,
	prototype_bus_read_end,
	prototype_bus_read,
	prototype_bus_write
};

