/*
 * $Id$
 *
 * Altera UP3 Education Kit bus driver via BSR
 * Copyright (C) 2005 Kent Palmkvist
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
 * Written by Kent Palmkvist <kentp@isy.liu.se>, 2005.
 *
 * Documentation:
 * [1] System Level Solutions Inc., "UP3 Education Kit, Reference Manual",
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

int databusio[16] = {94,96,98,100,102,104,106,113,95,97,99,101,103,105,107,114};
int addrbusio[20] = {93,88,87,86,85,84,83,63,64,65,66,67,68,74,75,76,77,82,81,78};

typedef struct {
	uint32_t last_adr;
	signal_t *ad[20];
	signal_t *dq[16];
        signal_t *nsdce;
        signal_t *sdclk;
        signal_t *noe;
        signal_t *nsrce;
        signal_t *nflce;
        signal_t *nflbyte;
        signal_t *nflby;
        signal_t *nwe;
        signal_t *lcde;
        signal_t *lcdrs;
        signal_t *lcdrw;
} bus_params_t;

#define	LAST_ADR	((bus_params_t *) bus->params)->last_adr
#define	AD		((bus_params_t *) bus->params)->ad
#define DQ              ((bus_params_t *) bus->params)->dq
#define	nSDce		((bus_params_t *) bus->params)->nsdce
#define	nOE		((bus_params_t *) bus->params)->noe
#define nSRce           ((bus_params_t *) bus->params)->nsrce
#define	nFLce		((bus_params_t *) bus->params)->nflce
#define	nFLbyte		((bus_params_t *) bus->params)->nflbyte
#define nFLby           ((bus_params_t *) bus->params)->nflby
#define nWE             ((bus_params_t *) bus->params)->nwe
#define SDclk           ((bus_params_t *) bus->params)->sdclk
#define LCDe            ((bus_params_t *) bus->params)->lcde
#define LCDrs           ((bus_params_t *) bus->params)->lcdrs
#define LCDrw           ((bus_params_t *) bus->params)->lcdrw

/* All addresses and length are in Bytes */
/* Assume 8 bit flash data bus */
#define FLASHSTART      UINT32_C(0x0000000)
#define FLASHSIZE       UINT64_C(0x0200000)  /* Number of bytes */
/* Assume 16 bit SRAM data bus */
#define SRAMSTART       0x0200000
#define SRAMSIZE        0x0020000
#define LCDSTART        0x0300000
#define LCDSIZE         0x0100000

/**
 * bus->driver->(*new_bus)
 *
 */
static bus_t *
slsup3_bus_new( chain_t *chain, char *cmd_params[] )
{
	bus_t *bus;
	part_t *part;
	char buff[10];
	int i;
	int failed = 0;

	bus = calloc( 1, sizeof (bus_t) );
	if (!bus)
		return NULL;

	bus->driver = &slsup3_bus;
	bus->params = calloc( 1, sizeof (bus_params_t) );
	if (!bus->params) {
		free( bus );
		return NULL;
	}

	CHAIN = chain;
	PART = part = chain->parts->parts[chain->active_part];

	for(i = 0; i < 20 ; i++) {
		sprintf( buff, "IO%d", addrbusio[i] );
		failed |= generic_bus_attach_sig( part, &(AD[i]), buff );
	}

	for(i = 0; i < 16 ; i++) {
		sprintf( buff, "IO%d", databusio[i] );
		failed |= generic_bus_attach_sig( part, &(DQ[i]), buff );
	}

	failed |= generic_bus_attach_sig( part, &(nOE),     "IO118" );

	failed |= generic_bus_attach_sig( part, &(nSRce),   "IO116" );

	failed |= generic_bus_attach_sig( part, &(nSDce),   "IO119" );

	failed |= generic_bus_attach_sig( part, &(nFLce),   "IO117" );

	failed |= generic_bus_attach_sig( part, &(nFLbyte), "IO115" );

	failed |= generic_bus_attach_sig( part, &(nFLby),   "IO80"  );

	failed |= generic_bus_attach_sig( part, &(nWE),     "IO79"  );

	failed |= generic_bus_attach_sig( part, &(SDclk),   "IO11"  );

	failed |= generic_bus_attach_sig( part, &(LCDe),    "IO50"  );

	failed |= generic_bus_attach_sig( part, &(LCDrs),   "IO108" );

	failed |= generic_bus_attach_sig( part, &(LCDrw),   "IO73"  );

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
slsup3_bus_printinfo( bus_t *bus )
{
	int i;

	for (i = 0; i < CHAIN->parts->len; i++)
		if (PART == CHAIN->parts->parts[i])
			break;
	printf( _("SLS UP3 bus driver via BSR (JTAG part No. %d)\n"), i );
}

/**
 * bus->driver->(*prepare)
 *
 */
static void
slsup3_bus_prepare( bus_t *bus )
{
	part_set_instruction( PART, "EXTEST" );
	chain_shift_instructions( CHAIN );
}

/**
 * bus->driver->(*area)
 *
 */
static int
slsup3_bus_area( bus_t *bus, uint32_t adr, bus_area_t *area )
{
	if ((adr >= FLASHSTART) && (adr < (FLASHSTART + FLASHSIZE))) {
		area->description = N_("Flash Memory (2 MByte) byte mode");
		area->start = FLASHSTART;
		area->length = FLASHSIZE;
		area->width = 8; /* 16 */

		return 0;
	}

	if ((adr >= SRAMSTART) && (adr < (SRAMSTART + SRAMSIZE))) {
		area->description = N_("SRAM 128KByte (64K x 16)");
		area->start = SRAMSTART;
		area->length = SRAMSIZE;
		area->width = 16;

		return 0;
	}

	if ((adr >= LCDSTART) && (adr < (LCDSTART + LCDSIZE))) {
		area->description = N_("LCD Display (RS select by A0)");
		area->start = LCDSTART;
		area->length = LCDSIZE;
		area->width = 8;

		return 0;
	}

	area->description = NULL;
	area->start = UINT32_C(0x0400000);
	area->length = UINT64_C(0xFFC00000);
	area->width = 0;

	return 0;
}

static void
setup_address( bus_t *bus, uint32_t a )
{
	int i;
	part_t *p = PART;
	bus_area_t area;

	slsup3_bus_area( bus, a, &area );
	if (area.width > 16)
		return;

	part_set_signal( p, LCDrs, 1, a & 1);

	/* FLASH memory address setup. Use DQ15 to select byte */
	if ((a >= (FLASHSTART)) && (a < (FLASHSTART + FLASHSIZE))) {
	        for (i = 0; i < 20; i++)
		        part_set_signal( p, AD[i], 1, (a >> (i+1)) & 1 );
		part_set_signal( p, nFLce, 1, 0);
		part_set_signal( p, DQ[15], 1, (a & 1));
	} else
	        part_set_signal( p, nFLce, 1, 1);

	/* SRAM memory address setup */
	if ((a >= SRAMSTART) && (a < (SRAMSTART + SRAMSIZE))) {
	        part_set_signal( p, nSRce, 1, 0);
		for (i = 0; i < 20; i++)
		        part_set_signal( p, AD[i], 1, (a >> (i + (area.width / 8) - 1)) & 1 );
	} else
	        part_set_signal( p, nSRce, 1, 1);


}

static void
set_data_in( bus_t *bus, uint32_t adr )
{
	int i;
	part_t *p = PART;
	bus_area_t area;

	slsup3_bus_area( bus, adr, &area );
	if (area.width > 16)
		return;

	for (i = 0; i < area.width; i++)
		part_set_signal( p, DQ[i], 0, 0 );
}

static void
setup_data( bus_t *bus, uint32_t adr, uint32_t d )
{
	int i;
	part_t *p = PART;
	bus_area_t area;

	slsup3_bus_area( bus, adr, &area );
	if (area.width > 16)
		return;

	for (i = 0; i < area.width; i++)
		part_set_signal( p, DQ[i], 1, (d >> i) & 1 );
}

static uint32_t
get_data( bus_t *bus, uint32_t adr )
{
	bus_area_t area;
	int i;
	uint32_t d = 0;
	part_t *p = PART;

	slsup3_bus_area( bus, adr, &area );
	if (area.width > 16)
		return 0;

	for (i = 0; i < area.width; i++)
		d |= (uint32_t) (part_get_signal( p, DQ[i] ) << i);

	return d;
}

/**
 * bus->driver->(*read_start)
 *
 */
static void
slsup3_bus_read_start( bus_t *bus, uint32_t adr )
{
	part_t *p = PART;

	LAST_ADR = adr;

	part_set_signal( p, nSDce, 1, 1); /* Inihibit SDRAM */
	part_set_signal( p, nOE, 1, 0);
	part_set_signal( p, nSRce, 1, 1);
	part_set_signal( p, nFLce, 1, 1);
	part_set_signal( p, nFLbyte, 1, 0);
	part_set_signal( p, nWE, 1, 1);
	part_set_signal( p, SDclk, 1, 0);
	part_set_signal( p, LCDe, 1, 0);
	part_set_signal( p, LCDrw, 1, 1);

	setup_address( bus, adr );

	if ((adr >= LCDSTART) && (adr < (LCDSTART + LCDSIZE))) {
	    part_set_signal( p, LCDe, 1, 1);
	    chain_shift_data_registers( CHAIN, 0 );
	    part_set_signal( p, LCDe, 1, 0);
	}

	set_data_in( bus, adr );

	chain_shift_data_registers( CHAIN, 0 );

}

/**
 * bus->driver->(*read_next)
 *
 */
static uint32_t
slsup3_bus_read_next( bus_t *bus, uint32_t adr )
{
	uint32_t d;

	part_t *p = PART;

	setup_address( bus, adr );

	if ((adr >= LCDSTART) && (adr < (LCDSTART + LCDSIZE))) {
	    part_set_signal( p, LCDe, 1, 1);
	    chain_shift_data_registers( CHAIN, 0 );
	    part_set_signal( p, LCDe, 1, 0);
	}

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
slsup3_bus_read_end( bus_t *bus )
{
	part_t *p = PART;
	uint32_t d;

	if ((LAST_ADR >= LCDSTART) && (LAST_ADR < (LCDSTART + LCDSIZE))) {
	    part_set_signal( p, LCDe, 1, 1);
	    chain_shift_data_registers( CHAIN, 0 );
	    part_set_signal( p, LCDe, 1, 0);
	}

	part_set_signal( p, nOE, 1, 1);

	chain_shift_data_registers( CHAIN, 1 );

	d = get_data( bus, LAST_ADR );

	return d;
}

/**
 * bus->driver->(*write)
 *
 */
static void
slsup3_bus_write( bus_t *bus, uint32_t adr, uint32_t data )
{
	part_t *p = PART;
	chain_t *chain = CHAIN;

	part_set_signal( p, nSDce, 1, 1); /* Inihibit SDRAM */
	part_set_signal( p, nOE, 1, 1);
	part_set_signal( p, nSRce, 1, 1);
	part_set_signal( p, nFLce, 1, 1);
	part_set_signal( p, nFLbyte, 1, 0);
	part_set_signal( p, nWE, 1, 1);
	part_set_signal( p, SDclk, 1, 0);
	part_set_signal( p, LCDe, 1, 0);
	part_set_signal( p, LCDrw, 1, 0);

	setup_address( bus, adr );
	setup_data( bus, adr, data );

	if ((adr >= LCDSTART) && (adr < (LCDSTART + LCDSIZE))) {
	    chain_shift_data_registers( chain, 0 );
	    part_set_signal( p, LCDe, 1, 1);
	    chain_shift_data_registers( CHAIN, 0 );
	    part_set_signal( p, LCDe, 1, 0);
	    chain_shift_data_registers( CHAIN, 0 );
	} else {

	  chain_shift_data_registers( chain, 0 );

	  part_set_signal( p, nWE, 1, 0 );
	  chain_shift_data_registers( chain, 0 );
	  part_set_signal( p, nWE, 1, 1 );
	  chain_shift_data_registers( chain, 0 );
	}
}

const bus_driver_t slsup3_bus = {
	"slsup3",
	N_("SLS UP3 compatible bus driver via BSR"),
	slsup3_bus_new,
	generic_bus_free,
	slsup3_bus_printinfo,
	slsup3_bus_prepare,
	slsup3_bus_area,
	slsup3_bus_read_start,
	slsup3_bus_read_next,
	slsup3_bus_read_end,
	generic_bus_read,
	slsup3_bus_write,
	NULL
};
