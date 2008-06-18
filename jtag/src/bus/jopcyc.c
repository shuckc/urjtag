/*
 * $Id$
 *
 * Bus driver for the Cyclone Boards manufactured by JOP.design.
 *
 *   http://www.jopdesign.com/
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
 * Written by Arnim Laeuger <arniml@users.sourceforge.net>, 2005.
 *
 * Notes:
 * ------
 *   This driver supports both RAMs and the Flash memory found on
 *   the Cyclone Boards. So far, it has been tested with the EP1C12
 *   board - the CYCBIG1M32M product. In general, the board equipped
 *   with the EP1C6 should work without any modifications of this
 *   driver. You will definitely require a proper device description
 *   for the EP1C6Q240.
 *
 *   http://jopdesign.com/cyclone/cyc.pdf
 *
 *   The three external components are assigned different address
 *   ranges. These are arbtitrary but help to distinguish the devices.
 *
 *     RAMA:  0x00000000 - 0x0007FFFF
 *     RAMB:  0x00080000 - 0x000FFFFF
 *     Flash: 0x00100000 - 0x0017FFFF
 *
 *   JTAG Tool generates byte addresses when accessing memories. Thus
 *   this driver discards the LSB when the RAM ranges are addressed.
 *   readmem and writemem care for proper address increment based on
 *   the bus width.
 *   On the other hand, this driver reads and writes always one word
 *   (= 2 bytes) from/to the RAMs. It does not use the byte-enables.
 *   This is mainly due to the lack of byte-enable information in the
 *   bus-driver API.
 *
 *   Remember to clarify the endianess of your data when working with
 *   the RAMs.
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

#define RAM_ADDR_WIDTH 18
#define RAM_DATA_WIDTH 16
#define FLASH_ADDR_WIDTH 19
#define FLASH_DATA_WIDTH 8

/* length is in number of bytes
   the full address width is taken to build the power of 2 */
#define RAM_LENGTH (1 << (RAM_ADDR_WIDTH+1))
#define FLASH_LENGTH (1 << FLASH_ADDR_WIDTH)

#define RAMA_START 0
#define RAMB_START RAM_LENGTH
#define FLASH_START 2*RAM_LENGTH

typedef enum {RAM, FLASH, NAND} ctype_t;

typedef struct {
	ctype_t  ctype;
	char *cname;
	signal_t *a[FLASH_ADDR_WIDTH];
	signal_t *d[RAM_DATA_WIDTH];
	signal_t *ncs;
	signal_t *noe;
	signal_t *nwe;
	signal_t *nlb;
	signal_t *nub;
	signal_t *ncs2;
	signal_t *nrdy;
} component_t;

typedef struct {
	uint32_t     last_addr; /* holds last address of read or write access */
	component_t  rama;
	component_t  ramb;
	component_t  flash;
	signal_t     *ser_txd;
	signal_t     *ser_nrts;
	signal_t     *ser_rxd;
	signal_t     *ser_ncts;
} bus_params_t;

#define LAST_ADDR ((bus_params_t *) bus->params)->last_addr
#define A         comp->a
#define D         comp->d
#define nCS       comp->ncs
#define nOE       comp->noe
#define nWE       comp->nwe
#define nLB       comp->nlb
#define nUB       comp->nub
#define nCS2      comp->ncs2
#define nRDY      comp->nrdy

#define COMP_RAMA  &(((bus_params_t *) bus->params)->rama)
#define COMP_RAMB  &(((bus_params_t *) bus->params)->ramb)
#define COMP_FLASH &(((bus_params_t *) bus->params)->flash)

#define SER_RXD  ((bus_params_t *) bus->params)->ser_rxd
#define SER_NRTS ((bus_params_t *) bus->params)->ser_nrts
#define SER_TXD  ((bus_params_t *) bus->params)->ser_txd
#define SER_NCTS ((bus_params_t *) bus->params)->ser_ncts

/**
 * bus->driver->(*new_bus)
 *
 */
static bus_t *
jopcyc_bus_new( chain_t *chain, char *cmd_params[] )
{
	bus_t *bus;
	part_t *part;
	int failed = 0;
	component_t *comp;

	bus = calloc( 1, sizeof (bus_t) );
	if (!bus)
		return NULL;

	bus->driver = &jopcyc_bus;
	bus->params = calloc( 1, sizeof (bus_params_t) );
	if (!bus->params) {
		free( bus );
		return NULL;
	}

	CHAIN = chain;
	PART = part = chain->parts->parts[chain->active_part];

	/*
	 * Setup RAMA
	 */
	comp = COMP_RAMA;
	comp->ctype = RAM;
	comp->cname = "RAMA";

	failed |= generic_bus_attach_sig( part, &(A[ 0]), "IO64"  );
	failed |= generic_bus_attach_sig( part, &(A[ 1]), "IO66"  );
	failed |= generic_bus_attach_sig( part, &(A[ 2]), "IO68"  );
	failed |= generic_bus_attach_sig( part, &(A[ 3]), "IO74"  );
	failed |= generic_bus_attach_sig( part, &(A[ 4]), "IO76"  );
	failed |= generic_bus_attach_sig( part, &(A[ 5]), "IO107" );
	failed |= generic_bus_attach_sig( part, &(A[ 6]), "IO113" );
	failed |= generic_bus_attach_sig( part, &(A[ 7]), "IO115" );
	failed |= generic_bus_attach_sig( part, &(A[ 8]), "IO117" );
	failed |= generic_bus_attach_sig( part, &(A[ 9]), "IO119" );
	failed |= generic_bus_attach_sig( part, &(A[10]), "IO118" );
	failed |= generic_bus_attach_sig( part, &(A[11]), "IO116" );
	failed |= generic_bus_attach_sig( part, &(A[12]), "IO114" );
	failed |= generic_bus_attach_sig( part, &(A[13]), "IO108" );
	failed |= generic_bus_attach_sig( part, &(A[14]), "IO106" );
	failed |= generic_bus_attach_sig( part, &(A[15]), "IO67"  );
	failed |= generic_bus_attach_sig( part, &(A[16]), "IO65"  );
	failed |= generic_bus_attach_sig( part, &(A[17]), "IO63"  );
	A[18] = NULL;

	failed |= generic_bus_attach_sig( part, &(D[ 0]), "IO82"  );
	failed |= generic_bus_attach_sig( part, &(D[ 1]), "IO84"  );
	failed |= generic_bus_attach_sig( part, &(D[ 2]), "IO86"  );
	failed |= generic_bus_attach_sig( part, &(D[ 3]), "IO88"  );
	failed |= generic_bus_attach_sig( part, &(D[ 4]), "IO94"  );
	failed |= generic_bus_attach_sig( part, &(D[ 5]), "IO98"  );
	failed |= generic_bus_attach_sig( part, &(D[ 6]), "IO100" );
	failed |= generic_bus_attach_sig( part, &(D[ 7]), "IO104" );
	failed |= generic_bus_attach_sig( part, &(D[ 8]), "IO101" );
	failed |= generic_bus_attach_sig( part, &(D[ 9]), "IO99"  );
	failed |= generic_bus_attach_sig( part, &(D[10]), "IO95"  );
	failed |= generic_bus_attach_sig( part, &(D[11]), "IO93"  );
	failed |= generic_bus_attach_sig( part, &(D[12]), "IO87"  );
	failed |= generic_bus_attach_sig( part, &(D[13]), "IO85"  );
	failed |= generic_bus_attach_sig( part, &(D[14]), "IO83"  );
	failed |= generic_bus_attach_sig( part, &(D[15]), "IO79"  );

	failed |= generic_bus_attach_sig( part, &(nCS),   "IO78"  );
	failed |= generic_bus_attach_sig( part, &(nOE),   "IO73"  );
	failed |= generic_bus_attach_sig( part, &(nWE),   "IO105" );
	failed |= generic_bus_attach_sig( part, &(nLB),   "IO77"  );
	failed |= generic_bus_attach_sig( part, &(nUB),   "IO75"  );
	nCS2 = NULL;
	nRDY = NULL;

	/*
	 * Setup RAMB
     */
	comp = COMP_RAMB;
	comp->ctype = RAM;
	comp->cname = "RAMB";

	failed |= generic_bus_attach_sig( part, &(A[ 0]), "IO237" );
	failed |= generic_bus_attach_sig( part, &(A[ 1]), "IO235" );
	failed |= generic_bus_attach_sig( part, &(A[ 2]), "IO233" );
	failed |= generic_bus_attach_sig( part, &(A[ 3]), "IO227" );
	failed |= generic_bus_attach_sig( part, &(A[ 4]), "IO225" );
	failed |= generic_bus_attach_sig( part, &(A[ 5]), "IO194" );
	failed |= generic_bus_attach_sig( part, &(A[ 6]), "IO188" );
	failed |= generic_bus_attach_sig( part, &(A[ 7]), "IO186" );
	failed |= generic_bus_attach_sig( part, &(A[ 8]), "IO184" );
	failed |= generic_bus_attach_sig( part, &(A[ 9]), "IO182" );
	failed |= generic_bus_attach_sig( part, &(A[10]), "IO183" );
	failed |= generic_bus_attach_sig( part, &(A[11]), "IO185" );
	failed |= generic_bus_attach_sig( part, &(A[12]), "IO187" );
	failed |= generic_bus_attach_sig( part, &(A[13]), "IO193" );
	failed |= generic_bus_attach_sig( part, &(A[14]), "IO195" );
	failed |= generic_bus_attach_sig( part, &(A[15]), "IO234" );
	failed |= generic_bus_attach_sig( part, &(A[16]), "IO236" );
	failed |= generic_bus_attach_sig( part, &(A[17]), "IO238" );
	A[18] = NULL;

	failed |= generic_bus_attach_sig( part, &(D[ 0]), "IO219" );
	failed |= generic_bus_attach_sig( part, &(D[ 1]), "IO217" );
	failed |= generic_bus_attach_sig( part, &(D[ 2]), "IO215" );
	failed |= generic_bus_attach_sig( part, &(D[ 3]), "IO213" );
	failed |= generic_bus_attach_sig( part, &(D[ 4]), "IO207" );
	failed |= generic_bus_attach_sig( part, &(D[ 5]), "IO203" );
	failed |= generic_bus_attach_sig( part, &(D[ 6]), "IO201" );
	failed |= generic_bus_attach_sig( part, &(D[ 7]), "IO197" );
	failed |= generic_bus_attach_sig( part, &(D[ 8]), "IO200" );
	failed |= generic_bus_attach_sig( part, &(D[ 9]), "IO202" );
	failed |= generic_bus_attach_sig( part, &(D[10]), "IO206" );
	failed |= generic_bus_attach_sig( part, &(D[11]), "IO208" );
	failed |= generic_bus_attach_sig( part, &(D[12]), "IO214" );
	failed |= generic_bus_attach_sig( part, &(D[13]), "IO216" );
	failed |= generic_bus_attach_sig( part, &(D[14]), "IO218" );
	failed |= generic_bus_attach_sig( part, &(D[15]), "IO222" );

	failed |= generic_bus_attach_sig( part, &(nCS),   "IO223" );
	failed |= generic_bus_attach_sig( part, &(nOE),   "IO228" );
	failed |= generic_bus_attach_sig( part, &(nWE),   "IO196" );
	failed |= generic_bus_attach_sig( part, &(nLB),   "IO224" );
	failed |= generic_bus_attach_sig( part, &(nUB),   "IO226" );
	nCS2 = NULL;
	nRDY = NULL;

	/*
	 * Setup FLASH
	 */
	comp = COMP_FLASH;
	comp->ctype = FLASH;
	comp->cname = "FLASH";

	failed |= generic_bus_attach_sig( part, &(A[ 0]), "IO47"  );
	failed |= generic_bus_attach_sig( part, &(A[ 1]), "IO48"  );
	failed |= generic_bus_attach_sig( part, &(A[ 2]), "IO49"  );
	failed |= generic_bus_attach_sig( part, &(A[ 3]), "IO50"  );
	failed |= generic_bus_attach_sig( part, &(A[ 4]), "IO125" );
	failed |= generic_bus_attach_sig( part, &(A[ 5]), "IO127" );
	failed |= generic_bus_attach_sig( part, &(A[ 6]), "IO131" );
	failed |= generic_bus_attach_sig( part, &(A[ 7]), "IO133" );
	failed |= generic_bus_attach_sig( part, &(A[ 8]), "IO158" );
	failed |= generic_bus_attach_sig( part, &(A[ 9]), "IO16"  );
	failed |= generic_bus_attach_sig( part, &(A[10]), "IO20"  );
	failed |= generic_bus_attach_sig( part, &(A[11]), "IO14"  );
	failed |= generic_bus_attach_sig( part, &(A[12]), "IO135" );
	failed |= generic_bus_attach_sig( part, &(A[13]), "IO156" );
	failed |= generic_bus_attach_sig( part, &(A[14]), "IO144" );
	failed |= generic_bus_attach_sig( part, &(A[15]), "IO137" );
	failed |= generic_bus_attach_sig( part, &(A[16]), "IO139" );
	failed |= generic_bus_attach_sig( part, &(A[17]), "IO143" );
	failed |= generic_bus_attach_sig( part, &(A[18]), "IO141" );

	failed |= generic_bus_attach_sig( part, &(D[ 0]), "IO46"  );
	failed |= generic_bus_attach_sig( part, &(D[ 1]), "IO45"  );
	failed |= generic_bus_attach_sig( part, &(D[ 2]), "IO44"  );
	failed |= generic_bus_attach_sig( part, &(D[ 3]), "IO165" );
	failed |= generic_bus_attach_sig( part, &(D[ 4]), "IO164" );
	failed |= generic_bus_attach_sig( part, &(D[ 5]), "IO17"  );
	failed |= generic_bus_attach_sig( part, &(D[ 6]), "IO18"  );
	failed |= generic_bus_attach_sig( part, &(D[ 7]), "IO19"  );
	D[ 8] = NULL;
	D[ 9] = NULL;
	D[10] = NULL;
	D[11] = NULL;
	D[12] = NULL;
	D[13] = NULL;
	D[14] = NULL;
	D[15] = NULL;

	failed |= generic_bus_attach_sig( part, &(nWE),   "IO15"  );
	failed |= generic_bus_attach_sig( part, &(nOE),   "IO24"  );
	failed |= generic_bus_attach_sig( part, &(nCS),   "IO37"  );
	failed |= generic_bus_attach_sig( part, &(nCS2),  "IO23"  );

	/* CLK1 is not observable :-(
	failed |= generic_bus_attach_sig( part, &(nRDY),  "CLK1"  );
	*/
	nRDY = NULL;

	nLB = NULL;
	nUB = NULL;

	/*
	 * Setup Serial Port
	 */
	failed |= generic_bus_attach_sig( part, &(SER_RXD),  "CLK2" );
	failed |= generic_bus_attach_sig( part, &(SER_NRTS), "IO177" );
	failed |= generic_bus_attach_sig( part, &(SER_TXD),  "IO178"  );
	failed |= generic_bus_attach_sig( part, &(SER_NCTS), "CLK0"  );

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
jopcyc_bus_printinfo( bus_t *bus )
{
	int i;

	for (i = 0; i < CHAIN->parts->len; i++)
		if (PART == CHAIN->parts->parts[i])
			break;
	printf( _("JOP.design Cyclone Board compatible bus driver via BSR (JTAG part No. %d)\n"), i );
}

static void
setup_address( bus_t *bus, uint32_t a, component_t *comp )
{
	int i;
	part_t *p = PART;
	int addr_width;

	LAST_ADDR = a;

	switch (comp->ctype) {
		case RAM:
			addr_width = RAM_ADDR_WIDTH;
			/* address a is a byte address so it is transferred into
			   a word address here */
			a >>= 1;
			break;
		case FLASH:
			addr_width = FLASH_ADDR_WIDTH;
			break;
		default:
			addr_width = 0;
			break;
	}

	for (i = 0; i < addr_width; i++)
		part_set_signal( p, A[i], 1, (a >> i) & 1 );
}

static int
detect_data_width( component_t *comp )
{
	int width;

	switch (comp->ctype) {
		case RAM:
			width = RAM_DATA_WIDTH;
			break;
		case FLASH:
			width = FLASH_DATA_WIDTH;
			break;
		default:
			width = 0;
			break;
	}

	return width;
}

static void
set_data_in( bus_t *bus, component_t *comp )
{
	int i;
	part_t *p = PART;
	int width;

	width = detect_data_width( comp );

	for (i = 0; i < width; i++)
		part_set_signal( p, D[i], 0, 0 );
}

static void
setup_data( bus_t *bus, uint32_t d, component_t *comp )
{
	int i;
	part_t *p = PART;
	int width;

	width = detect_data_width( comp );

	for (i = 0; i < width; i++)
		part_set_signal( p, D[i], 1, (d >> i) & 1 );
}

/**
 * bus->driver->(*prepare)
 *
 */
static void
jopcyc_bus_prepare( bus_t *bus )
{
	part_t *p = PART;
	chain_t *chain = CHAIN;
	component_t *comp;

	/* Preload update registers
	   See AN039, "Guidelines for IEEE Std. 1149.1 Boundary Scan Testing */

	part_set_instruction( p, "SAMPLE/PRELOAD" );
	chain_shift_instructions( chain );

	/* RAMA */
	comp = COMP_RAMA;
	set_data_in( bus, comp );
	part_set_signal( p, nCS, 1, 1 );
	part_set_signal( p, nWE, 1, 1 );
	part_set_signal( p, nOE, 1, 1 );
	part_set_signal( p, nLB, 1, 1 );
	part_set_signal( p, nUB, 1, 1 );

	/* RAMB */
	comp = COMP_RAMB;
	set_data_in( bus, comp );
	part_set_signal( p, nCS, 1, 1 );
	part_set_signal( p, nWE, 1, 1 );
	part_set_signal( p, nOE, 1, 1 );
	part_set_signal( p, nLB, 1, 1 );
	part_set_signal( p, nUB, 1, 1 );

	/* FLASH */
	comp = COMP_FLASH;
	set_data_in( bus, comp );
	part_set_signal( p, nCS,  1, 1 );
	part_set_signal( p, nWE,  1, 1 );
	part_set_signal( p, nOE,  1, 1 );
	part_set_signal( p, nCS2, 1, 1 );
	part_set_signal( p, nRDY, 0, 0 );

	/* Serial Port */
	part_set_signal( p, SER_RXD,  0, 0 );
	part_set_signal( p, SER_NRTS, 1, 1 );
	part_set_signal( p, SER_TXD,  1, 1 );
	part_set_signal( p, SER_NCTS, 0, 0 );

	chain_shift_data_registers( chain, 0 );

	part_set_instruction( p, "EXTEST" );
	chain_shift_instructions( chain );
}

static int
comp_bus_area( bus_t *bus, uint32_t adr, bus_area_t *area, component_t **comp )
{
	if (adr < RAMB_START) {
		area->description = "RAMA Component";
		area->start  = RAMA_START;
		area->length = RAM_LENGTH;
		area->width  = RAM_DATA_WIDTH;
		*comp        = COMP_RAMA;
	} else if (adr < FLASH_START) {
		area->description = "RAMB Component";
		area->start  = RAMB_START;
		area->length = RAM_LENGTH;
		area->width  = RAM_DATA_WIDTH;
		*comp        = COMP_RAMB;
	} else if (adr < FLASH_START + FLASH_LENGTH) {
		area->description = "FLASH Component";
		area->start  = FLASH_START;
		area->length = FLASH_LENGTH;
		area->width  = FLASH_DATA_WIDTH;
		*comp        = COMP_FLASH;
	} else {
		area->description = "Dummy";
		area->start  = 2 * RAM_LENGTH + FLASH_LENGTH;
		area->length = UINT64_C(0x100000000);
		area->width  = 0;
		*comp        = NULL;
	}

	return 0;
}

/**
 * bus->driver->(*area)
 *
 */
static int
jopcyc_bus_area( bus_t *bus, uint32_t adr, bus_area_t *area )
{
	component_t *comp;

	return comp_bus_area( bus, adr, area, &comp );
}

/**
 * bus->driver->(*read_start)
 *
 */
static void
jopcyc_bus_read_start( bus_t *bus, uint32_t adr )
{
	part_t *p = PART;
	chain_t *chain = CHAIN;
	bus_area_t area;
	component_t *comp;

	comp_bus_area( bus, adr, &area, &comp );
	if (!comp) {
		printf( _("Address out of range\n") );
		LAST_ADDR = adr;
		return;
	}

	part_set_signal( p, nCS, 1, 0 );
	part_set_signal( p, nWE, 1, 1 );
	part_set_signal( p, nOE, 1, 0 );
	if (comp->ctype == RAM) {
		part_set_signal( p, nLB, 1, 0 );
		part_set_signal( p, nUB, 1, 0 );
	}

	setup_address( bus, adr, comp );
	set_data_in( bus, comp );

	chain_shift_data_registers( chain, 0 );
}

/**
 * bus->driver->(*read_next)
 *
 */
static uint32_t
jopcyc_bus_read_next( bus_t *bus, uint32_t adr )
{
	part_t *p = PART;
	chain_t *chain = CHAIN;
	int i;
	uint32_t d = 0;
	bus_area_t area;
	component_t *comp;

	comp_bus_area( bus, adr, &area, &comp );
	if (!comp) {
		printf( _("Address out of range\n") );
		LAST_ADDR = adr;
		return 0;
	}

	setup_address( bus, adr, comp );
	chain_shift_data_registers( chain, 1 );

	for (i = 0; i < area.width; i++)
		d |= (uint32_t) (part_get_signal( p, D[i] ) << i);

	return d;
}

/**
 * bus->driver->(*read_end)
 *
 */
static uint32_t
jopcyc_bus_read_end( bus_t *bus )
{
	part_t *p = PART;
	chain_t *chain = CHAIN;
	int i;
	uint32_t d = 0;
	bus_area_t area;
	component_t *comp;

	/* use last address of access to determine component */
	comp_bus_area( bus, LAST_ADDR, &area, &comp );
	if (!comp) {
		printf( _("Address out of range\n") );
		return 0;
	}

	part_set_signal( p, nCS, 1, 1 );
	part_set_signal( p, nOE, 1, 1 );
	if (comp->ctype == RAM) {
		part_set_signal( p, nLB, 1, 1 );
		part_set_signal( p, nUB, 1, 1 );
	}
	chain_shift_data_registers( chain, 1 );

	for (i = 0; i < area.width; i++)
		d |= (uint32_t) (part_get_signal( p, D[i] ) << i);

	return d;
}

/**
 * bus->driver->(*write)
 *
 */
static void
jopcyc_bus_write( bus_t *bus, uint32_t adr, uint32_t data )
{
	part_t *p = PART;
	chain_t *chain = CHAIN;
	bus_area_t area;
	component_t *comp;

	comp_bus_area( bus, adr, &area, &comp );
	if (!comp) {
		printf( _("Address out of range\n") );
		return;
	}

	part_set_signal( p, nCS, 1, 0 );
	part_set_signal( p, nWE, 1, 1 );
	part_set_signal( p, nOE, 1, 1 );
	if (comp->ctype == RAM) {
		part_set_signal( p, nLB, 1, 0 );
		part_set_signal( p, nUB, 1, 0 );
	}

	setup_address( bus, adr, comp );
	setup_data( bus, data, comp );

	chain_shift_data_registers( chain, 0 );

	part_set_signal( p, nWE, 1, 0 );
	chain_shift_data_registers( chain, 0 );
	part_set_signal( p, nWE, 1, 1 );
	part_set_signal( p, nCS, 1, 1 );
	if (comp->ctype == RAM) {
		part_set_signal( p, nLB, 1, 1 );
		part_set_signal( p, nUB, 1, 1 );
	}
	chain_shift_data_registers( chain, 0 );
}

const bus_driver_t jopcyc_bus = {
	"jopcyc",
	N_("JOP.design Cyclone Board compatible bus driver via BSR"),
	jopcyc_bus_new,
	generic_bus_free,
	jopcyc_bus_printinfo,
	jopcyc_bus_prepare,
	jopcyc_bus_area,
	jopcyc_bus_read_start,
	jopcyc_bus_read_next,
	jopcyc_bus_read_end,
	generic_bus_read,
	jopcyc_bus_write,
	NULL
};


/*
 Local Variables:
 mode:C
 tab-width:2
 indent-tabs-mode:t
 End:
*/
