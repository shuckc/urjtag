/*
 * $Id$
 *
 * Motorola MPC824x compatible bus driver via BSR
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
 * Written by Marcel Telka <marcel@telka.sk>, 2003.
 *
 * Documentation:
 * [1] Motorola, Inc., "MPC8245 Integrated Processor User's Manual",
 *     MPC8245UM/D, 10/2001, Rev. 1
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
	int boot_nfoe;
	int boot_sdma1;
	uint32_t last_adr;
	signal_t *ar[23];
	signal_t *nrcs0;
	signal_t *nwe;
	signal_t *nfoe;
	signal_t *d[32];
} bus_params_t;

#define	CHAIN		((bus_params_t *) bus->params)->chain
#define	PART		((bus_params_t *) bus->params)->part
#define	boot_nFOE	((bus_params_t *) bus->params)->boot_nfoe
#define	boot_SDMA1	((bus_params_t *) bus->params)->boot_sdma1
#define	LAST_ADR	((bus_params_t *) bus->params)->last_adr
#define	AR		((bus_params_t *) bus->params)->ar
#define	nRCS0		((bus_params_t *) bus->params)->nrcs0
#define	nWE		((bus_params_t *) bus->params)->nwe
#define	nFOE		((bus_params_t *) bus->params)->nfoe
#define	D		((bus_params_t *) bus->params)->d

int BUS_WIDTH = 8;
char REVBITS = 0;
char dbgAddr = 0;
char dbgData = 0;


/**
 * bus->driver->(*new_bus)
 *
 */
static bus_t *
mpc824x_bus_new( chain_t *chain, char *cmd_params[] )
{
	bus_t *bus;
	char buff[10];
	int i;
	int failed = 0;
	part_t *part;
	signal_t *s_nfoe;
	signal_t *s_sdma1;

	char param[16], value[16];

	char dfltWidth = 1;

	dbgAddr = 0;
	dbgData = 0;
	REVBITS = 0;

	for (i=2; cmd_params[i]; i++) {
	  if (strstr( cmd_params[i], "=")) {
	    sscanf( cmd_params[i], "%[^=]%*c%s", param, value );

	    if (!strcmp( "width", param )) {
	      if (!strcmp( "8", value )) {
		BUS_WIDTH = 8;
		dfltWidth = 0;
	      } else if (!strcmp( "32", value )) {
		BUS_WIDTH = 32;
		dfltWidth = 0;
	      }
	      else if (!strcmp( "64", value )) {
		//		BUS_WIDTH = 64;  // Needs to fix, look at setup_data()
		BUS_WIDTH = 32;
		printf(_("   Bus width 64 exists in mpc824x, but not supported by UrJTAG currently\n"));
		dfltWidth = 1;
	      }
	      else {
		printf(_("   Only 8,32 and 64 bus width are supported for Banks 0 and 1\n"));
		return NULL;
	      }
	    }
	  } else {
	    if (!strcmp( "revbits", cmd_params[i]))
	      REVBITS = 1;

	    if (!strcmp( "help", cmd_params[i])) {
	      printf(_("Usage: initbus mpc824x [width=WIDTH] [revbits] [dbgAddr] [dbgData]\n\n"
		       "   WIDTH      data bus width - 8, 32, 64 (default 8)\n"
		       "   revbits    reverse bits in data bus (default - no)\n"
		       "   dbgAddr    display address bus state (default - no)\n"
		       "   dbgData    display data bus state (default - no)\n"));
	      return NULL;
	    }

	    if (!strcmp( "dbgAddr", cmd_params[i]))
	      dbgAddr = 1;


	    if (!strcmp( "dbgData", cmd_params[i]))
	      dbgData = 1;


	  }

	}
	if (dfltWidth)
	  printf(_("   Using default bus width %d\n"), BUS_WIDTH);

	//	REVBITS = 0;



	if (!chain || !chain->parts || chain->parts->len <= chain->active_part || chain->active_part < 0)
		return NULL;

	bus = calloc( 1, sizeof (bus_t) );
	if (!bus)
		return NULL;

	bus->driver = &mpc824x_bus;
	bus->params = calloc( 1, sizeof (bus_params_t) );
	if (!bus->params) {
		free( bus );
		return NULL;
	}

	CHAIN = chain;
	PART = part = chain->parts->parts[chain->active_part];

	s_nfoe = part_find_signal( part, "nFOE" );
	s_sdma1 = part_find_signal( part, "SDMA1" );
	part_set_signal( part, s_nfoe, 0, 0 );
	part_set_signal( part, s_sdma1, 0, 0 );

	part_set_instruction( part, "SAMPLE/PRELOAD" );
	chain_shift_instructions( chain );
	chain_shift_data_registers( chain, 0 );
	part_set_instruction( part, "EXTEST" );
	chain_shift_instructions( chain );
	chain_shift_data_registers( chain, 1 );

	boot_nFOE = part_get_signal( part, s_nfoe );
	boot_SDMA1 = part_get_signal( part, s_sdma1 );


	for (i = 0; i <= 10; i++) {
		sprintf( buff, "SDMA%d", i );
		AR[i] = part_find_signal( part, buff );
		if (!AR[i]) {
			printf( _("signal '%s' not found\n"), buff );
			failed = 1;
			break;
		}
	}
	AR[11] = part_find_signal( part, "SDBA0" );
	if (!AR[11]) {
		printf( _("signal '%s' not found\n"), "SDBA0" );
		failed = 1;
	}
	for (i = 0; i < 8; i++) {
		sprintf( buff, "PAR%d", i );
		AR[19 - i] = part_find_signal( part, buff );
		if (!AR[19 - i]) {
			printf( _("signal '%s' not found\n"), buff );
			failed = 1;
			break;
		}
	}
	AR[20] = part_find_signal( part, "SDBA1" );
	if (!AR[20]) {
		printf( _("signal '%s' not found\n"), "SDBA1" );
		failed = 1;
	}
	AR[21] = part_find_signal( part, "SDMA11" );
	if (!AR[21]) {
	  printf( _("signal '%s' not found\n"), "SDMA11" );
	  failed = 1;
	}
	AR[22] = part_find_signal( part, "SDMA12" );
	if (!AR[22]) {
	  printf( _("signal '%s' not found\n"), "SDMA12" );
	  failed = 1;
	}


	nRCS0 = part_find_signal( part, "nRCS0" );
	if (!nRCS0) {
		printf( _("signal '%s' not found\n"), "nRCS0" );
		failed = 1;
	}
	nWE = part_find_signal( part, "nWE" );
	if (!nWE) {
		printf( _("signal '%s' not found\n"), "nWE" );
		failed = 1;
	}
	nFOE = part_find_signal( part, "nFOE" );
	if (!nWE) {
		printf( _("signal '%s' not found\n"), "nFOE" );
		failed = 1;
	}

	/*
	    Freescale MPC824x uses inversed bit order ([1], p. 2-18):
	    msb is MDH[0] while lsb is MDH[31]
	    Flash chips usually use another bit orded:
	    msb is D[31] and lsb is D[0]

	    This should be rewired in the PCB (MDH[0] - D[31], ..., MDH[31] - D[0]).
	    Otherwise you will have to use "revbits" UrJTAG parameter and
	    binary files with reversed bit order.
	*/

	for (i = 0; i < 32; i++) { /* Needs to be fixed for 64-bit bus width */
	  sprintf( buff, "MDH%d",  31 - i );
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
mpc824x_bus_printinfo( bus_t *bus )
{
	int i;

	for (i = 0; i < CHAIN->parts->len; i++)
		if (PART == CHAIN->parts->parts[i])
			break;
	printf( _("Motorola MPC824x compatible bus driver via BSR (JTAG part No. %d)\n"), i );
}

/**
 * bus->driver->(*prepare)
 *
 */
static void
mpc824x_bus_prepare( bus_t *bus )
{
	part_set_instruction( PART, "EXTEST" );
	chain_shift_instructions( CHAIN );
}

/**
 * bus->driver->(*area)
 *
 */
static int
mpc824x_bus_area( bus_t *bus, uint32_t adr, bus_area_t *area )
{

	if (adr < UINT32_C(0xFF000000)) {
		area->description = NULL;
		area->start = UINT32_C(0x00000000);
		area->length = UINT64_C(0xFF000000);
		area->width = 0;

		return 0;
	}

	if (adr < UINT32_C(0xFF800000)) {
		area->description = N_("Base ROM Interface (Bank 1)");
		area->start = UINT32_C(0xFF000000);
		area->length = UINT64_C(0x00800000);
		area->width = 0;

		return 0;
	}

	if (boot_SDMA1 == 0) {
		area->description = N_("Base ROM Interface (Bank 0)");
		area->start = UINT32_C(0xFF800000);
		area->length = UINT64_C(0x00800000);
		area->width = BUS_WIDTH;

		return 0;
	}

	/* extended addresing mode is disabled (SDMA1 is 1) */
	if (adr < UINT32_C(0xFFC00000)) {
		area->description = NULL;
		area->start = UINT32_C(0xFF800000);
		area->length = UINT64_C(0x00400000);
		area->width = BUS_WIDTH;

		return 0;
	}

	area->description = N_("Base ROM Interface (Bank 0)");
	area->start = UINT32_C(0xFFC00000);
	area->length = UINT64_C(0x00400000);
	area->width = BUS_WIDTH;

	return 0;
}

static void
setup_address( bus_t *bus, uint32_t a )
{
	int i;
	part_t *p = PART;

	switch (BUS_WIDTH) {
	case 8:/* 8-bit data bus */
	  for (i = 0; i < 23; i++)
	    part_set_signal( p, AR[i], 1, (a >> i) & 1 );
	case 32:/* 32-bit data bus */
	  for (i = 0; i < 21; i++)
	    part_set_signal( p, AR[i], 1, (a >> (i+2)) & 1 );
	  break;
	case 64:
	  for (i = 0; i < 20; i++)
	    part_set_signal( p, AR[i], 1, (a >> (i+3)) & 1 );
	}

	/* Just for debugging */
	if (dbgAddr) {
	  int j, k;
	  switch (BUS_WIDTH) {
	  case 8:  k = 23; break;
	  case 32: k = 21; break;
	  case 64: k = 20;
	  }

	  printf(_("Addr    [%2d:0]: %06X   "), k, a);
	  for (i=0; i<3; i++) {
	    for (j=0; j<8; j++)
	      if ((i*8+j) >= (23 - k))
		printf("%1d", (a >> (23 - (i*8+j)) ) & 1);
	      else
		printf(" ");
	    printf(" ");
	  };
	  printf("\n");
	}

}

static void
set_data_in( bus_t *bus, uint32_t adr )
{
	int i;
	part_t *p = PART;
	bus_area_t area;

	mpc824x_bus_area( bus, adr, &area );
	if (area.width > 64)
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

	mpc824x_bus_area( bus, adr, &area );
	if (area.width > 64)
	  return;

	for (i = 0; i < area.width; i++)
		part_set_signal( p, D[i], 1, (d >> ((REVBITS==1) ? BUS_WIDTH-1-i : i)) & 1 );

	/* Just for debugging */
	if (dbgData) {
	  printf(_("Data WR [%d:0]: %08X   "), area.width-1, d);
	  int j;
	  int bytes =0;
	  if (BUS_WIDTH==8) bytes = 1;
	  else if (BUS_WIDTH==32) bytes = 4;
	  else if (BUS_WIDTH==64) bytes = 4; /* Needs to be fixed - d is 32-bit long, so no 64 bit mode is possible */

	  for (i=0; i<bytes; i++) {
	    for (j=0; j<8; j++)
	      if (REVBITS)
		printf("%1d", (d >> (BUS_WIDTH-1 - (i*8+j)) ) & 1);
	      else
		printf("%1d", (d >> (              (i*8+j)) ) & 1);
	    printf(" ");
	  };
	  printf("\n");
	}

}

static uint32_t
get_data( bus_t *bus, uint32_t adr )
{
	bus_area_t area;
	int i;
	uint32_t d = 0;
	part_t *p = PART;

	mpc824x_bus_area( bus, adr, &area );
	if (area.width > 64)
		return 0;

	for (i = 0; i < area.width; i++)
		d |= (uint32_t) (part_get_signal( p, D[i] ) << ((REVBITS==1) ? BUS_WIDTH-1-i : i));

	/* Just for debugging */
	if (dbgData) {
	  printf(_("Data RD [%d:0]: %08X   "), area.width-1, d);
	  int j;
	  int bytes = 0;
	  if (BUS_WIDTH==8) bytes = 1;
	  else if (BUS_WIDTH==32) bytes = 4;
	  else if (BUS_WIDTH==64) bytes = 4; /* Needs to be fixed - d is 32-bit long, so no 64 bit mode is possible */

	  for (i=0; i<bytes; i++) {
	    for (j=0; j<8; j++)
	      if (REVBITS)
		printf("%1d", (d >> (BUS_WIDTH-1 - (i*8+j)) ) & 1);
	      else
		printf("%1d", (d >> (              (i*8+j)) ) & 1);
	    printf(" ");
	  };
	  printf("\n");
	}

	return d;
}

/**
 * bus->driver->(*read_start)
 *
 */
static void
mpc824x_bus_read_start( bus_t *bus, uint32_t adr )
{
	part_t *p = PART;

	LAST_ADR = adr;

	/* see Figure 6-45 in [1] */
	part_set_signal( p, nRCS0, 1, 0 );
	part_set_signal( p, nWE, 1, 1 );
	part_set_signal( p, nFOE, 1, 0 );

	setup_address( bus, adr );
	set_data_in( bus, adr );

	chain_shift_data_registers( CHAIN, 0 );
}

/**
 * bus->driver->(*read_next)
 *
 */
static uint32_t
mpc824x_bus_read_next( bus_t *bus, uint32_t adr )
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
mpc824x_bus_read_end( bus_t *bus )
{
	part_t *p = PART;

	part_set_signal( p, nRCS0, 1, 1 );
	part_set_signal( p, nFOE, 1, 1 );

	chain_shift_data_registers( CHAIN, 1 );

	return get_data( bus, LAST_ADR );
}

/**
 * bus->driver->(*read)
 *
 */
static uint32_t
mpc824x_bus_read( bus_t *bus, uint32_t adr )
{
	mpc824x_bus_read_start( bus, adr );
	return mpc824x_bus_read_end( bus );
}

/**
 * bus->driver->(*write)
 *
 */
static void
mpc824x_bus_write( bus_t *bus, uint32_t adr, uint32_t data )
{
	part_t *p = PART;

	LAST_ADR = adr;


	/* see Figure 6-47 in [1] */
	part_set_signal( p, nRCS0, 1, 0 );
	part_set_signal( p, nWE, 1, 1 );
	part_set_signal( p, nFOE, 1, 1 );

	setup_address( bus, adr );

	setup_data( bus, adr, data );

	chain_shift_data_registers( CHAIN, 0 );

	part_set_signal( p, nWE, 1, 0 );
	chain_shift_data_registers( CHAIN, 0 );
	part_set_signal( p, nWE, 1, 1 );
	part_set_signal( p, nRCS0, 1, 1 );
	chain_shift_data_registers( CHAIN, 0 );



}

const bus_driver_t mpc824x_bus = {
	"mpc824x",
	N_("Motorola MPC824x compatible bus driver via BSR"),
	mpc824x_bus_new,
	generic_bus_free,
	mpc824x_bus_printinfo,
	mpc824x_bus_prepare,
	mpc824x_bus_area,
	mpc824x_bus_read_start,
	mpc824x_bus_read_next,
	mpc824x_bus_read_end,
	mpc824x_bus_read,
	mpc824x_bus_write,
	NULL
};
