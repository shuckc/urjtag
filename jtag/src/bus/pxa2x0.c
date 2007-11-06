/*
 * $Id$
 *
 * Intel PXA2x0 compatible bus driver via BSR
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
 *
 * Documentation:
 * [1] Intel Corporation, "Intel PXA250 and PXA210 Application Processors
 *     Developer's Manual", February 2002, Order Number: 278522-001
 *
 */

#include "sysdep.h"

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <pxa2x0/mc.h>

#include "part.h"
#include "bus.h"
#include "bssignal.h"
#include "jtag.h"
#include "buses.h"

typedef struct {
	chain_t *chain;
	part_t *part;
	uint32_t last_adr;
	signal_t *ma[26];
	signal_t *md[32];
	signal_t *ncs[1];
	signal_t *dqm[4];
	signal_t *rdnwr;
	signal_t *nwe;
	signal_t *noe;
	signal_t *nsdcas;
	MC_registers_t MC_registers;
	int inited;
} bus_params_t;

#define	CHAIN	((bus_params_t *) bus->params)->chain
#define	PART	((bus_params_t *) bus->params)->part
#define	LAST_ADR	((bus_params_t *) bus->params)->last_adr
#define	MA		((bus_params_t *) bus->params)->ma
#define	MD		((bus_params_t *) bus->params)->md
#define	nCS		((bus_params_t *) bus->params)->ncs
#define	DQM		((bus_params_t *) bus->params)->dqm
#define	RDnWR		((bus_params_t *) bus->params)->rdnwr
#define	nWE		((bus_params_t *) bus->params)->nwe
#define	nOE		((bus_params_t *) bus->params)->noe
#define	nSDCAS		((bus_params_t *) bus->params)->nsdcas

#define	MC_pointer	(&((bus_params_t *) bus->params)->MC_registers)

#define	INITED		((bus_params_t *) bus->params)->inited

static void
setup_address( bus_t *bus, uint32_t a )
{
	int i;
	part_t *p = PART;

	for (i = 0; i < 26; i++)
		part_set_signal( p, MA[i], 1, (a >> i) & 1 );
}

static int pxa2x0_bus_area( bus_t *bus, uint32_t adr, bus_area_t *area );

static void
set_data_in( bus_t *bus, uint32_t adr )
{
	int i;
	part_t *p = PART;
	bus_area_t area;

	pxa2x0_bus_area( bus, adr, &area );

	for (i = 0; i < area.width; i++)
		part_set_signal( p, MD[i], 0, 0 );
}

static void
setup_data( bus_t *bus, uint32_t adr, uint32_t d )
{
	int i;
	part_t *p = PART;
	bus_area_t area;

	pxa2x0_bus_area( bus, adr, &area );

	for (i = 0; i < area.width; i++)
		part_set_signal( p, MD[i], 1, (d >> i) & 1 );
}

static void
pxa2x0_bus_printinfo( bus_t *bus )
{
	int i;

	for (i = 0; i < CHAIN->parts->len; i++)
		if (PART == CHAIN->parts->parts[i])
			break;
	printf( _("Intel PXA2x0 compatible bus driver via BSR (JTAG part No. %d)\n"), i );
}

static void
pxa2x0_bus_init( bus_t *bus )
{
	chain_t *chain = CHAIN;
	part_t *p = PART;

	if (INITED == 1)
		return;

	part_set_instruction( p, "SAMPLE/PRELOAD" );
	chain_shift_instructions( chain );
	chain_shift_data_registers( chain, 1 );

	BOOT_DEF = BOOT_DEF_PKG_TYPE | BOOT_DEF_BOOT_SEL(part_get_signal( p, part_find_signal( p, "BOOT_SEL[2]" ) ) << 2
							| part_get_signal( p, part_find_signal( p, "BOOT_SEL[1]" ) ) << 1
							| part_get_signal( p, part_find_signal( p, "BOOT_SEL[0]" ) ));

	part_set_instruction( p, "BYPASS" );
	chain_shift_instructions( chain );

	INITED = 1;
}

static void
pxa250_bus_prepare( bus_t *bus )
{
	pxa2x0_bus_init( bus );

	part_set_instruction( PART, "EXTEST" );
	chain_shift_instructions( CHAIN );
}

static void
pxa250_bus_read_start( bus_t *bus, uint32_t adr )
{
	chain_t *chain = CHAIN;
	part_t *p = PART;

	LAST_ADR = adr;
	if (adr >= 0x04000000)
		return;

	/* see Figure 6-13 in [1] */
	part_set_signal( p, nCS[0], 1, 0 );
	part_set_signal( p, DQM[0], 1, 0 );
	part_set_signal( p, DQM[1], 1, 0 );
	part_set_signal( p, DQM[2], 1, 0 );
	part_set_signal( p, DQM[3], 1, 0 );
	part_set_signal( p, RDnWR, 1, 1 );
	part_set_signal( p, nWE, 1, 1 );
	part_set_signal( p, nOE, 1, 0 );
	part_set_signal( p, nSDCAS, 1, 0 );

	setup_address( bus, adr );
	set_data_in( bus, adr );

	chain_shift_data_registers( chain, 0 );
}

static uint32_t
pxa250_bus_read_next( bus_t *bus, uint32_t adr )
{
	part_t *p = PART;
	chain_t *chain = CHAIN;
	uint32_t d;
	uint32_t old_last_adr = LAST_ADR;

	LAST_ADR = adr;

	if (adr < UINT32_C(0x04000000)) {
		int i;
		bus_area_t area;

		pxa2x0_bus_area( bus, adr, &area );

		/* see Figure 6-13 in [1] */
		setup_address( bus, adr );
		chain_shift_data_registers( chain, 1 );

		d = 0;
		for (i = 0; i < area.width; i++)
			d |= (uint32_t) (part_get_signal( p, MD[i] ) << i);

		return d;
	}

	if (adr < UINT32_C(0x48000000))
		return 0;

	if (adr < UINT32_C(0x4C000000)) {
		if (old_last_adr == (MC_BASE + BOOT_DEF_OFFSET))
			return BOOT_DEF;

		return 0;
	}

	return 0;
}

static uint32_t
pxa250_bus_read_end( bus_t *bus )
{
	part_t *p = PART;
	chain_t *chain = CHAIN;

	if (LAST_ADR < UINT32_C(0x04000000)) {
		int i;
		uint32_t d = 0;
		bus_area_t area;

		pxa2x0_bus_area( bus, LAST_ADR, &area );

		/* see Figure 6-13 in [1] */
		part_set_signal( p, nCS[0], 1, 1 );
		part_set_signal( p, nOE, 1, 1 );
		part_set_signal( p, nSDCAS, 1, 1 );

		chain_shift_data_registers( chain, 1 );

		for (i = 0; i < area.width; i++)
			d |= (uint32_t) (part_get_signal( p, MD[i] ) << i);

		return d;
	}

	if (LAST_ADR < UINT32_C(0x48000000))
		return 0;

	if (LAST_ADR < UINT32_C(0x4C000000)) {
		if (LAST_ADR == (MC_BASE + BOOT_DEF_OFFSET))
			return BOOT_DEF;

		return 0;
	}

	return 0;
}

static uint32_t
pxa250_bus_read( bus_t *bus, uint32_t adr )
{
	pxa250_bus_read_start( bus, adr );
	return pxa250_bus_read_end( bus );
}

static void
pxa250_bus_write( bus_t *bus, uint32_t adr, uint32_t data )
{
	/* see Figure 6-17 in [1] */
	part_t *p = PART;
	chain_t *chain = CHAIN;

	if (adr >= 0x04000000)
		return;

	part_set_signal( p, nCS[0], 1, 0 );
	part_set_signal( p, DQM[0], 1, 0 );
	part_set_signal( p, DQM[1], 1, 0 );
	part_set_signal( p, DQM[2], 1, 0 );
	part_set_signal( p, DQM[3], 1, 0 );
	part_set_signal( p, RDnWR, 1, 0 );
	part_set_signal( p, nWE, 1, 1 );
	part_set_signal( p, nOE, 1, 1 );
	part_set_signal( p, nSDCAS, 1, 0 );

	setup_address( bus, adr );
	setup_data( bus, adr, data );

	chain_shift_data_registers( chain, 0 );

	part_set_signal( p, nWE, 1, 0 );
	chain_shift_data_registers( chain, 0 );
	part_set_signal( p, nWE, 1, 1 );
	chain_shift_data_registers( chain, 0 );
}

static int
pxa2x0_bus_area( bus_t *bus, uint32_t adr, bus_area_t *area )
{
	pxa2x0_bus_init( bus );

	/* Static Chip Select 0 (64 MB) */
	if (adr < UINT32_C(0x04000000)) {
		area->description = N_("Static Chip Select 0");
		area->start = UINT32_C(0x00000000);
		area->length = UINT64_C(0x04000000);

		/* see Table 6-36. in [1] */
		switch (get_BOOT_DEF_BOOT_SEL(BOOT_DEF)) {
			case 0:
				area->width = 32;
				break;
			case 1:
				area->width = 16;
				break;
			case 2:
			case 3:
				area->width = 0;
				break;
			case 4:
			case 5:
			case 6:
			case 7:
				printf( "TODO - BOOT_SEL: %d\n", get_BOOT_DEF_BOOT_SEL(BOOT_DEF) );
				return -1;
			default:
				printf( "BUG in the code, file %s, line %d.\n", __FILE__, __LINE__ );
				return -1;
		}
		return 0;
	}

	if (adr < UINT32_C(0x48000000)) {
		area->description = NULL;
		area->start = UINT32_C(0x04000000);
		area->length = UINT64_C(0x44000000);
		area->width = 0;

		return 0;
	}

	if (adr < UINT32_C(0x4C000000)) {
		area->description = N_("Memory Mapped registers (Memory Ctl)");
		area->start = UINT32_C(0x48000000);
		area->length = UINT64_C(0x04000000);
		area->width = 32;

		return 0;
	}

	area->description = NULL;
	area->start = UINT32_C(0x4C000000);
	area->length = UINT64_C(0xB4000000);
	area->width = 0;

	return 0;
}

static void
pxa250_bus_free( bus_t *bus )
{
	free( bus->params );
	free( bus );
}

static bus_t *pxa2x0_bus_new( void );

const bus_driver_t pxa2x0_bus = {
	"pxa2x0",
	N_("Intel PXA2x0 compatible bus driver via BSR"),
	pxa2x0_bus_new,
	pxa250_bus_free,
	pxa2x0_bus_printinfo,
	pxa250_bus_prepare,
	pxa2x0_bus_area,
	pxa250_bus_read_start,
	pxa250_bus_read_next,
	pxa250_bus_read_end,
	pxa250_bus_read,
	pxa250_bus_write,
    NULL
};

static bus_t *
pxa2x0_bus_new( void )
{
	bus_t *bus;
	char buff[10];
	int i;
	int failed = 0;

	if (!chain || !chain->parts || chain->parts->len <= chain->active_part || chain->active_part < 0)
		return NULL;

	bus = malloc( sizeof (bus_t) );
	if (!bus)
		return NULL;

	bus->driver = &pxa2x0_bus;
	bus->params = calloc( 1, sizeof (bus_params_t) );
	if (!bus->params) {
		free( bus );
		return NULL;
	}

	CHAIN = chain;
	PART = chain->parts->parts[chain->active_part];

	for (i = 0; i < 26; i++) {
		sprintf( buff, "MA[%d]", i );
		MA[i] = part_find_signal( PART, buff );
		if (!MA[i]) {
			printf( _("signal '%s' not found\n"), buff );
			failed = 1;
			break;
		}
	}
	for (i = 0; i < 32; i++) {
		sprintf( buff, "MD[%d]", i );
		MD[i] = part_find_signal( PART, buff );
		if (!MD[i]) {
			printf( _("signal '%s' not found\n"), buff );
			failed = 1;
			break;
		}
	}
	for (i = 0; i < 1; i++) {
		sprintf( buff, "nCS[%d]", i );
		nCS[i] = part_find_signal( PART, buff );
		if (!nCS[i]) {
			printf( _("signal '%s' not found\n"), buff );
			failed = 1;
			break;
		}
	}
	for (i = 0; i < 4; i++) {
		sprintf( buff, "DQM[%d]", i );
		DQM[i] = part_find_signal( PART, buff );
		if (!DQM[i]) {
			printf( _("signal '%s' not found\n"), buff );
			failed = 1;
			break;
		}
	}
	RDnWR = part_find_signal( PART, "RDnWR" );
	if (!RDnWR) {
		printf( _("signal '%s' not found\n"), "RDnWR" );
		failed = 1;
	}
	nWE = part_find_signal( PART, "nWE" );
	if (!nWE) {
		printf( _("signal '%s' not found\n"), "nWE" );
		failed = 1;
	}
	nOE = part_find_signal( PART, "nOE" );
	if (!nOE) {
		printf( _("signal '%s' not found\n"), "nOE" );
		failed = 1;
	}
	nSDCAS = part_find_signal( PART, "nSDCAS" );
	if (!nSDCAS) {
		printf( _("signal '%s' not found\n"), "nSDCAS" );
		failed = 1;
	}

	if (failed) {
		free( bus->params );
		free( bus );
		return NULL;
	}

	INITED = 0;

	return bus;
}
