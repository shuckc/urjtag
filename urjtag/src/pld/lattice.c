/*
 * Driver for Lattice FPGAs
 *
 * Copyright (C) 2012, Chris Shucksmith
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
 * Written by Chris Shucksmith <chris@shucksmith.co.uk>, 2012
 *
 */

#include <sysdep.h>

#include <string.h>
#include <stdlib.h>

#include <urjtag/tap.h>
#include <urjtag/part.h>
#include <urjtag/chain.h>
#include <urjtag/tap_state.h>
#include <urjtag/tap_register.h>
#include <urjtag/data_register.h>
#include <urjtag/part_instruction.h>
#include <urjtag/pld.h>
#include <urjtag/bitops.h>
#include "lattice.h"

static int
lat_set_ir_and_shift (urj_chain_t *chain, urj_part_t *part, char *iname)
{
    urj_part_set_instruction (part, iname);
    if (part->active_instruction == NULL)
    {
        urj_error_set (URJ_ERROR_PLD, "unknown instruction '%s'", iname);
        return URJ_STATUS_FAIL;
    }
    urj_tap_chain_shift_instructions (chain);

    return URJ_STATUS_OK;
}

static int
lat_instruction_resize_dr (urj_part_t *part, const char *ir_name,
        const char *dr_name, int dr_len)
{
    urj_data_register_t *d;
    urj_part_instruction_t *i;

    i = urj_part_find_instruction (part, ir_name);

    if (i == NULL)
    {
        urj_error_set (URJ_ERROR_PLD, "unknown instruction '%s'", ir_name);
        return URJ_STATUS_FAIL;
    }

    d = urj_part_find_data_register (part, dr_name);

    if (d == NULL)
    {
        d = urj_part_data_register_alloc (dr_name, dr_len);
        d->next = part->data_registers;
        part->data_registers = d;
    }
    else if (d->in->len != dr_len)
    {
        /* data register length does not match */
        urj_part_data_register_realloc (d, dr_len);
    }

    i->data_register = d;

    return URJ_STATUS_OK;
}

static int
lat_print_status_ecp3 (urj_pld_t *pld)
{
    urj_chain_t *chain = pld->chain;
    urj_part_t *part = pld->part;
    urj_tap_register_t *r;


    /* set all devices in bypass mode */
    urj_tap_reset_bypass (chain);

    if (lat_set_ir_and_shift (chain, part, "LSCC_READ_STATUS") != URJ_STATUS_OK)
    {
         urj_log (URJ_LOG_LEVEL_ERROR, _("unable to perform LSCC_READ_STATUS instruction\n"));
         return URJ_STATUS_FAIL;
    }

    urj_tap_chain_shift_data_registers (chain, 1);
    r = part->active_instruction->data_register->out;

    /** for ECP3 we need to check 32 bit status is 00020000 after masking 00060007
     * m  0000 0000 0000 0110 0000 0000 0000 0111                   
     * =  xxxx xxxx xxxx x01x xxxx xxxx xxxx x000 
     * Function of these bits currently unknown.
     * Bit 17 is not set when programming fails, most likely DONE pin
     */        
    urj_log (URJ_LOG_LEVEL_NORMAL, _("Status register\n"));
    urj_log (URJ_LOG_LEVEL_NORMAL, _("\tSTATUS 0 (=0)     %d\n"), r->data[0]);
    urj_log (URJ_LOG_LEVEL_NORMAL, _("\tSTATUS 1 (=0)     %d\n"), r->data[1]);
    urj_log (URJ_LOG_LEVEL_NORMAL, _("\tSTATUS 2 (=0)     %d\n"), r->data[2]);
    urj_log (URJ_LOG_LEVEL_NORMAL, _("\tSTATUS 17 (=1)    %d DONE\n"), r->data[17]);
    urj_log (URJ_LOG_LEVEL_NORMAL, _("\tSTATUS 18 (=0)    %d\n"), r->data[18]);

    return URJ_STATUS_OK;
}

static int
lat_configure (urj_pld_t *pld, FILE *bit_file)
{
    urj_chain_t *chain = pld->chain;
    urj_part_t *part = pld->part;
    urj_part_instruction_t *i;
    lat_bitstream_t *bs;
    uint32_t u, p;
    uint32_t usercode = 0xFFFFFFFF;
    int dr_len;
    char *dr_data;
    int status = URJ_STATUS_FAIL;

    /* set all devices in bypass mode */
    urj_tap_reset_bypass (chain);

    bs = lat_bitstream_alloc ();
    if (bs == NULL)
    {
        goto fail;
    }

    /* parse bit file */
    if (lat_bitstream_load_bit (bit_file, bs) != URJ_STATUS_OK)
    {
        urj_error_set (URJ_ERROR_PLD, _("Invalid bitfile"));
        goto fail_free;
    }

    urj_log (URJ_LOG_LEVEL_NORMAL, _("Bitstream information:\n"));
    
    lat_header_t *elem = bs->header;
    while (elem != NULL)
    {
        urj_log (URJ_LOG_LEVEL_NORMAL, _("\t%s\n"), elem->text);        
        elem = elem->next;        
    }

    urj_log (URJ_LOG_LEVEL_NORMAL, _("Erasing...\n"));
    
    if (lat_set_ir_and_shift (chain, part, "LSCC_REFRESH") != URJ_STATUS_OK)
    {
        goto fail_free;
    }
    urj_tap_chain_defer_clock (chain, 0, 0, 5);

    if (lat_set_ir_and_shift (chain, part, "ISC_ENABLE") != URJ_STATUS_OK)
    {
        goto fail_free;
    }
    urj_tap_chain_defer_clock (chain, 0, 0, 5);

    if (usercode != 0)
    {

        i = urj_part_find_instruction (part, "ISC_PROGRAM_USERCODE");
        dr_data = i->data_register->in->data;
        for (u = 0; u < 32; u++) dr_data[u] = 1;

        if (lat_set_ir_and_shift (chain, part, "ISC_PROGRAM_USERCODE") != URJ_STATUS_OK)
        {
            urj_log (URJ_LOG_LEVEL_ERROR, _("lsc: unable to select ISC_PROGRAM_USERCODE instruction\n"));
            goto fail_free;
        }

        /* push usercode value out through dr */
        urj_tap_chain_shift_data_registers (chain, 0);

        /* read USERCODE to verify */
        if (lat_set_ir_and_shift (chain, part, "ISC_READ_USERCODE") != URJ_STATUS_OK)
        {
            urj_log (URJ_LOG_LEVEL_ERROR, _("lsc: unable to select ISC_READ_USERCODE instruction\n"));
            goto fail_free;
        }

        urj_tap_chain_shift_data_registers (chain, 1);
        status = ( urj_tap_register_match (part->active_instruction->data_register->out, "11111111111111111111111111111111") ) ? URJ_STATUS_OK : URJ_STATUS_FAIL;
        if (status == URJ_STATUS_FAIL)
        {
            urj_log (URJ_LOG_LEVEL_ERROR, _("lsc: part USERCODE did not read back OK on status pins\n"));
        }

    }

    if (lat_set_ir_and_shift (chain, part, "ISC_ERASE") != URJ_STATUS_OK)
    {
        goto fail_free;
    }
    // wait 5 TCK (max 2ms)
    urj_tap_chain_defer_clock (chain, 0, 0, 5);
    
    if (lat_set_ir_and_shift (chain, part, "LSCC_RESET_ADDRESS") != URJ_STATUS_OK)
    {
       urj_log (URJ_LOG_LEVEL_ERROR, _("lsc: unable to perform RESET_ADDRESS instruction\n"));
        goto fail_free;
    }
    urj_tap_chain_defer_clock (chain, 0, 0, 5);

    /* SVF does another READ usercode here, so copy that (not sure if required sequence or not)
     * read USERCODE to verify
     */
    if (lat_set_ir_and_shift (chain, part, "ISC_READ_USERCODE") != URJ_STATUS_OK)
    {
        urj_log (URJ_LOG_LEVEL_ERROR, _("lsc: unable to select ISC_READ_USERCODE instruction (2)\n"));
        goto fail_free;
    }

    urj_tap_chain_shift_data_registers (chain, 1);

    urj_log (URJ_LOG_LEVEL_NORMAL, _("Programming...\n"));
    /* copy data into shift register. The data must be prefixed by LSCC_PADDING_SZ bytes of 0xff, reversed bytewise
     * and then bit-swapped msb/lsb.
     */
    dr_len = (bs->length + LSCC_PADDING_SZ) * 8 ;
    urj_log (URJ_LOG_LEVEL_NORMAL, _("bitstream burst dr-length %d \n"), dr_len);

    if (lat_set_ir_and_shift (chain, part, "LSCC_RESET_ADDRESS") != URJ_STATUS_OK)
    {
       urj_log (URJ_LOG_LEVEL_ERROR, _("lsc: unable to perform RESET_ADDRESS instruction\n"));
        goto fail_free;
    }
    urj_tap_chain_defer_clock (chain, 0, 0, 5);

    if (lat_instruction_resize_dr (part, "LSCC_BITSTREAM_BURST", "BITST", dr_len) != URJ_STATUS_OK)
        goto fail_free;

    i = urj_part_find_instruction (part, "LSCC_BITSTREAM_BURST");

    dr_data = i->data_register->in->data;
    
    for (u = bs->length, p=0; u > 0; u--, p++)
    {
        /* flip bits and reverse */
        dr_data[8*p+0] = (bs->data[u] & 0x80) ? 1 : 0;
        dr_data[8*p+1] = (bs->data[u] & 0x40) ? 1 : 0;
        dr_data[8*p+2] = (bs->data[u] & 0x20) ? 1 : 0;
        dr_data[8*p+3] = (bs->data[u] & 0x10) ? 1 : 0;
        dr_data[8*p+4] = (bs->data[u] & 0x08) ? 1 : 0;
        dr_data[8*p+5] = (bs->data[u] & 0x04) ? 1 : 0;
        dr_data[8*p+6] = (bs->data[u] & 0x02) ? 1 : 0;
        dr_data[8*p+7] = (bs->data[u] & 0x01) ? 1 : 0;
    }
    // append padding
    for (p = bs->length; p < (bs->length + LSCC_PADDING_SZ); p++)
    {
        dr_data[8*p+0] = 1;
        dr_data[8*p+1] = 1;
        dr_data[8*p+2] = 1;
        dr_data[8*p+3] = 1;
        dr_data[8*p+4] = 1;
        dr_data[8*p+5] = 1;
        dr_data[8*p+6] = 1;
        dr_data[8*p+7] = 1;
    }

    if (lat_set_ir_and_shift (chain, part, "LSCC_BITSTREAM_BURST") != URJ_STATUS_OK)
    {
        urj_log (URJ_LOG_LEVEL_ERROR, _("lsc: unable to perform BITSTREAM_BURST instruction\n"));
        goto fail_free;
    }

    /* push entire bitstream out through dr */
    urj_tap_chain_shift_data_registers (chain, 0);

    urj_tap_chain_defer_clock (chain, 0, 0, 256);

    /* SVF does another READ usercode after programming, repeating
     */
    if (lat_set_ir_and_shift (chain, part, "ISC_READ_USERCODE") != URJ_STATUS_OK)
    {
        urj_log (URJ_LOG_LEVEL_ERROR, _("lsc: unable to select ISC_READ_USERCODE instruction (2)\n"));
        goto fail_free;
    }

    urj_tap_chain_shift_data_registers (chain, 1);

    urj_tap_chain_defer_clock (chain, 0, 0, 100);

    urj_log (URJ_LOG_LEVEL_NORMAL, _("Resuming to user mode...\n"));

    if (lat_set_ir_and_shift (chain, part, "ISC_DISABLE") != URJ_STATUS_OK)
    {
        urj_log (URJ_LOG_LEVEL_ERROR, _("lsc: unable to perform ISC_DISABLE instruction\n"));
        goto fail_free;
    }

    if (lat_set_ir_and_shift (chain, part, "BYPASS") != URJ_STATUS_OK)
    {
        urj_log (URJ_LOG_LEVEL_ERROR, _("lsc: unable to perform BYPASS instruction\n"));
        goto fail_free;
    }

    /* check status bits */   

    if (lat_set_ir_and_shift (chain, part, "LSCC_READ_STATUS") != URJ_STATUS_OK)
    {
        urj_log (URJ_LOG_LEVEL_ERROR, _("lsc: unable to perform READ_STATUS instruction\n"));
        goto fail_free;
    }

    /* for ECP3 we check 32 bit status is 00020000 after masking 00060007
     * m  0000 0000 0000 0110 0000 0000 0000 0111                   
     * =  xxxx xxxx xxxx x01x xxxx xxxx xxxx x000 
     */
    urj_tap_chain_shift_data_registers (chain, 1);
    status = ( urj_tap_register_match (part->active_instruction->data_register->out, "?????????????01??????????????000") ) ? URJ_STATUS_OK : URJ_STATUS_FAIL;
    if (status == URJ_STATUS_FAIL)
    {
       urj_log (URJ_LOG_LEVEL_ERROR, _("lsc: part did not read back OK on status pins\n"));
    }

    urj_tap_reset_bypass (chain);

    urj_tap_chain_flush (chain);

 fail_free:
    lat_bitstream_free (bs);
 fail:
    return status;
}

static int
lat_reconfigure (urj_pld_t *pld)
{
    urj_chain_t *chain = pld->chain;
    urj_part_t *part = pld->part;

    urj_tap_reset_bypass (chain);

    if (lat_set_ir_and_shift (chain, part, "LSCC_REFRESH") != URJ_STATUS_OK)
        return URJ_STATUS_FAIL;
    
    urj_tap_reset (chain);
    urj_tap_chain_flush (chain);

    return URJ_STATUS_OK;
}

static int
lat_detect_ecp3 (urj_pld_t *pld)
{
    urj_part_t *part = pld->part;
    uint32_t idcode;
    
    idcode = urj_tap_register_get_value (part->id);
    
    switch (idcode)
    {
        case LATTICE_PART_ECP3_35EA:
            break;
        default:
            return URJ_STATUS_FAIL;
    }

    /* install custom instructions */

    if (!urj_part_find_instruction(part, "LSCC_READ_STATUS"))
    {
       urj_part_data_register_define(part, "STATUS", 32 );
       urj_part_instruction_define (part, "LSCC_READ_STATUS", "01010011", "STATUS");
    }

    /* Cause recondfiguration - see "Configuration Modes" 
     * in LatticeECP3 sysCONFIG Usage Guide 
     */
    if (!urj_part_find_instruction(part, "LSCC_REFRESH"))
       urj_part_instruction_define (part, "LSCC_REFRESH", "00100011", "BYPASS");

    if (!urj_part_find_instruction(part, "LSCC_RESET_ADDRESS"))
       urj_part_instruction_define (part, "LSCC_RESET_ADDRESS", "00100001", "BYPASS");

    if (!urj_part_find_instruction(part, "LSCC_BITSTREAM_BURST")) 
    {
        urj_part_data_register_define(part, "BITST", 32 ); /* resize later */
        urj_part_instruction_define (part, "LSCC_BITSTREAM_BURST", "00000010", "BITST");
    }
 
    /* exit programming mode */
    if (!urj_part_find_instruction(part, "ISC_DISABLE"))
       urj_part_instruction_define (part, "ISC_DISABLE", "00011110", "BYPASS");
    
    /* enable program mode */
    if (!urj_part_find_instruction(part, "ISC_ENABLE"))
       urj_part_instruction_define (part, "ISC_ENABLE", "00010101", "BYPASS");
    
    if (!urj_part_find_instruction(part, "ISC_PROGRAM_USERCODE")) 
       urj_part_instruction_define (part, "ISC_PROGRAM_USERCODE", "00011010", "BYPASS");
    
    if (!urj_part_find_instruction(part, "ISC_ERASE")) 
       urj_part_instruction_define (part, "ISC_ERASE", "00000011", "BYPASS");
    
    return URJ_STATUS_OK;
}

const urj_pld_driver_t urj_pld_lat_ecp3_driver = {
    .name = N_("Latice ECP3 Family"),
    .detect = lat_detect_ecp3,
    .print_status = lat_print_status_ecp3,
    .configure = lat_configure,
    .reconfigure = lat_reconfigure,
    .read_register = NULL,
    .write_register = NULL,
    .register_width = 2,
};

