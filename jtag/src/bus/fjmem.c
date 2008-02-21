/*
 * $Id$
 *
 * Bus driver for the FPGA JTAG memory (fjmem) design.
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
 * Written by Arnim Laeuger <arniml@users.sourceforge.net>, 2008.
 *
 */

#include "sysdep.h"

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <part.h>
#include <bus.h>
#include <chain.h>
#include <jtag.h>
#include <buses.h>
#include <cmd.h>
#include <tap.h>


#undef DEBUG

#define FJMEM_INST_NAME "FJMEM_INST"
#define FJMEM_REG_NAME  "FJMEM_REG"
#define FJMEM_MAX_REG_LEN 2048

struct block_param {
	struct block_param *next;
	uint16_t num;
	uint32_t start, end;
	uint16_t addr_width;
	uint16_t data_width;
	uint8_t  ashift;
};
typedef struct block_param block_param_t;

struct block_desc {
	uint16_t       reg_len;
	uint16_t       instr_pos;
	uint16_t       block_pos;
	uint16_t       block_len;
	uint16_t       addr_pos;
	uint16_t       addr_len;
	uint16_t       data_pos;
	uint16_t       data_len;
	block_param_t *blocks;
};
typedef struct block_desc block_desc_t;

typedef struct {
	chain_t       *chain;
	part_t        *part;
	uint32_t       last_addr;
	data_register *fjmem_reg;
	block_desc_t   block_desc;
} bus_params_t;


#define CHAIN      ((bus_params_t *) bus->params)->chain
#define PART       ((bus_params_t *) bus->params)->part
#define LAST_ADDR  ((bus_params_t *) bus->params)->last_addr
#define FJMEM_REG  ((bus_params_t *) bus->params)->fjmem_reg
#define BLOCK_DESC ((bus_params_t *) bus->params)->block_desc

static void
setup_address( bus_t *bus, uint32_t a, block_param_t *block )
{
	data_register *dr = FJMEM_REG;
	block_desc_t *bd = &(BLOCK_DESC);
	int idx;
	uint16_t num = block->num;

	LAST_ADDR = a;

	/* correct address for > 8 bit data widths */
	a >>= block->ashift;

	/* set block number */
	for (idx = 0; idx < bd->block_len; idx++) {
		dr->in->data[bd->block_pos + idx] = num & 1;
		num >>= 1;
	}

	/* set address */
	for (idx = 0; idx < block->addr_width; idx++) {
		dr->in->data[bd->addr_pos + idx] = a & 1;
		a >>= 1;
	}
}


static void
setup_data( bus_t *bus, uint32_t d, block_param_t *block )
{
	data_register *dr = FJMEM_REG;
	block_desc_t *bd = &(BLOCK_DESC);
	int idx;

	/* set data */
	for (idx = 0; idx < block->data_width; idx++) {
		dr->in->data[bd->data_pos + idx] = d & 1;
		d >>= 1;
	}
}


static int block_bus_area( bus_t *bus, uint32_t adr, bus_area_t *area, block_param_t **bl_match );


/* ***************************************************************************
 * fjmem_bus_printinfo
 * ***************************************************************************/
static void
fjmem_bus_printinfo( bus_t *bus )
{
	int i;

	for (i = 0; i < CHAIN->parts->len; i++)
		if (PART == CHAIN->parts->parts[i])
			break;
	printf( _("fjmem FPGA bus driver via USER register (JTAG part No. %d)\n"), i );
}


/* ***************************************************************************
 * fjmem_bus_prepare
 * ***************************************************************************/
static void
fjmem_bus_prepare( bus_t *bus )
{
	part_t *p = PART;
	chain_t *chain = CHAIN;

	/* ensure FJMEM_INST is active */
	part_set_instruction( p, FJMEM_INST_NAME );
	chain_shift_instructions( chain );
}


/* ***************************************************************************
 * fjmem_bus_read_start
 * ***************************************************************************/
static void
fjmem_bus_read_start( bus_t *bus, uint32_t adr )
{
	chain_t *chain = CHAIN;
	block_desc_t *bd = &(BLOCK_DESC);
	data_register *dr = FJMEM_REG;
	bus_area_t area;
	block_param_t *block;

	block_bus_area( bus, adr, &area, &block );
	if (!block) {
		printf( _("Address out of range\n") );
		LAST_ADDR = adr;
		return;
	}

	setup_address( bus, adr, block );

	/* select read instruction */
	dr->in->data[bd->instr_pos+0] = 1;
	dr->in->data[bd->instr_pos+1] = 0;
	dr->in->data[bd->instr_pos+2] = 0;

	chain_shift_data_registers( chain, 0 );
}


/* ***************************************************************************
 * fjmem_bus_read_next
 * ***************************************************************************/
static uint32_t
fjmem_bus_read_next( bus_t *bus, uint32_t adr )
{
	chain_t *chain = CHAIN;
	block_desc_t *bd = &(BLOCK_DESC);
	data_register *dr = FJMEM_REG;
	uint32_t d;
	bus_area_t area;
	block_param_t *block;
	int idx;

	block_bus_area( bus, adr, &area, &block );
	if (!block) {
		printf( _("Address out of range\n") );
		LAST_ADDR = adr;
		return 0;
	}

	setup_address( bus, adr, block );
	chain_shift_data_registers( chain, 1 );

	/* extract data from TDO stream */
	d = 0;
	for (idx = 0; idx < block->data_width; idx++)
		if (dr->out->data[bd->data_pos + idx])
			d |= 1 << idx;

	return d;
}


/* ***************************************************************************
 * fjmem_bus_read_end
 * ***************************************************************************/
static uint32_t
fjmem_bus_read_end( bus_t *bus )
{
	chain_t *chain = CHAIN;
	block_desc_t *bd = &(BLOCK_DESC);
	data_register *dr = FJMEM_REG;
	uint32_t d;
	bus_area_t area;
	block_param_t *block;
	int idx;

	block_bus_area( bus, LAST_ADDR, &area, &block );
	if (!block) {
		printf( _("Address out of range\n") );
		return 0;
	}

	/* prepare idle instruction to disable any spurious unintentional reads */
	dr->in->data[bd->instr_pos+0] = 0;
	dr->in->data[bd->instr_pos+1] = 0;
	dr->in->data[bd->instr_pos+2] = 0;

	chain_shift_data_registers( chain, 1 );

	/* extract data from TDO stream */
	d = 0;
	for (idx = 0; idx < block->data_width; idx++)
		if (dr->out->data[bd->data_pos + idx])
			d |= 1 << idx;

	return d;
}


/* ***************************************************************************
 * fjmem_bus_read
 * ***************************************************************************/
static uint32_t
fjmem_bus_read( bus_t *bus, uint32_t adr )
{
	fjmem_bus_read_start( bus, adr );
	return fjmem_bus_read_end( bus );
}


/* ***************************************************************************
 * fjmem_bus_write
 * ***************************************************************************/
static void
fjmem_bus_write( bus_t *bus, uint32_t adr, uint32_t data )
{
	chain_t *chain = CHAIN;
	block_desc_t *bd = &(BLOCK_DESC);
	data_register *dr = FJMEM_REG;
	bus_area_t area;
	block_param_t *block;

	block_bus_area( bus, adr, &area, &block );
	if (!block) {
		printf( _("Address out of range\n") );
		return;
	}

	setup_address( bus, adr, block );
	setup_data( bus, data, block );

	/* select write instruction */
	dr->in->data[bd->instr_pos+0] = 0;
	dr->in->data[bd->instr_pos+1] = 1;
	dr->in->data[bd->instr_pos+2] = 0;

	chain_shift_data_registers( chain, 0 );
}


/* ***************************************************************************
 * jopcyc_bus_area
 * ***************************************************************************/

static int
block_bus_area( bus_t *bus, uint32_t adr, bus_area_t *area, block_param_t **bl_match )
{
	block_param_t *bl = BLOCK_DESC.blocks;
	uint32_t prev_start;

	*bl_match = NULL;

	/* run through all detected/queried blocks and check if adr falls into
	   one of their ranges */
	prev_start = 0;
	while (bl) {
		if ((bl->start <= adr) && (bl->end >= adr)) {
			/* adr lies inside a matching block range */
			area->description = NULL;
			area->start  = bl->start;
			area->length = bl->end - bl->start + 1;
			area->width  = bl->data_width;
			*bl_match    = bl;
			prev_start   = area->start;
		} else if (((prev_start > adr) || (prev_start == 0)) && (bl->end < adr)) {
			/* a gap between blocks */
			area->description = "Dummy";
			area->start  = bl->end + 1;
			area->length = prev_start > 0 ? prev_start - (bl->end+1) : UINT64_C(0x100000000);
			area->width  = 0;
			*bl_match    = NULL;
			prev_start   = area->start;
		} else
			prev_start   = bl->start;

		bl = bl->next;
	}

	return 0;
}


static int
fjmem_bus_area( bus_t *bus, uint32_t adr, bus_area_t *area )
{
	block_param_t *bl;

	return block_bus_area( bus, adr, area, &bl );
}


static void
fjmem_free_blocks( block_param_t *bl )
{
	if (bl) {
		fjmem_free_blocks( bl->next );
		free( bl );
	}
}

static void
fjmem_bus_free( bus_t *bus )
{
	data_register *dr = FJMEM_REG;

	/* fill all fields with '0'
	   -> prepare idle instruction for next startup/detect */
	part_set_instruction( PART, FJMEM_INST_NAME );
	chain_shift_instructions( CHAIN );

	register_fill( dr->in, 0 );
	chain_shift_data_registers( CHAIN, 0 );

	fjmem_free_blocks( BLOCK_DESC.blocks );
	BLOCK_DESC.blocks = NULL;
	free( bus->params );
	free( bus );
}

static int
fjmem_detect_reg_len( chain_t *chain, part_t *part, char *opcode )
{
	data_register *dr;
	instruction *i;
	int l, fjmem_reg_len;
	char *tdo_bit;

	/* build register FJMEM_REG with length of 1 bit*/
	dr = data_register_alloc( FJMEM_REG_NAME, 1 );
	if (!dr) {
		printf( _("out of memory\n") );
		return 0;
	}

	dr->next = part->data_registers;
	part->data_registers = dr;

	/* build instruction FJMEM_INST with code given by command line parameter
	   that maps to FJMEM_REG */
	if (strlen( opcode ) != part->instruction_length) {
		printf( _("invalid instruction length\n") );
		return 0;
	}
	i = instruction_alloc( FJMEM_INST_NAME, part->instruction_length, opcode );
	if (!i) {
		printf( _("out of memory\n") );
		return 0;
	}
	i->next = part->instructions;
	part->instructions = i;
	i->data_register = dr;

	/* force jtag reset on all parts of the chain
	   -> they're in BYPASS mode now */
	chain_set_trst( chain, 0 );
	chain_set_trst( chain, 1 );
	tap_reset( chain );

	/* flood all BYPASS registers with 0 for the following detection */
	register_fill( dr->in, 0);
	tap_capture_dr( chain );
	for (l = 0; l < chain->parts->len; l++)
		tap_shift_register( chain, dr->in, NULL, EXITMODE_SHIFT );
	/* shift once more and return to idle state */
	tap_shift_register( chain, dr->in, NULL, EXITMODE_IDLE );

	/* set the FJMEM_INST instruction and activate it */
	part_set_instruction( part, FJMEM_INST_NAME );
	chain_shift_instructions( chain );

	/* now detect the register length of FJMEM_REG:
	   shift 1s through the data register until they appear at TDO
	   NB: We don't shift only through the FJMEM_REG but also through the
	       registers of all other parts in the chain. They're set to
	       BYPASS hopefully. */
	fjmem_reg_len = 0;
	register_fill( dr->in,  1);
	register_fill( dr->out, 0);
	tdo_bit = dr->out->data;

	tap_capture_dr( chain );
	/* read current TDO and then shift once */
	tap_shift_register( chain, dr->in, dr->out, EXITMODE_SHIFT );
	register_get_string( dr->out );
	while ((tdo_bit[0] == 0) && (fjmem_reg_len < FJMEM_MAX_REG_LEN)) {
		/* read current TDO and then shift once */
		tap_shift_register( chain, dr->in, dr->out, EXITMODE_SHIFT );
		tdo_bit = dr->out->data;
		fjmem_reg_len++;
	}
	/* consider BYPASS register of other parts in the chain */
	fjmem_reg_len -= chain->parts->len - 1;
	/* shift once more and return to idle state */
	tap_shift_register( chain, dr->in, NULL, EXITMODE_IDLE );
#ifdef DEBUG
	printf("FJMEM data register length: %d\n", fjmem_reg_len);
#endif

	return fjmem_reg_len < FJMEM_MAX_REG_LEN ? fjmem_reg_len : -1;
}

static int
fjmem_detect_fields( chain_t *chain, part_t *part, bus_t *bus )
{
	block_desc_t *bd = &(BLOCK_DESC);
	data_register *dr = FJMEM_REG;
	int idx;
#ifdef DEBUG
	const char *reg_string;
#endif

	/* set safe defaults */
	bd->block_len = 0;
	bd->addr_pos  = 0;
	bd->addr_len  = 0;
	bd->data_pos  = 0;
	bd->data_len  = 0;

	/* extend FJMEM_REG to finally detected size */
	if (dr->in)
		free( dr->in );
	if ((dr->in = register_alloc( bd->reg_len )) == NULL) {
		printf( _("out of memory\n") );
		return 0;
	}
	if (dr->out)
		free( dr->out );
	if ((dr->out = register_alloc( bd->reg_len )) == NULL) {
		printf( _("out of memory\n") );
		return 0;
	}

	/* The detect instruction (all-1) has been shifted into FJMEM_REG
	   previously, with the next shift we will read the field marker
	   pattern.
	   Shift in the query for block 0, will be used lateron. */
	register_fill( dr->in, 0 );
	/* enter query instruction: 110 */
	dr->in->data[bd->instr_pos+1] = 1;
	dr->in->data[bd->instr_pos+2] = 1;

	/* shift register */
	chain_shift_data_registers( chain, 1 );

	/* and examine output from field detect */
#ifdef DEBUG
	reg_string = register_get_string( dr->out );
	printf("captured: %s\n", reg_string);
#endif
	/* scan block field */
	idx = bd->block_pos;
	while (dr->out->data[idx] && (idx < dr->out->len))
		idx++;
	bd->block_len = idx - bd->block_pos;
	/* scan address field */
	bd->addr_pos = idx;
	while ((dr->out->data[idx] == 0) && (idx < dr->out->len))
		idx++;
	bd->addr_len = idx - bd->addr_pos;
	/* scan data field */
	bd->data_pos = idx;
	while (dr->out->data[idx] && (idx < dr->out->len))
		idx++;
	bd->data_len = idx - bd->data_pos;

#ifdef DEBUG
	printf("block pos: %d, len: %d\n", bd->block_pos, bd->block_len);
	printf("addr  pos: %d, len: %d\n", bd->addr_pos,  bd->addr_len);
	printf("data  pos: %d, len: %d\n", bd->data_pos,  bd->data_len);
#endif

	if ((bd->block_len > 0) &&
			(bd->addr_len  > 0) &&
			(bd->data_len  > 0))
		return 1;
	else
		return 0;
}

static int
fjmem_query_blocks( chain_t *chain, part_t *part, bus_t *bus )
{
	block_desc_t *bd = &(BLOCK_DESC);
	data_register *dr = FJMEM_REG;
	int max_block_num, block_num;
	int failed = 0;
#ifdef DEBUG
	const char *reg_string;
#endif

	/* the current block number is 0, it has been requested by the previous
	   shift during fjmem_detect_fields */
	max_block_num = (1 << bd->block_len) - 1;
	for (block_num = 0; block_num <= max_block_num; block_num++) {
		int next_block_num = block_num + 1;
		int idx;
		int addr_len, data_len;

		/* prepare the next query before shifting the data register */
		for (idx = 0; idx < bd->block_len; idx++) {
			dr->in->data[bd->block_pos + idx] = next_block_num & 1;
			next_block_num >>= 1;
		}
		chain_shift_data_registers( chain, 1 );

		/* and examine output from block query */
#ifdef DEBUG
		reg_string = register_get_string( dr->out );
		printf("captured: %s\n", reg_string);
#endif

		/* extract address field length */
		for (addr_len = 0; addr_len < bd->addr_len; addr_len++)
			if (dr->out->data[bd->addr_pos + addr_len] == 0)
				break;

		/* extract data field length */
		for (data_len = 0; data_len < bd->data_len; data_len++)
			if (dr->out->data[bd->data_pos + data_len] == 0)
				break;

		/* it's a valid block only if address field and data field are
		   both larger than 0 */
		if ((addr_len > 0) && (data_len > 0)) {
			block_param_t *bl;
			int nbytes;

			if ((bl = (block_param_t *)malloc( sizeof( block_param_t ) )) == NULL) {
				printf( _("out of memory\n") );
				failed |= 1;
				break;
			}

			bl->next       = bd->blocks;
			bl->num        = block_num;
			bl->addr_width = addr_len;
			bl->data_width = data_len;
			bd->blocks     = bl;

			/* determine address shift, depends on data width */
			nbytes = data_len / 8;
			if (data_len % 8)
				nbytes++;

			bl->ashift     = 0;
			while (nbytes != 1) {
				bl->ashift++;
				nbytes >>= 1;
			}

			/* determine start address of this block */
			if (bl->next == NULL)
				bl->start = 0;
			else {
				if ((bl->addr_width << bl->ashift) <= (bl->next->addr_width << bl->next->ashift)) {
					bl->start = bl->next->start + (1 << (bl->next->addr_width + bl->next->ashift));
				} else {
					uint32_t mask = 1 << (bl->addr_width + bl->ashift);
					bl->start = bl->next->start & ~(mask - 1);
					bl->start += mask;
				}
			}
			/* and fill in end address of this block */
			bl->end = bl->start + (1 << (bl->addr_width + bl->ashift)) - 1;

#ifdef DEBUG
			printf("block # %d\n", block_num);
			printf(" start 0x%08x\n", bl->start);
			printf(" end   0x%08x\n", bl->end);
			printf(" addr len %d\n", bl->addr_width);
			printf(" data len %d\n", bl->data_width);
#endif
		}
	}

	return failed ? 0 : 1;
}

static bus_t *
fjmem_bus_new( char *params[] )
{
	bus_t *bus = NULL;
	int failed = 0;
	char param[16], value[16];
	part_t *part;
	int fjmem_reg_len;

	if (!chain || !chain->parts || chain->parts->len <= chain->active_part || chain->active_part < 0)
		return NULL;

	if (chain->active_part >= chain->parts->len) {
		printf( _("%s: no active part\n"), "fjmem" );
		return NULL;
	}
	part = chain->parts->parts[chain->active_part];

	if ( cmd_params(params) != 3 ) {
		printf( _("Parameter for instruction code missing.\n") );
		return NULL;
	}
	sscanf( params[2], "%[^=]%*c%s", param, value );
	if (strcasecmp( param, "opcode" ) == 0) {
		block_desc_t *bd;

		fjmem_reg_len = fjmem_detect_reg_len( chain, part, value );
		if (fjmem_reg_len <= 0)
			return NULL;

		bus = malloc( sizeof (bus_t) );
		if (!bus)
			return NULL;

		bus->driver = &fjmem_bus;
		bus->params = malloc( sizeof (bus_params_t) );
		if (!bus->params) {
			free( bus );
			return NULL;
		}

		CHAIN = chain;
		PART = chain->parts->parts[chain->active_part];
		FJMEM_REG = part_find_data_register( PART, FJMEM_REG_NAME );
		bd = &(BLOCK_DESC);
		bd->blocks = NULL;
		bd->reg_len = fjmem_reg_len;
		bd->instr_pos = 0;
		bd->block_pos = bd->instr_pos + 4; /* 3 bits for instruction field, 1 bit ack field */

		if (fjmem_detect_fields( chain, part, bus ) > 0) {
			if (fjmem_query_blocks( chain, part, bus ) > 0) {
			} else
				failed |= 1;
		} else
			failed |= 1;

		if (failed) {
			free( bus->params );
			free( bus );
			return NULL;
		}
	}

	return bus;
}

const bus_driver_t fjmem_bus = {
	"fjmem",
	N_("FPGA JTAG memory bus driver via USER register, requires parameters:\n"
	   "           opcode=<USERx OPCODE>"),
	fjmem_bus_new,
	fjmem_bus_free,
	fjmem_bus_printinfo,
	fjmem_bus_prepare,
	fjmem_bus_area,
	fjmem_bus_read_start,
	fjmem_bus_read_next,
	fjmem_bus_read_end,
	fjmem_bus_read,
	fjmem_bus_write,
	NULL
};


/*
 Local Variables:
 mode:C
 tab-width:2
 indent-tabs-mode:t
 End:
*/
