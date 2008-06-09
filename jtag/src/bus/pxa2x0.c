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
 * 2005-01-29: Cliff Brake <cliff.brake@gmail.com> <http://bec-systems.com>
 *   - added support for PXA270
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

#include "part.h"
#include "bus.h"
#include "bssignal.h"
#include "jtag.h"
#include "buses.h"
#include "generic_bus.h"

#include "pxa2x0_mc.h"


/*
 * the following defines are used in proc field of the the
 * bus_params_t structure and are used in various functions
 * below
 */

#define PROC_PXA25x	1	// including px26x series
#define PROC_PXA27x	2


#define nCS_TOTAL 6

typedef struct {
	char* sig_name;
	int enabled;
        int bus_width;  // set 0 for disabled (or auto-detect)
	char label_buf[81];
} ncs_map_entry;

/*
 * Tables indexed by nCS[index]
 * An array of plain char* would probably do it too, but anyway...
 *
 * Note: the setup of nCS[*] is board-specific, rather than chip-specific!
 * The memory mapping and nCS[*] functions are normally set up by the boot loader.
 * In our JTAG code, we manipulate the outer pins explicitly, without the help
 * of the CPU's memory controller - hence the need to mimick its setup.
 *
 * Note that bus_area() and bus_read()/bus_write() use a window of 64MB
 * per nCS pin (26bit addresses), which seems to be the most common option.
 * For static CS[0] and CS[1] == 128 MB, the algorithms have to be modified...
 */

// Fool-proof basic mapping with only nCS[0] wired.
// nCS[0] doesn't collide with any other GPIO functions.
static ncs_map_entry pxa25x_ncs_map[nCS_TOTAL] = {
	{"nCS[0]", 1, 0},
	{NULL, 0, 0},
	{NULL, 0, 0},
	{NULL, 0, 0},
	{NULL, 0, 0},
	{NULL, 0, 0}
};

// Default mapping with all nCS[*] GPIO pins used as nCS.
// Note that the same GPIO pins might be used e.g. for PCCard
// service space access or PWM outputs, or some other purpose.
static ncs_map_entry pxa27x_ncs_map[nCS_TOTAL] = {
	{"nCS[0]",   1,  0},   // nCS[0]
	{"GPIO[15]", 1, 16},   // nCS[1]
	{"GPIO[78]", 1, 16},   // nCS[2]
	{"GPIO[79]", 1, 16},   // nCS[3]
	{"GPIO[80]", 1, 16},   // nCS[4]
	{"GPIO[33]", 1, 16}    // nCS[5]
};


typedef struct {
	chain_t *chain;
	part_t *part;
	uint32_t last_adr;
	signal_t *ma[26];
	signal_t *md[32];
	signal_t *ncs[nCS_TOTAL];
	signal_t *dqm[4];
	signal_t *rdnwr;
	signal_t *nwe;
	signal_t *noe;
	signal_t *nsdcas;
	MC_registers_t MC_registers;
	int inited;
	int proc;
} bus_params_t;

#define	CHAIN	((bus_params_t *) bus->params)->chain
#define	PART	((bus_params_t *) bus->params)->part
#define PROC	((bus_params_t *) bus->params)->proc
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

/*
 * bus->driver->(*new_bus)
 *
 */
static int
pxa2xx_bus_new_common(bus_t * bus)
{
        int failed = 0;
        ncs_map_entry* ncs_map = NULL;
#ifdef PREPATCHNEVER
	bus_t *bus;
	char buff[10];
	int i;

	if (!chain || !chain->parts || chain->parts->len <= chain->active_part || chain->active_part < 0)
		return NULL;

	bus = calloc( 1, sizeof (bus_t) );
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
#endif
	int i;
	char buff[10];

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

	if (PROC == PROC_PXA25x) {
		ncs_map = pxa25x_ncs_map;
	}
	else if (PROC == PROC_PXA27x) {
		ncs_map = pxa27x_ncs_map;
	}
	else
	{
		printf( "BUG in the code, file %s, line %d: unknown PROC\n", __FILE__, __LINE__ );
		ncs_map = pxa25x_ncs_map; // be dumb by default
	}
	for (i = 0; i < nCS_TOTAL; i++) {
		if (ncs_map[i].enabled > 0)
		{
			nCS[i] = part_find_signal( PART, ncs_map[i].sig_name );
			if (!nCS[i]) {
				printf( _("signal '%s' not found\n"), buff );
				failed = 1;
				break;
			}
		}
		else // disabled - this GPIO pin is unused or used for some other function
		{
			nCS[i] = NULL;
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

	return failed;
}

/**
 * bus->driver->(*new_bus)
 *
 */
static bus_t *
pxa2x0_bus_new( chain_t *chain, char *cmd_params[] )
{
	bus_t *bus;
	int failed = 0;

	if (!chain || !chain->parts || chain->parts->len <= chain->active_part || chain->active_part < 0)
		return NULL;

	bus = calloc( 1, sizeof (bus_t) );
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
	PROC = PROC_PXA25x;

	failed = pxa2xx_bus_new_common(bus);

	if (failed) {
		free( bus->params );
		free( bus );
		return NULL;
	}

	INITED = 0;

	return bus;
}

/**
 * bus->driver->(*new_bus)
 *
 */
static bus_t *
pxa27x_bus_new( chain_t *chain, char *cmd_params[] )
{
	bus_t *bus;
	int failed = 0;

	if (!chain || !chain->parts || chain->parts->len <= chain->active_part || chain->active_part < 0)
		return NULL;

	bus = calloc( 1, sizeof (bus_t) );
	if (!bus)
		return NULL;

	bus->driver = &pxa27x_bus;
	bus->params = calloc( 1, sizeof (bus_params_t) );
	if (!bus->params) {
		free( bus );
		return NULL;
	}

	CHAIN = chain;
	PART = chain->parts->parts[chain->active_part];
	PROC = PROC_PXA27x;

	failed = pxa2xx_bus_new_common(bus);

	if (failed) {
		free( bus->params );
		free( bus );
		return NULL;
	}

	INITED = 0;

	return bus;
}

/**
 * bus->driver->(*printinfo)
 *
 */
static void
pxa2x0_bus_printinfo( bus_t *bus )
{
	int i;

	for (i = 0; i < CHAIN->parts->len; i++)
		if (PART == CHAIN->parts->parts[i])
			break;
	printf( _("Intel PXA2x0 compatible bus driver via BSR (JTAG part No. %d)\n"), i );
}

/**
 * bus->driver->(*printinfo)
 *
 */
static void
pxa27x_bus_printinfo( bus_t *bus )
{
	int i;

	for (i = 0; i < CHAIN->parts->len; i++)
		if (PART == CHAIN->parts->parts[i])
			break;
	printf( _("Intel PXA27x compatible bus driver via BSR (JTAG part No. %d)\n"), i );
}

/**
 * bus->driver->(*init)
 *
 */
static int
pxa2xx_bus_init( bus_t *bus )
{
	chain_t *chain = CHAIN;
	part_t *p = PART;

	if (INITED == 1)
		return 0;

	part_set_instruction( p, "SAMPLE/PRELOAD" );
	chain_shift_instructions( chain );
	chain_shift_data_registers( chain, 1 );

	if (PROC == PROC_PXA25x)
	{
		BOOT_DEF = BOOT_DEF_PKG_TYPE |
			BOOT_DEF_BOOT_SEL(part_get_signal( p, part_find_signal( p, "BOOT_SEL[2]" ) ) << 2
					| part_get_signal( p, part_find_signal( p, "BOOT_SEL[1]" ) ) << 1
					| part_get_signal( p, part_find_signal( p, "BOOT_SEL[0]" ) ));
	}
	else if (PROC == PROC_PXA27x)
	{
		BOOT_DEF = BOOT_DEF_PKG_TYPE |
			BOOT_DEF_BOOT_SEL(part_get_signal( p, part_find_signal( p, "BOOT_SEL" ) ));
	}
	else
		printf( "BUG in the code, file %s, line %d.\n", __FILE__, __LINE__ );

	part_set_instruction( p, "BYPASS" );
	chain_shift_instructions( chain );

	INITED = 1;

	return 0;
}

/**
 * bus->driver->(*prepare)
 *
 */
static void
pxa2xx_bus_prepare( bus_t *bus )
{
	(void)pxa2xx_bus_init( bus );

	part_set_instruction( PART, "EXTEST" );
	chain_shift_instructions( CHAIN );
}

/**
 * bus->driver->(*area)
 *
 */
static int
pxa2xx_bus_area( bus_t *bus, uint32_t adr, bus_area_t *area )
{
	uint32_t tmp_addr;
	int ncs_index;
	(void)pxa2xx_bus_init( bus );

	/* Static Chip Select 0 (64 MB) */
	if (adr < UINT32_C(0x04000000)) {
		area->description = N_("Static Chip Select 0");
		area->start = UINT32_C(0x00000000);
		area->length = UINT64_C(0x04000000);

		if (pxa25x_ncs_map[0].bus_width > 0)
		{
			area->width = pxa25x_ncs_map[0].bus_width;
		}
		else
		{
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
		}
		return 0;
	}

	/* Static Chip Select 1..5 (per 64 MB) */
	for (ncs_index = 1, tmp_addr = 0x04000000; ncs_index <= 5; ncs_index++, tmp_addr += 0x04000000)
	{
		if ((adr >= tmp_addr) && (adr < tmp_addr + 0x04000000)) { // if the addr is within our window
			sprintf(pxa25x_ncs_map[ncs_index].label_buf, "Static Chip Select %d = %s %s",
				ncs_index, pxa25x_ncs_map[ncs_index].sig_name,
				pxa25x_ncs_map[ncs_index].enabled ? "" : "(disabled)");
			area->description = pxa25x_ncs_map[ncs_index].label_buf;
			area->start = tmp_addr;
			area->length = UINT64_C(0x04000000);
			area->width = pxa25x_ncs_map[ncs_index].bus_width;

			return 0;
		}
	}

	if (adr < UINT32_C(0x48000000)) {
		area->description = NULL;
		area->start = UINT32_C(0x18000000);
		area->length = UINT64_C(0x30000000);
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

/**
 * bus->driver->(*area)
 *
 */
static int
pxa27x_bus_area( bus_t *bus, uint32_t adr, bus_area_t *area )
{
	uint32_t tmp_addr;
	int ncs_index;
	(void)pxa2xx_bus_init( bus );

	/* Static Chip Select 0 (64 MB) */
	if (adr < UINT32_C(0x04000000)) {
		area->description = N_("Static Chip Select 0");
		area->start = UINT32_C(0x00000000);
		area->length = UINT64_C(0x04000000);

		if (pxa27x_ncs_map[0].bus_width > 0)
		{
			area->width = pxa27x_ncs_map[0].bus_width;
		}
		else
		{
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
                }
		return 0;
	}

	/* Static Chip Select 1..5 (per 64 MB) */
	for (ncs_index = 1, tmp_addr = 0x04000000; ncs_index <= 5; ncs_index++, tmp_addr += 0x04000000)
	{
		//printf( "Checking area %08X - %08X... ", tmp_addr, tmp_addr + 0x04000000 - 1);
		if ((adr >= tmp_addr) && (adr < tmp_addr + 0x04000000)) { // if the addr is within our window
			//printf( "match\n");
			sprintf(pxa27x_ncs_map[ncs_index].label_buf, "Static Chip Select %d = %s %s",
				ncs_index, pxa27x_ncs_map[ncs_index].sig_name,
				pxa27x_ncs_map[ncs_index].enabled ? "" : "(disabled)");
			area->description = pxa27x_ncs_map[ncs_index].label_buf;
			area->start = tmp_addr;
			area->length = UINT64_C(0x04000000);
			area->width = pxa27x_ncs_map[ncs_index].bus_width;

			return 0;
		}
		//else printf( "no match\n");
	}

	if (adr < UINT32_C(0x40000000)) {
		area->description = NULL;
		area->start = UINT32_C(0x18000000);
		area->length = UINT64_C(0x28000000);
		area->width = 0;

		return 0;
	}

	if (adr < UINT32_C(0x60000000)) {
		area->description = N_("PXA270 internal address space (cfg, SRAM)");
		area->start = UINT32_C(0x40000000);
		area->length = UINT64_C(0x20000000);
		area->width = 32;

		return 0;
	}

	if (adr < UINT32_C(0xA0000000)) {
		area->description = NULL;
		area->start = UINT32_C(0x60000000);
		area->length = UINT64_C(0x40000000);
		area->width = 0;

		return 0;
	}

	if (adr < UINT32_C(0xB0000000)) {
		area->description = N_("PXA270 SDRAM space (4x 64MB)");
		area->start = UINT32_C(0xA0000000);
		area->length = UINT64_C(0x10000000);
		area->width = 32;

		return 0;
	}

	area->description = NULL;
	area->start = UINT32_C(0xB0000000);
	area->length = UINT64_C(0x50000000);
	area->width = 0;

	return 0;
}

static void
setup_address( bus_t *bus, uint32_t a )
{
	int i;
	part_t *p = PART;

	for (i = 0; i < 26; i++)
		part_set_signal( p, MA[i], 1, (a >> i) & 1 );
}

static void
set_data_in( bus_t *bus, uint32_t adr )
{
	int i;
	part_t *p = PART;
	bus_area_t area;

	bus->driver->area( bus, adr, &area );

	for (i = 0; i < area.width; i++)
		part_set_signal( p, MD[i], 0, 0 );
}

static void
setup_data( bus_t *bus, uint32_t adr, uint32_t d )
{
	int i;
	part_t *p = PART;
	bus_area_t area;

	bus->driver->area( bus, adr, &area );

	for (i = 0; i < area.width; i++)
		part_set_signal( p, MD[i], 1, (d >> i) & 1 );
}

/**
 * bus->driver->(*read_start)
 *
 */
static void
pxa2xx_bus_read_start( bus_t *bus, uint32_t adr )
{
	int cs_index = 0;

	chain_t *chain = CHAIN;
	part_t *p = PART;

	LAST_ADR = adr;
	if (adr >= 0x18000000)
		return;

	cs_index = adr >> 26;
	if (nCS[cs_index] == NULL)
		return;

	/* see Figure 6-13 in [1] */
	part_set_signal( p, nCS[cs_index], 1, 0 );
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

/**
 * bus->driver->(*read_next)
 *
 */
static uint32_t
pxa2xx_bus_read_next( bus_t *bus, uint32_t adr )
{
	part_t *p = PART;
	chain_t *chain = CHAIN;
	uint32_t d;
	uint32_t old_last_adr = LAST_ADR;

	LAST_ADR = adr;

	if (adr < UINT32_C(0x18000000)) {
		int i;
		bus_area_t area;

		if (nCS[adr >> 26] == NULL) // avoid undefined nCS windows
			return 0;

		bus->driver->area( bus, adr, &area );

		/* see Figure 6-13 in [1] */
		setup_address( bus, adr );
		chain_shift_data_registers( chain, 1 );

		d = 0;
		for (i = 0; i < area.width; i++)
			d |= (uint32_t) (part_get_signal( p, MD[i] ) << i);

		return d;
	}

        // anything above 0x18000000 is essentially unreachable...
	if (adr < UINT32_C(0x48000000))
		return 0;

	if (adr < UINT32_C(0x4C000000)) {
		if (old_last_adr == (MC_BASE + BOOT_DEF_OFFSET))
			return BOOT_DEF;

		return 0;
	}

	return 0;
}

/**
 * bus->driver->(*read_end)
 *
 */
static uint32_t
pxa2xx_bus_read_end( bus_t *bus )
{
	part_t *p = PART;
	chain_t *chain = CHAIN;

	if (LAST_ADR < UINT32_C(0x18000000)) {
		int i;
		uint32_t d = 0;
		bus_area_t area;

		if (nCS[LAST_ADR >> 26] == NULL) // avoid undefined nCS windows
			return 0;

		bus->driver->area( bus, LAST_ADR, &area );

		/* see Figure 6-13 in [1] */
		part_set_signal( p, nCS[0], 1, 1 );
		part_set_signal( p, nOE, 1, 1 );
		part_set_signal( p, nSDCAS, 1, 1 );

		chain_shift_data_registers( chain, 1 );

		for (i = 0; i < area.width; i++)
			d |= (uint32_t) (part_get_signal( p, MD[i] ) << i);

		return d;
	}

        // anything above 0x18000000 is essentially unreachable...
	if (LAST_ADR < UINT32_C(0x48000000))
		return 0;

	if (LAST_ADR < UINT32_C(0x4C000000)) {
		if (LAST_ADR == (MC_BASE + BOOT_DEF_OFFSET))
			return BOOT_DEF;

		return 0;
	}

	return 0;
}

/**
 * bus->driver->(*write)
 *
 */
static void
pxa2xx_bus_write( bus_t *bus, uint32_t adr, uint32_t data )
{
	int cs_index = 0;

	/* see Figure 6-17 in [1] */
	part_t *p = PART;
	chain_t *chain = CHAIN;

	if (adr >= 0x18000000)
		return;

	cs_index = adr >> 26;
	if (nCS[cs_index] == NULL)
		return;

	part_set_signal( p, nCS[cs_index], 1, 0 );
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

const bus_driver_t pxa2x0_bus = {
	"pxa2x0",
	N_("Intel PXA2x0 compatible bus driver via BSR"),
	pxa2x0_bus_new,
	generic_bus_free,
	pxa2x0_bus_printinfo,
	pxa2xx_bus_prepare,
	pxa2xx_bus_area,
	pxa2xx_bus_read_start,
	pxa2xx_bus_read_next,
	pxa2xx_bus_read_end,
	generic_bus_read,
	pxa2xx_bus_write,
	NULL /* patch 909598 call pxax0_bus_init, but the patch fails and doesnt look compatible */
};

const bus_driver_t pxa27x_bus = {
	"pxa27x",
	N_("Intel PXA27x compatible bus driver via BSR"),
	pxa27x_bus_new,
	generic_bus_free,
	pxa27x_bus_printinfo,
	pxa2xx_bus_prepare,
	pxa27x_bus_area,
	pxa2xx_bus_read_start,
	pxa2xx_bus_read_next,
	pxa2xx_bus_read_end,
	generic_bus_read,
	pxa2xx_bus_write,
	pxa2xx_bus_init
};
