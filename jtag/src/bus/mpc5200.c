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

#define LPC_NUM_CS	6
#define LPC_NUM_AD	24
#define LPC_NUM_D	8
#define LPC_ADDR_TO_CS(a) (a >> LPC_NUM_AD)
#define LPC_ADDR_SIZE ( ( 0x00000001 << LPC_NUM_AD ) * LPC_NUM_CS )


typedef struct {
	uint32_t last_adr;
	signal_t *ad[LPC_NUM_AD];
	signal_t *ncs[LPC_NUM_CS];
	signal_t *nwe;
	signal_t *noe;
	signal_t *d[LPC_NUM_D];
	signal_t *ata_iso;
} bus_params_t;

#define	LAST_ADR	((bus_params_t *) bus->params)->last_adr
#define	AD		((bus_params_t *) bus->params)->ad
#define	nCS		((bus_params_t *) bus->params)->ncs
#define	nWE		((bus_params_t *) bus->params)->nwe
#define	nOE		((bus_params_t *) bus->params)->noe
#define	D		((bus_params_t *) bus->params)->d
#define ATA_ISO		((bus_params_t *) bus->params)->ata_iso

/**
 * bus->driver->(*new_bus)
 *
 */
static bus_t *
mpc5200_bus_new( chain_t *chain, const bus_driver_t *driver, char *cmd_params[] )
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

	/* Get the signals */
	for (i = 0; i < LPC_NUM_AD; i++) {
		sprintf( buff, "EXT_AD_%d", i );
		failed |= generic_bus_attach_sig( part, &(AD[i]), buff );
	}

	for (i = 0; i < LPC_NUM_CS; i++) {
		sprintf( buff, "LP_CS%d_B", i );
		failed |= generic_bus_attach_sig( part, &(nCS[i]), buff );
	}

	failed |= generic_bus_attach_sig( part, &(nWE), "LP_RW" );

	failed |= generic_bus_attach_sig( part, &(nOE), "LP_OE" );

	for (i = 0; i < LPC_NUM_D; i++) {
		sprintf( buff, "EXT_AD_%d", i+LPC_NUM_AD );
		failed |= generic_bus_attach_sig( part, &(D[i]), buff );
	}

	failed |= generic_bus_attach_sig( part, &(ATA_ISO), "ATA_ISOLATION" );

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
 * bus->driver->(*area)
 *
 */
static int
mpc5200_bus_area( bus_t *bus, uint32_t adr, bus_area_t *area )
{
	if( adr < LPC_ADDR_SIZE )
	{
		area->description = N_("LocalPlus Bus");
		area->start = UINT32_C(0x00000000);
		area->length = LPC_ADDR_SIZE;
		area->width = LPC_NUM_D;
		return URJTAG_STATUS_OK;
	}

	area->description = NULL;
	area->start = LPC_ADDR_SIZE;
	area->length = UINT64_C(0x100000000) - LPC_ADDR_SIZE;
	area->width = 0;
	return URJTAG_STATUS_OK;
}

static void
setup_address( bus_t *bus, uint32_t a )
{
	int i;
	part_t *p = PART;

	for (i = 0; i < LPC_NUM_AD; i++)
		part_set_signal( p, AD[i], 1, (a >> i) & 1 );
}

static void
set_data_in( bus_t *bus, uint32_t adr )
{
	int i;
	part_t *p = PART;
	bus_area_t area;

	mpc5200_bus_area( bus, adr, &area );
	if (area.width > LPC_NUM_D)
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
	if (area.width > LPC_NUM_D)
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
	if (area.width > LPC_NUM_D)
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
	int i;
	uint8_t cs = LPC_ADDR_TO_CS(adr);

	LAST_ADR = adr;

	/* see Figure 6-45 in [1] */

	for (i = 0; i < LPC_NUM_CS; i++) {
		part_set_signal( p, nCS[i], 1, !(cs==i) );
	}

	part_set_signal( p, ATA_ISO,1, 1 );
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
	int i;

	for (i = 0; i < LPC_NUM_CS; i++) {
		part_set_signal( p, nCS[i], 1, 1 );
	}
	part_set_signal( p, nOE, 1, 1 );

	chain_shift_data_registers( CHAIN, 1 );

	return get_data( bus, LAST_ADR );
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
	uint8_t cs = LPC_ADDR_TO_CS(adr);
	int i;

	for (i = 0; i < LPC_NUM_CS; i++) {
		part_set_signal( p, nCS[i], 1, !(cs==i) );
	}
	part_set_signal( p, ATA_ISO,1, 1 );
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
	generic_bus_prepare_extest,
	mpc5200_bus_area,
	mpc5200_bus_read_start,
	mpc5200_bus_read_next,
	mpc5200_bus_read_end,
	generic_bus_read,
	mpc5200_bus_write,
	generic_bus_no_init
};
