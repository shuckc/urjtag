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

#include <sysdep.h>

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <urjtag/log.h>
#include <urjtag/part.h>
#include <urjtag/bus.h>
#include <urjtag/chain.h>
#include <urjtag/cmd.h>
#include <urjtag/tap.h>
#include <urjtag/data_register.h>
#include <urjtag/tap_register.h>
#include <urjtag/part_instruction.h>

#include "buses.h"
#include "generic_bus.h"

#undef DEBUG

#define FJMEM_INST_NAME "FJMEM_INST"
#define FJMEM_REG_NAME  "FJMEM_REG"
#define FJMEM_MAX_REG_LEN 2048

struct block_param
{
    struct block_param *next;
    uint16_t num;
    uint32_t start, end;
    uint16_t addr_width;
    uint16_t data_width;
    uint8_t ashift;
};
typedef struct block_param block_param_t;

struct block_desc
{
    uint16_t reg_len;
    uint16_t instr_pos;
    uint16_t block_pos;
    uint16_t block_len;
    uint16_t addr_pos;
    uint16_t addr_len;
    uint16_t data_pos;
    uint16_t data_len;
    block_param_t *blocks;
};
typedef struct block_desc block_desc_t;

typedef struct
{
    uint32_t last_addr;
    urj_data_register_t *fjmem_reg;
    block_desc_t block_desc;
} bus_params_t;

#define LAST_ADDR  ((bus_params_t *) bus->params)->last_addr
#define FJMEM_REG  ((bus_params_t *) bus->params)->fjmem_reg
#define BLOCK_DESC ((bus_params_t *) bus->params)->block_desc

static int
fjmem_detect_reg_len (urj_chain_t *chain, urj_part_t *part, const char *opcode,
                      int len)
{
    urj_data_register_t *dr;
    urj_part_instruction_t *i;
    int l, fjmem_reg_len;
    char *tdo_bit;

    /* build register FJMEM_REG with length of 1 bit */
    dr = urj_part_data_register_alloc (FJMEM_REG_NAME, 1);
    if (!dr)
        // retain error state
        return 0;

    dr->next = part->data_registers;
    part->data_registers = dr;

    /* build instruction FJMEM_INST with code given by command line parameter
       that maps to FJMEM_REG */
    if (strlen (opcode) != part->instruction_length)
    {
        urj_error_set (URJ_ERROR_INVALID, _("invalid instruction length"));
        return 0;
    }
    i = urj_part_instruction_alloc (FJMEM_INST_NAME, part->instruction_length,
                                    opcode);
    if (!i)
        // retain error state
        return 0;
    i->next = part->instructions;
    part->instructions = i;
    i->data_register = dr;

    /* force jtag reset on all parts of the chain
       -> they're in BYPASS mode now */
    urj_tap_chain_set_trst (chain, 0);
    urj_tap_chain_set_trst (chain, 1);
    urj_tap_reset_bypass (chain);

    /* flood all BYPASS registers with 0 for the following detection */
    urj_tap_register_fill (dr->in, 0);
    urj_tap_capture_dr (chain);
    for (l = 0; l < chain->parts->len; l++)
        urj_tap_shift_register (chain, dr->in, NULL,
                                URJ_CHAIN_EXITMODE_SHIFT);
    /* shift once more and return to idle state */
    urj_tap_shift_register (chain, dr->in, NULL, URJ_CHAIN_EXITMODE_IDLE);

    /* set the FJMEM_INST instruction and activate it */
    urj_part_set_instruction (part, FJMEM_INST_NAME);
    urj_tap_chain_shift_instructions (chain);

    /* skip autodetect if register length was already specified */
    if (len)
        return len;

    /* now detect the register length of FJMEM_REG:
       shift 1s through the data register until they appear at TDO
       NB: We don't shift only through the FJMEM_REG but also through the
       registers of all other parts in the chain. They're set to
       BYPASS hopefully. */
    fjmem_reg_len = 0;
    urj_tap_register_fill (dr->in, 1);
    urj_tap_register_fill (dr->out, 0);
    tdo_bit = dr->out->data;

    urj_tap_capture_dr (chain);
    /* read current TDO and then shift once */
    urj_tap_shift_register (chain, dr->in, dr->out, URJ_CHAIN_EXITMODE_SHIFT);
    urj_tap_register_get_string (dr->out);
    while ((tdo_bit[0] == 0) && (fjmem_reg_len < FJMEM_MAX_REG_LEN))
    {
        /* read current TDO and then shift once */
        urj_tap_shift_register (chain, dr->in, dr->out,
                                URJ_CHAIN_EXITMODE_SHIFT);
        tdo_bit = dr->out->data;
        fjmem_reg_len++;
    }
    /* consider BYPASS register of other parts in the chain */
    fjmem_reg_len -= chain->parts->len - 1;
    /* shift once more and return to idle state */
    urj_tap_shift_register (chain, dr->in, NULL, URJ_CHAIN_EXITMODE_IDLE);
    urj_log (URJ_LOG_LEVEL_DEBUG, "FJMEM data register length: %d\n",
             fjmem_reg_len);

    return fjmem_reg_len < FJMEM_MAX_REG_LEN ? fjmem_reg_len : -1;
}

static int
fjmem_detect_fields (urj_chain_t *chain, urj_part_t *part, urj_bus_t *bus)
{
    block_desc_t *bd = &(BLOCK_DESC);
    urj_data_register_t *dr = FJMEM_REG;
    int idx;

    /* set safe defaults */
    bd->block_len = 0;
    bd->addr_pos = 0;
    bd->addr_len = 0;
    bd->data_pos = 0;
    bd->data_len = 0;

    /* extend FJMEM_REG to finally detected size */
    if (dr->in)
        free (dr->in);
    if ((dr->in = urj_tap_register_alloc (bd->reg_len)) == NULL)
        // retain error state
        return 0;
    if (dr->out)
        free (dr->out);
    if ((dr->out = urj_tap_register_alloc (bd->reg_len)) == NULL)
        // retain error state
        return 0;

    /* Shift the detect instruction (all-1) into FJMEM_REG. */
    urj_tap_register_fill (dr->in, 1);
    urj_tap_chain_shift_data_registers (chain, 1);

    /* With the next shift we will read the field marker pattern.
       Shift in the query for block 0, will be used lateron. */
    urj_tap_register_fill (dr->in, 0);
    /* enter query instruction: 110 */
    dr->in->data[bd->instr_pos + 1] = 1;
    dr->in->data[bd->instr_pos + 2] = 1;

    /* shift register */
    urj_tap_chain_shift_data_registers (chain, 1);

    /* and examine output from field detect */
    urj_log (URJ_LOG_LEVEL_DEBUG, "captured: %s\n",
             urj_tap_register_get_string (dr->out));
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

    urj_log (URJ_LOG_LEVEL_DEBUG, "block pos: %d, len: %d\n",
             bd->block_pos, bd->block_len);
    urj_log (URJ_LOG_LEVEL_DEBUG, "addr  pos: %d, len: %d\n",
             bd->addr_pos, bd->addr_len);
    urj_log (URJ_LOG_LEVEL_DEBUG, "data  pos: %d, len: %d\n",
             bd->data_pos, bd->data_len);

    if ((bd->block_len > 0) && (bd->addr_len > 0) && (bd->data_len > 0))
        return 1;
    else
        return 0;
}

static int
fjmem_query_blocks (urj_chain_t *chain, urj_part_t *part, urj_bus_t *bus)
{
    block_desc_t *bd = &(BLOCK_DESC);
    urj_data_register_t *dr = FJMEM_REG;
    int max_block_num, block_num;
    int failed = 0;

    /* the current block number is 0, it has been requested by the previous
       shift during fjmem_detect_fields */
    max_block_num = (1 << bd->block_len) - 1;
    for (block_num = 0; block_num <= max_block_num; block_num++)
    {
        int next_block_num = block_num + 1;
        int idx;
        int addr_len, data_len;

        /* prepare the next query before shifting the data register */
        for (idx = 0; idx < bd->block_len; idx++)
        {
            dr->in->data[bd->block_pos + idx] = next_block_num & 1;
            next_block_num >>= 1;
        }
        urj_tap_chain_shift_data_registers (chain, 1);

        /* and examine output from block query */
        urj_log (URJ_LOG_LEVEL_DEBUG, "captured: %s\n",
                 urj_tap_register_get_string (dr->out));

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
        if ((addr_len > 0) && (data_len > 0))
        {
            block_param_t *bl;
            int nbytes;

            if ((bl = calloc (1, sizeof (block_param_t))) == NULL)
            {
                urj_error_set (URJ_ERROR_OUT_OF_MEMORY, "calloc(%zd,%zd) fails",
                               (size_t) 1, sizeof (urj_bus_t));
                failed |= 1;
                break;
            }

            bl->next = bd->blocks;
            bl->num = block_num;
            bl->addr_width = addr_len;
            bl->data_width = data_len;
            bd->blocks = bl;

            /* determine address shift, depends on data width */
            nbytes = data_len / 8;
            if (data_len % 8)
                nbytes++;

            bl->ashift = 0;
            while (nbytes != 1)
            {
                bl->ashift++;
                nbytes >>= 1;
            }

            /* determine start address of this block */
            if (bl->next == NULL)
                bl->start = 0;
            else
            {
                if ((bl->addr_width << bl->ashift) <=
                    (bl->next->addr_width << bl->next->ashift))
                {
                    bl->start =
                        bl->next->start +
                        (1 << (bl->next->addr_width + bl->next->ashift));
                }
                else
                {
                    uint32_t mask = 1 << (bl->addr_width + bl->ashift);
                    bl->start = bl->next->start & ~(mask - 1);
                    bl->start += mask;
                }
            }
            /* and fill in end address of this block */
            bl->end = bl->start + (1 << (bl->addr_width + bl->ashift)) - 1;

            urj_log (URJ_LOG_LEVEL_DEBUG, "block # %d\n", block_num);
            urj_log (URJ_LOG_LEVEL_DEBUG, " start 0x%08lx\n",
                     (long unsigned) bl->start);
            urj_log (URJ_LOG_LEVEL_DEBUG, " end   0x%08lx\n",
                     (long unsigned) bl->end);
            urj_log (URJ_LOG_LEVEL_DEBUG, " addr len %d\n", bl->addr_width);
            urj_log (URJ_LOG_LEVEL_DEBUG, " data len %d\n", bl->data_width);
        }
    }

    return failed ? 0 : 1;
}

/**
 * bus->driver->(*new_bus)
 *
 */
static urj_bus_t *
fjmem_bus_new (urj_chain_t *chain, const urj_bus_driver_t *driver,
               const urj_param_t *params[])
{
    urj_bus_t *bus = NULL;
    int failed = 0;
    urj_part_t *part;
    const char *opcode = NULL;
    int fjmem_reg_len = 0;
    int idx;

    bus = urj_bus_generic_new (chain, driver, sizeof (bus_params_t));
    if (bus == NULL)
        return NULL;
    part = bus->part;

    /* parse parameters */
    for (idx = 0; params[idx] != NULL; idx++)
    {
        switch (params[idx]->key)
        {
        case URJ_BUS_PARAM_KEY_OPCODE:
            opcode = params[idx]->value.string;
            break;
        case URJ_BUS_PARAM_KEY_LEN:
            fjmem_reg_len = params[idx]->value.lu;
            break;
        default:
            urj_bus_generic_free (bus);
            urj_error_set (URJ_ERROR_SYNTAX, "unrecognized bus parameter '%s'",
                           urj_param_string(&urj_bus_param_list, params[idx]));
            return NULL;
        }
    }

    if (opcode == NULL)
    {
        urj_bus_generic_free (bus);
        urj_error_set (URJ_ERROR_SYNTAX,
                       _("Parameter for instruction opcode missing"));
        return NULL;
    }

    block_desc_t *bd;

    fjmem_reg_len = fjmem_detect_reg_len (chain, part, opcode, fjmem_reg_len);
    if (fjmem_reg_len <= 0)
        return NULL;

    bus->chain = chain;
    // @@@@ RFHH check result
    FJMEM_REG = urj_part_find_data_register (part, FJMEM_REG_NAME);
    bd = &(BLOCK_DESC);
    bd->blocks = NULL;
    bd->reg_len = fjmem_reg_len;
    bd->instr_pos = 0;
    bd->block_pos = bd->instr_pos + 4;      /* 3 bits for instruction field, 1 bit ack field */

    if (fjmem_detect_fields (chain, part, bus) <= 0)
        failed |= 1;
    else if (fjmem_query_blocks (chain, part, bus) <= 0)
        failed |= 1;

    if (failed)
    {
        urj_bus_generic_free (bus);
        return NULL;
    }

    return bus;
}

static void
fjmem_free_blocks (block_param_t *bl)
{
    if (bl)
    {
        fjmem_free_blocks (bl->next);
        free (bl);
    }
}

/**
 * bus->driver->(*free_bus)
 *
 */
static void
fjmem_bus_free (urj_bus_t *bus)
{
    urj_data_register_t *dr = FJMEM_REG;

    /* fill all fields with '0'
       -> prepare idle instruction for next startup/detect */
    urj_part_set_instruction (bus->part, FJMEM_INST_NAME);
    urj_tap_chain_shift_instructions (bus->chain);

    urj_tap_register_fill (dr->in, 0);
    urj_tap_chain_shift_data_registers (bus->chain, 0);

    fjmem_free_blocks (BLOCK_DESC.blocks);
    BLOCK_DESC.blocks = NULL;

    urj_bus_generic_free (bus);
}

/**
 * bus->driver->(*printinfo)
 *
 */
static void
fjmem_bus_printinfo (urj_log_level_t ll, urj_bus_t *bus)
{
    int i;

    for (i = 0; i < bus->chain->parts->len; i++)
        if (bus->part == bus->chain->parts->parts[i])
            break;
    urj_log (ll, _("fjmem FPGA bus driver via USER register (JTAG part No. %d)\n"),
            i);
}

/**
 * bus->driver->(*prepare)
 *
 */
static void
fjmem_bus_prepare (urj_bus_t *bus)
{
    if (!bus->initialized)
        URJ_BUS_INIT (bus);

    /* ensure FJMEM_INST is active */
    urj_part_set_instruction (bus->part, FJMEM_INST_NAME);
    urj_tap_chain_shift_instructions (bus->chain);
}

static int
block_bus_area (urj_bus_t *bus, uint32_t adr, urj_bus_area_t *area,
                block_param_t **bl_match)
{
    block_param_t *bl = BLOCK_DESC.blocks;
    uint32_t prev_start;

    *bl_match = NULL;

    /* run through all detected/queried blocks and check if adr falls into
       one of their ranges */
    prev_start = 0;
    while (bl)
    {
        if ((bl->start <= adr) && (bl->end >= adr))
        {
            /* adr lies inside a matching block range */
            area->description = NULL;
            area->start = bl->start;
            area->length = bl->end - bl->start + 1;
            area->width = bl->data_width;
            *bl_match = bl;
            prev_start = area->start;
        }
        else if (((prev_start > adr) || (prev_start == 0)) && (bl->end < adr))
        {
            /* a gap between blocks */
            area->description = "Dummy";
            area->start = bl->end + 1;
            area->length =
                prev_start >
                0 ? prev_start - (bl->end + 1) : UINT64_C (0x100000000);
            area->width = 0;
            *bl_match = NULL;
            prev_start = area->start;
        }
        else
            prev_start = bl->start;

        bl = bl->next;
    }

    return URJ_STATUS_OK;
}

/**
 * bus->driver->(*area)
 *
 */
static int
fjmem_bus_area (urj_bus_t *bus, uint32_t adr, urj_bus_area_t *area)
{
    block_param_t *bl;

    return block_bus_area (bus, adr, area, &bl);
}

static void
setup_address (urj_bus_t *bus, uint32_t a, block_param_t *block)
{
    urj_data_register_t *dr = FJMEM_REG;
    block_desc_t *bd = &(BLOCK_DESC);
    int idx;
    uint16_t num = block->num;

    LAST_ADDR = a;

    /* correct address for > 8 bit data widths */
    a >>= block->ashift;

    /* set block number */
    for (idx = 0; idx < bd->block_len; idx++)
    {
        dr->in->data[bd->block_pos + idx] = num & 1;
        num >>= 1;
    }

    /* set address */
    for (idx = 0; idx < block->addr_width; idx++)
    {
        dr->in->data[bd->addr_pos + idx] = a & 1;
        a >>= 1;
    }
}

static void
setup_data (urj_bus_t *bus, uint32_t d, block_param_t *block)
{
    urj_data_register_t *dr = FJMEM_REG;
    block_desc_t *bd = &(BLOCK_DESC);
    int idx;

    /* set data */
    for (idx = 0; idx < block->data_width; idx++)
    {
        dr->in->data[bd->data_pos + idx] = d & 1;
        d >>= 1;
    }
}

/**
 * bus->driver->(*read_start)
 *
 */
static int
fjmem_bus_read_start (urj_bus_t *bus, uint32_t adr)
{
    urj_chain_t *chain = bus->chain;
    block_desc_t *bd = &(BLOCK_DESC);
    urj_data_register_t *dr = FJMEM_REG;
    urj_bus_area_t area;
    block_param_t *block;

    block_bus_area (bus, adr, &area, &block);
    if (!block)
    {
        urj_error_set (URJ_ERROR_OUT_OF_BOUNDS, _("Address out of range"));
        LAST_ADDR = adr;
        return URJ_STATUS_FAIL;
    }

    setup_address (bus, adr, block);

    /* select read instruction */
    dr->in->data[bd->instr_pos + 0] = 1;
    dr->in->data[bd->instr_pos + 1] = 0;
    dr->in->data[bd->instr_pos + 2] = 0;

    urj_tap_chain_shift_data_registers (chain, 0);

    return URJ_STATUS_OK;
}

/**
 * bus->driver->(*read_next)
 *
 */
static uint32_t
fjmem_bus_read_next (urj_bus_t *bus, uint32_t adr)
{
    urj_chain_t *chain = bus->chain;
    block_desc_t *bd = &(BLOCK_DESC);
    urj_data_register_t *dr = FJMEM_REG;
    uint32_t d;
    urj_bus_area_t area;
    block_param_t *block;
    int idx;

    block_bus_area (bus, adr, &area, &block);
    if (!block)
    {
        urj_error_set (URJ_ERROR_OUT_OF_BOUNDS, _("Address out of range"));
        LAST_ADDR = adr;
        return 0;
    }

    setup_address (bus, adr, block);
    urj_tap_chain_shift_data_registers (chain, 1);

    /* extract data from TDO stream */
    d = 0;
    for (idx = 0; idx < block->data_width; idx++)
        if (dr->out->data[bd->data_pos + idx])
            d |= 1 << idx;

    return d;
}

/**
 * bus->driver->(*read_end)
 *
 */
static uint32_t
fjmem_bus_read_end (urj_bus_t *bus)
{
    urj_chain_t *chain = bus->chain;
    block_desc_t *bd = &(BLOCK_DESC);
    urj_data_register_t *dr = FJMEM_REG;
    uint32_t d;
    urj_bus_area_t area;
    block_param_t *block;
    int idx;

    block_bus_area (bus, LAST_ADDR, &area, &block);
    if (!block)
    {
        urj_error_set (URJ_ERROR_OUT_OF_BOUNDS, _("Address out of range"));
        return 0;
    }

    /* prepare idle instruction to disable any spurious unintentional reads */
    dr->in->data[bd->instr_pos + 0] = 0;
    dr->in->data[bd->instr_pos + 1] = 0;
    dr->in->data[bd->instr_pos + 2] = 0;

    urj_tap_chain_shift_data_registers (chain, 1);

    /* extract data from TDO stream */
    d = 0;
    for (idx = 0; idx < block->data_width; idx++)
        if (dr->out->data[bd->data_pos + idx])
            d |= 1 << idx;

    return d;
}

/**
 * bus->driver->(*write)
 *
 */
static void
fjmem_bus_write (urj_bus_t *bus, uint32_t adr, uint32_t data)
{
    urj_chain_t *chain = bus->chain;
    block_desc_t *bd = &(BLOCK_DESC);
    urj_data_register_t *dr = FJMEM_REG;
    urj_bus_area_t area;
    block_param_t *block;

    block_bus_area (bus, adr, &area, &block);
    if (!block)
    {
        urj_error_set (URJ_ERROR_OUT_OF_BOUNDS, _("Address out of range"));
        return;
    }

    setup_address (bus, adr, block);
    setup_data (bus, data, block);

    /* select write instruction */
    dr->in->data[bd->instr_pos + 0] = 0;
    dr->in->data[bd->instr_pos + 1] = 1;
    dr->in->data[bd->instr_pos + 2] = 0;

    urj_tap_chain_shift_data_registers (chain, 0);
}

const urj_bus_driver_t urj_bus_fjmem_bus = {
    "fjmem",
    N_("FPGA JTAG memory bus driver via USER register, requires parameters:\n"
       "           opcode=<USERx OPCODE> [len=<FJMEM REG LEN>]"),
    fjmem_bus_new,
    fjmem_bus_free,
    fjmem_bus_printinfo,
    fjmem_bus_prepare,
    fjmem_bus_area,
    fjmem_bus_read_start,
    fjmem_bus_read_next,
    fjmem_bus_read_end,
    urj_bus_generic_read,
    urj_bus_generic_write_start,
    fjmem_bus_write,
    urj_bus_generic_no_init,
    urj_bus_generic_no_enable,
    urj_bus_generic_no_disable,
    URJ_BUS_TYPE_PARALLEL,
};


/*
 Local Variables:
 mode:C
 tab-width:2
 indent-tabs-mode:t
 End:
*/
