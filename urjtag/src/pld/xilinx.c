/*
 * $Id$
 *
 * Driver for Xilinx FPGAs
 *
 * Copyright (C) 2010, Michael Walle
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
 * Written by Michael Walle <michael@walle.cc>, 2010
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
#include "xilinx.h"

static int
xlx_set_ir_and_shift (urj_chain_t *chain, urj_part_t *part, char *iname)
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
xlx_set_dr_and_shift (urj_chain_t *chain, urj_part_t *part,
        uint64_t value, int exitmode)
{
    if (part->active_instruction == NULL)
        return URJ_STATUS_FAIL;

    urj_tap_register_t *r = part->active_instruction->data_register->in;
    urj_tap_register_set_value (r, value);
    urj_tap_defer_shift_register (chain, r, NULL, exitmode);

    return URJ_STATUS_OK;
}

static int
xlx_instruction_resize_dr (urj_part_t *part, const char *ir_name,
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
xlx_write_register_xc3s (urj_pld_t *pld, uint32_t reg, uint32_t value)
{
    urj_chain_t *chain = pld->chain;
    urj_part_t *part = pld->part;

    if (value & 0xffff0000)
    {
        urj_log (URJ_LOG_LEVEL_WARNING,
                 _("Only 16 bit values supported. Truncating value."));

        value &= 0xffff;
    }

    /* use the same data register as they have the same length */
    if (xlx_instruction_resize_dr (part, "CFG_IN", "CFG_DR", 16)
            != URJ_STATUS_OK)
        return URJ_STATUS_FAIL;
    if (xlx_instruction_resize_dr (part, "CFG_OUT", "CFG_DR", 16)
            != URJ_STATUS_OK)
        return URJ_STATUS_FAIL;

    /* set all devices in bypass mode */
    urj_tap_reset_bypass (chain);

    if (xlx_set_ir_and_shift (chain, part, "CFG_IN") != URJ_STATUS_OK)
        return URJ_STATUS_FAIL;

    urj_tap_capture_dr (chain);
    /* sync */
    xlx_set_dr_and_shift (chain, part, flip16 (0xffff),
                          URJ_CHAIN_EXITMODE_SHIFT);
    xlx_set_dr_and_shift (chain, part, flip16 (0xaa99),
                          URJ_CHAIN_EXITMODE_SHIFT);
    /* noop */
    xlx_set_dr_and_shift (chain, part, flip16 (0x2000),
                          URJ_CHAIN_EXITMODE_SHIFT);
    /* type 2 packet (word count = 1) */
    reg = 0x3001 | ((reg & 0x3f) << 5);
    xlx_set_dr_and_shift (chain, part, flip16 (reg),
                          URJ_CHAIN_EXITMODE_SHIFT);
    xlx_set_dr_and_shift (chain, part, flip16 (value),
                          URJ_CHAIN_EXITMODE_SHIFT);
    /* noop */
    xlx_set_dr_and_shift (chain, part, flip16 (0x2000),
                          URJ_CHAIN_EXITMODE_SHIFT);
    xlx_set_dr_and_shift (chain, part, flip16 (0x2000),
                          URJ_CHAIN_EXITMODE_IDLE);

    urj_tap_chain_flush (chain);

    return URJ_STATUS_OK;
}

static int
xlx_write_register_xc4v (urj_pld_t *pld, uint32_t reg, uint32_t value)
{
    urj_chain_t *chain = pld->chain;
    urj_part_t *part = pld->part;

    /* use the same data register as they have the same length */
    if (xlx_instruction_resize_dr (part, "CFG_IN", "CFG_DR", 32)
            != URJ_STATUS_OK)
        return URJ_STATUS_FAIL;
    if (xlx_instruction_resize_dr (part, "CFG_OUT", "CFG_DR", 32)
            != URJ_STATUS_OK)
        return URJ_STATUS_FAIL;

    /* set all devices in bypass mode */
    urj_tap_reset_bypass (chain);

    if (xlx_set_ir_and_shift (chain, part, "CFG_IN") != URJ_STATUS_OK)
        return URJ_STATUS_FAIL;

    urj_tap_capture_dr (chain);
    /* sync */
    xlx_set_dr_and_shift (chain, part, flip32 (0xffffffff),
                          URJ_CHAIN_EXITMODE_SHIFT);
    xlx_set_dr_and_shift (chain, part, flip32 (0xaa995566),
                          URJ_CHAIN_EXITMODE_SHIFT);
    /* noop */
    xlx_set_dr_and_shift (chain, part, flip32 (0x20000000),
                          URJ_CHAIN_EXITMODE_SHIFT);
    /* type 2 packet (word count = 1) */
    reg = 0x30000001 | ((reg & 0x1f) << 13);
    xlx_set_dr_and_shift (chain, part, flip32 (reg),
                          URJ_CHAIN_EXITMODE_SHIFT);
    xlx_set_dr_and_shift (chain, part, flip32 (value),
                          URJ_CHAIN_EXITMODE_SHIFT);
    /* noop */
    xlx_set_dr_and_shift (chain, part, flip32 (0x20000000),
                          URJ_CHAIN_EXITMODE_SHIFT);
    xlx_set_dr_and_shift (chain, part, flip32 (0x20000000),
                          URJ_CHAIN_EXITMODE_IDLE);

    urj_tap_chain_flush (chain);

    return URJ_STATUS_OK;
}

static int
xlx_write_register_xc6s (urj_pld_t *pld, uint32_t reg, uint32_t value)
{
    urj_chain_t *chain = pld->chain;
    urj_part_t *part = pld->part;

    if (value & 0xffff0000)
    {
        urj_log (URJ_LOG_LEVEL_WARNING,
                _("Only 16 bit values supported. Truncating value."));

        value &= 0xffff;
    }

    /* use the same data register as they have the same length */
    if (xlx_instruction_resize_dr (part, "CFG_IN", "CFG_DR", 16)
            != URJ_STATUS_OK)
        return URJ_STATUS_FAIL;
    if (xlx_instruction_resize_dr (part, "CFG_OUT", "CFG_DR", 16)
            != URJ_STATUS_OK)
        return URJ_STATUS_FAIL;

    /* set all devices in bypass mode */
    urj_tap_reset_bypass (chain);

    if (xlx_set_ir_and_shift (chain, part, "CFG_IN") != URJ_STATUS_OK)
        return URJ_STATUS_FAIL;

    urj_tap_capture_dr (chain);
    /* sync */
    xlx_set_dr_and_shift (chain, part, flip16 (0xaa99),
                          URJ_CHAIN_EXITMODE_SHIFT);
    xlx_set_dr_and_shift (chain, part, flip16 (0x5566),
                          URJ_CHAIN_EXITMODE_SHIFT);
    /* noop */
    xlx_set_dr_and_shift (chain, part, flip16 (0x2000),
                          URJ_CHAIN_EXITMODE_SHIFT);
    /* type 2 packet (word count = 1) */
    reg = 0x3001 | ((reg & 0x3f) << 5);
    xlx_set_dr_and_shift (chain, part, flip16 (reg),
                          URJ_CHAIN_EXITMODE_SHIFT);
    xlx_set_dr_and_shift (chain, part, flip16 (value),
                          URJ_CHAIN_EXITMODE_SHIFT);
    /* noop */
    xlx_set_dr_and_shift (chain, part, flip16 (0x2000),
                          URJ_CHAIN_EXITMODE_SHIFT);
    xlx_set_dr_and_shift (chain, part, flip16 (0x2000),
                          URJ_CHAIN_EXITMODE_SHIFT);
    xlx_set_dr_and_shift (chain, part, flip16 (0x2000),
                          URJ_CHAIN_EXITMODE_SHIFT);
    xlx_set_dr_and_shift (chain, part, flip16 (0x2000),
                          URJ_CHAIN_EXITMODE_IDLE);

    urj_tap_chain_flush (chain);

    return URJ_STATUS_OK;
}

static int
xlx_read_register_xc3s (urj_pld_t *pld, uint32_t reg, uint32_t *value)
{
    urj_chain_t *chain = pld->chain;
    urj_part_t *part = pld->part;
    urj_tap_register_t *r;

    /* use the same data register as they have the same length */
    if (xlx_instruction_resize_dr (part, "CFG_IN", "CFG_DR", 16)
            != URJ_STATUS_OK)
        return URJ_STATUS_FAIL;
    if (xlx_instruction_resize_dr (part, "CFG_OUT", "CFG_DR", 16)
            != URJ_STATUS_OK)
        return URJ_STATUS_FAIL;

    /* set all devices in bypass mode */
    urj_tap_reset_bypass (chain);

    if (xlx_set_ir_and_shift (chain, part, "CFG_IN") != URJ_STATUS_OK)
        return URJ_STATUS_FAIL;

    urj_tap_capture_dr (chain);
    /* sync */
    xlx_set_dr_and_shift (chain, part, flip16 (0xffff),
                          URJ_CHAIN_EXITMODE_SHIFT);
    xlx_set_dr_and_shift (chain, part, flip16 (0xaa99),
                          URJ_CHAIN_EXITMODE_SHIFT);
    /* noop */
    xlx_set_dr_and_shift (chain, part, flip16 (0x2000),
                          URJ_CHAIN_EXITMODE_SHIFT);
    /* type 1 packet (word count = 1) */
    reg = 0x2801 | ((reg & 0x3f) << 5);
    xlx_set_dr_and_shift (chain, part, flip16 (reg),
                          URJ_CHAIN_EXITMODE_SHIFT);
    /* noop */
    xlx_set_dr_and_shift (chain, part, flip16 (0x2000),
                          URJ_CHAIN_EXITMODE_SHIFT);
    xlx_set_dr_and_shift (chain, part, flip16 (0x2000),
                          URJ_CHAIN_EXITMODE_IDLE);

    if (xlx_set_ir_and_shift (chain, part, "CFG_OUT") != URJ_STATUS_OK)
        return URJ_STATUS_FAIL;

    urj_tap_chain_shift_data_registers (chain, 1);

    r = part->active_instruction->data_register->out;

    *value = flip16 (urj_tap_register_get_value (r));

    return URJ_STATUS_OK;
}

static int
xlx_read_register_xc4v (urj_pld_t *pld, uint32_t reg, uint32_t *value)
{
    urj_chain_t *chain = pld->chain;
    urj_part_t *part = pld->part;
    urj_tap_register_t *r;

    /* use the same data register as they have the same length */
    if (xlx_instruction_resize_dr (part, "CFG_IN", "CFG_DR", 32)
            != URJ_STATUS_OK)
        return URJ_STATUS_FAIL;
    if (xlx_instruction_resize_dr (part, "CFG_OUT", "CFG_DR", 32)
            != URJ_STATUS_OK)
        return URJ_STATUS_FAIL;

    /* set all devices in bypass mode */
    urj_tap_reset_bypass (chain);

    if (xlx_set_ir_and_shift (chain, part, "CFG_IN") != URJ_STATUS_OK)
        return URJ_STATUS_FAIL;

    urj_tap_capture_dr (chain);
    /* sync */
    xlx_set_dr_and_shift (chain, part, flip32 (0xffffffff),
                          URJ_CHAIN_EXITMODE_SHIFT);
    xlx_set_dr_and_shift (chain, part, flip32 (0xaa995566),
                          URJ_CHAIN_EXITMODE_SHIFT);
    /* noop */
    xlx_set_dr_and_shift (chain, part, flip32 (0x20000000),
                          URJ_CHAIN_EXITMODE_SHIFT);
    /* type 1 packet (word count = 1) */
    reg = 0x28000001 | ((reg & 0x1f) << 13);
    xlx_set_dr_and_shift (chain, part, flip32 (reg),
                          URJ_CHAIN_EXITMODE_SHIFT);
    /* noop */
    xlx_set_dr_and_shift (chain, part, flip32 (0x20000000),
                          URJ_CHAIN_EXITMODE_SHIFT);
    xlx_set_dr_and_shift (chain, part, flip32 (0x20000000),
                          URJ_CHAIN_EXITMODE_IDLE);

    if (xlx_set_ir_and_shift (chain, part, "CFG_OUT") != URJ_STATUS_OK)
        return URJ_STATUS_FAIL;

    urj_tap_chain_shift_data_registers (chain, 1);

    r = part->active_instruction->data_register->out;

    *value = flip32 (urj_tap_register_get_value (r));

    return URJ_STATUS_OK;
}

static int
xlx_read_register_xc6s (urj_pld_t *pld, uint32_t reg, uint32_t *value)
{
    urj_chain_t *chain = pld->chain;
    urj_part_t *part = pld->part;
    urj_tap_register_t *r;

    /* use the same data register as they have the same length */
    if (xlx_instruction_resize_dr (part, "CFG_IN", "CFG_DR", 16)
            != URJ_STATUS_OK)
        return URJ_STATUS_FAIL;
    if (xlx_instruction_resize_dr (part, "CFG_OUT", "CFG_DR", 16)
            != URJ_STATUS_OK)
        return URJ_STATUS_FAIL;

    /* set all devices in bypass mode */
    urj_tap_reset_bypass (chain);

    if (xlx_set_ir_and_shift (chain, part, "CFG_IN") != URJ_STATUS_OK)
        return URJ_STATUS_FAIL;

    urj_tap_capture_dr (chain);
    /* sync */
    xlx_set_dr_and_shift (chain, part, flip16 (0xffff),
                          URJ_CHAIN_EXITMODE_SHIFT);
    xlx_set_dr_and_shift (chain, part, flip16 (0xaa99),
                          URJ_CHAIN_EXITMODE_SHIFT);
    xlx_set_dr_and_shift (chain, part, flip16 (0x5566),
                          URJ_CHAIN_EXITMODE_SHIFT);
    /* noop */
    xlx_set_dr_and_shift (chain, part, flip16 (0x2000),
                          URJ_CHAIN_EXITMODE_SHIFT);
    /* type 1 packet (word count = 1) */
    reg = 0x2801 | ((reg & 0x3f) << 5);
    xlx_set_dr_and_shift (chain, part, flip16 (reg),
                          URJ_CHAIN_EXITMODE_SHIFT);
    /* noop */
    xlx_set_dr_and_shift (chain, part, flip16 (0x2000),
                          URJ_CHAIN_EXITMODE_SHIFT);
    xlx_set_dr_and_shift (chain, part, flip16 (0x2000),
                          URJ_CHAIN_EXITMODE_SHIFT);
    xlx_set_dr_and_shift (chain, part, flip16 (0x2000),
                          URJ_CHAIN_EXITMODE_SHIFT);
    xlx_set_dr_and_shift (chain, part, flip16 (0x2000),
                          URJ_CHAIN_EXITMODE_IDLE);

    if (xlx_set_ir_and_shift (chain, part, "CFG_OUT") != URJ_STATUS_OK)
        return URJ_STATUS_FAIL;

    urj_tap_chain_shift_data_registers (chain, 1);

    r = part->active_instruction->data_register->out;

    *value = flip16 (urj_tap_register_get_value (r));

    return URJ_STATUS_OK;
}

static int
xlx_print_status_xc3s (urj_pld_t *pld)
{
    uint32_t status;

    if (xlx_read_register_xc3s (pld, XILINX_XC3S_REG_STAT, &status)
                != URJ_STATUS_OK)
        return URJ_STATUS_FAIL;

    urj_log (URJ_LOG_LEVEL_NORMAL, _("Status register (0x%04x)\n"), status);
    urj_log (URJ_LOG_LEVEL_NORMAL, _("\tSYNC_TIMEOUT %d\n"),
        (status & XC3S_STATUS_SYNC_TIMEOUT) ? 1 : 0);
    urj_log (URJ_LOG_LEVEL_NORMAL, _("\tSEUR_ERR     %d\n"),
        (status & XC3S_STATUS_SEU_ERR) ? 1 : 0);
    urj_log (URJ_LOG_LEVEL_NORMAL, _("\tDONE         %d\n"),
        (status & XC3S_STATUS_DONE) ? 1 : 0);
    urj_log (URJ_LOG_LEVEL_NORMAL, _("\tINIT         %d\n"),
        (status & XC3S_STATUS_INIT) ? 1 : 0);
    urj_log (URJ_LOG_LEVEL_NORMAL, _("\tMODE_M2      %d\n"),
        (status & XC3S_STATUS_MODE_M2) ? 1 : 0);
    urj_log (URJ_LOG_LEVEL_NORMAL, _("\tMODE_M1      %d\n"),
        (status & XC3S_STATUS_MODE_M1) ? 1 : 0);
    urj_log (URJ_LOG_LEVEL_NORMAL, _("\tMODE_M0      %d\n"),
        (status & XC3S_STATUS_MODE_M0) ? 1 : 0);
    urj_log (URJ_LOG_LEVEL_NORMAL, _("\tVSEL_VS2     %d\n"),
        (status & XC3S_STATUS_VSEL_VS2) ? 1 : 0);
    urj_log (URJ_LOG_LEVEL_NORMAL, _("\tVSEL_VS1     %d\n"),
        (status & XC3S_STATUS_VSEL_VS1) ? 1 : 0);
    urj_log (URJ_LOG_LEVEL_NORMAL, _("\tVSEL_VS0     %d\n"),
        (status & XC3S_STATUS_VSEL_VS0) ? 1 : 0);
    urj_log (URJ_LOG_LEVEL_NORMAL, _("\tGHIGH_B      %d\n"),
        (status & XC3S_STATUS_GHIGH_B) ? 1 : 0);
    urj_log (URJ_LOG_LEVEL_NORMAL, _("\tGWE          %d\n"),
        (status & XC3S_STATUS_GWE) ? 1 : 0);
    urj_log (URJ_LOG_LEVEL_NORMAL, _("\tGTS_CFG_B    %d\n"),
        (status & XC3S_STATUS_GTS_CFG_B) ? 1 : 0);
    urj_log (URJ_LOG_LEVEL_NORMAL, _("\tDCM_LOCK     %d\n"),
        (status & XC3S_STATUS_DCM_LOCK) ? 1 : 0);
    urj_log (URJ_LOG_LEVEL_NORMAL, _("\tID_ERROR     %d\n"),
        (status & XC3S_STATUS_ID_ERROR) ? 1 : 0);
    urj_log (URJ_LOG_LEVEL_NORMAL, _("\tCRC_ERROR    %d\n"),
        (status & XC3S_STATUS_CRC_ERROR) ? 1 : 0);

    return URJ_STATUS_OK;
}

static int
xlx_print_status_xc4v (urj_pld_t *pld)
{
    uint32_t status;

    if (xlx_read_register_xc4v (pld, XILINX_XC4V_REG_STAT, &status)
                != URJ_STATUS_OK)
        return URJ_STATUS_FAIL;

    urj_log (URJ_LOG_LEVEL_NORMAL, _("Status register (0x%08x)\n"), status);
    urj_log (URJ_LOG_LEVEL_NORMAL, _("\tXC4V_STATUS_DEC_ERROR     %d\n"),
        (status & XC4V_STATUS_DEC_ERROR) ? 1 : 0);
    urj_log (URJ_LOG_LEVEL_NORMAL, _("\tXC4V_STATUS_ID_ERROR      %d\n"),
        (status & XC4V_STATUS_ID_ERROR) ? 1 : 0);
    urj_log (URJ_LOG_LEVEL_NORMAL, _("\tXC4V_STATUS_DONE          %d\n"),
        (status & XC4V_STATUS_DONE) ? 1 : 0);
    urj_log (URJ_LOG_LEVEL_NORMAL, _("\tXC4V_STATUS_RELEASE_DONE  %d\n"),
        (status & XC4V_STATUS_RELEASE_DONE) ? 1 : 0);
    urj_log (URJ_LOG_LEVEL_NORMAL, _("\tXC4V_STATUS_INIT          %d\n"),
        (status & XC4V_STATUS_INIT) ? 1 : 0);
    urj_log (URJ_LOG_LEVEL_NORMAL, _("\tXC4V_STATUS_INIT_COMPLETE %d\n"),
        (status & XC4V_STATUS_INIT_COMPLETE) ? 1 : 0);
    urj_log (URJ_LOG_LEVEL_NORMAL, _("\tXC4V_STATUS_MODE_M2       %d\n"),
        (status & XC4V_STATUS_MODE_M2) ? 1 : 0);
    urj_log (URJ_LOG_LEVEL_NORMAL, _("\tXC4V_STATUS_MODE_M1       %d\n"),
        (status & XC4V_STATUS_MODE_M1) ? 1 : 0);
    urj_log (URJ_LOG_LEVEL_NORMAL, _("\tXC4V_STATUS_MODE_M0       %d\n"),
        (status & XC4V_STATUS_MODE_M0) ? 1 : 0);
    urj_log (URJ_LOG_LEVEL_NORMAL, _("\tXC4V_STATUS_GHIGH_B       %d\n"),
        (status & XC4V_STATUS_GHIGH_B) ? 1 : 0);
    urj_log (URJ_LOG_LEVEL_NORMAL, _("\tXC4V_STATUS_GWE           %d\n"),
        (status & XC4V_STATUS_GWE) ? 1 : 0);
    urj_log (URJ_LOG_LEVEL_NORMAL, _("\tXC4V_STATUS_GTS_CFG_B     %d\n"),
        (status & XC4V_STATUS_GTS_CFG_B) ? 1 : 0);
    urj_log (URJ_LOG_LEVEL_NORMAL, _("\tXC4V_STATUS_EOS           %d\n"),
        (status & XC4V_STATUS_EOS) ? 1 : 0);
    urj_log (URJ_LOG_LEVEL_NORMAL, _("\tXC4V_STATUS_DCI_MATCH     %d\n"),
        (status & XC4V_STATUS_DCI_MATCH) ? 1 : 0);
    urj_log (URJ_LOG_LEVEL_NORMAL, _("\tXC4V_STATUS_DCM_LOCK      %d\n"),
        (status & XC4V_STATUS_DCM_LOCK) ? 1 : 0);
    urj_log (URJ_LOG_LEVEL_NORMAL, _("\tXC4V_STATUS_PART_SECURED  %d\n"),
        (status & XC4V_STATUS_PART_SECURED) ? 1 : 0);
    urj_log (URJ_LOG_LEVEL_NORMAL, _("\tXC4V_STATUS_CRC_ERROR     %d\n"),
        (status & XC4V_STATUS_CRC_ERROR) ? 1 : 0);

    return URJ_STATUS_OK;
}


static int
xlx_print_status_xc6s (urj_pld_t *pld)
{
    uint32_t status;

    if (xlx_read_register_xc6s (pld, XILINX_XC6S_REG_STAT, &status)
                != URJ_STATUS_OK)
        return URJ_STATUS_FAIL;

    urj_log (URJ_LOG_LEVEL_NORMAL, _("Status register (0x%04x)\n"), status);
    urj_log (URJ_LOG_LEVEL_NORMAL, _("\tSWWD         %d\n"),
        (status & XC6S_STATUS_SWWD) ? 1 : 0);
    urj_log (URJ_LOG_LEVEL_NORMAL, _("\tIN_PWRDN     %d\n"),
        (status & XC6S_STATUS_IN_PWRDN) ? 1 : 0);
    urj_log (URJ_LOG_LEVEL_NORMAL, _("\tDONE         %d\n"),
        (status & XC6S_STATUS_DONE) ? 1 : 0);
    urj_log (URJ_LOG_LEVEL_NORMAL, _("\tINIT_B       %d\n"),
        (status & XC6S_STATUS_INIT_B) ? 1 : 0);
    urj_log (URJ_LOG_LEVEL_NORMAL, _("\tMODE_M1      %d\n"),
        (status & XC6S_STATUS_MODE_M1) ? 1 : 0);
    urj_log (URJ_LOG_LEVEL_NORMAL, _("\tMODE_M0      %d\n"),
        (status & XC6S_STATUS_MODE_M0) ? 1 : 0);
    urj_log (URJ_LOG_LEVEL_NORMAL, _("\tHSWAPEN      %d\n"),
        (status & XC6S_STATUS_HSWAPEN) ? 1 : 0);
    urj_log (URJ_LOG_LEVEL_NORMAL, _("\tPART_SECURED %d\n"),
        (status & XC6S_STATUS_PART_SECURED) ? 1 : 0);
    urj_log (URJ_LOG_LEVEL_NORMAL, _("\tDEC_ERROR    %d\n"),
        (status & XC6S_STATUS_DEC_ERROR) ? 1 : 0);
    urj_log (URJ_LOG_LEVEL_NORMAL, _("\tGHIGH_B      %d\n"),
        (status & XC6S_STATUS_GHIGH_B) ? 1 : 0);
    urj_log (URJ_LOG_LEVEL_NORMAL, _("\tGWE          %d\n"),
        (status & XC6S_STATUS_GWE) ? 1 : 0);
    urj_log (URJ_LOG_LEVEL_NORMAL, _("\tGTS_CFG_B    %d\n"),
        (status & XC6S_STATUS_GTS_CFG_B) ? 1 : 0);
    urj_log (URJ_LOG_LEVEL_NORMAL, _("\tDCM_LOCK     %d\n"),
        (status & XC6S_STATUS_DCM_LOCK) ? 1 : 0);
    urj_log (URJ_LOG_LEVEL_NORMAL, _("\tID_ERROR     %d\n"),
        (status & XC6S_STATUS_ID_ERROR) ? 1 : 0);
    urj_log (URJ_LOG_LEVEL_NORMAL, _("\tCRC_ERROR    %d\n"),
        (status & XC6S_STATUS_CRC_ERROR) ? 1 : 0);

    return URJ_STATUS_OK;
}

static int
xlx_configure (urj_pld_t *pld, FILE *bit_file)
{
    urj_chain_t *chain = pld->chain;
    urj_part_t *part = pld->part;
    urj_part_instruction_t *i;
    xlx_bitstream_t *bs;
    uint32_t u;
    int dr_len;
    char *dr_data;
    int status = URJ_STATUS_OK;

    /* set all devices in bypass mode */
    urj_tap_reset_bypass (chain);

    bs = xlx_bitstream_alloc ();
    if (bs == NULL)
    {
        status = URJ_STATUS_FAIL;
        goto fail;
    }

    /* parse bit file */
    if (xlx_bitstream_load_bit (bit_file, bs) != URJ_STATUS_OK)
    {
        urj_error_set (URJ_ERROR_PLD, _("Invalid bitfile"));

        status = URJ_STATUS_FAIL;
        goto fail_free;
    }

    urj_log (URJ_LOG_LEVEL_NORMAL, _("Bitstream information:\n"));
    urj_log (URJ_LOG_LEVEL_NORMAL, _("\tDesign: %s\n"), bs->design);
    urj_log (URJ_LOG_LEVEL_NORMAL, _("\tPart name: %s\n"), bs->part_name);
    urj_log (URJ_LOG_LEVEL_NORMAL, _("\tDate: %s\n"), bs->date);
    urj_log (URJ_LOG_LEVEL_NORMAL, _("\tTime: %s\n"), bs->time);
    urj_log (URJ_LOG_LEVEL_NORMAL, _("\tBitstream length: %d\n"), bs->length);

    dr_len = bs->length * 8;

    if (xlx_instruction_resize_dr (part, "CFG_IN", "CFG_DR", dr_len)
            != URJ_STATUS_OK)
        return URJ_STATUS_FAIL;

    i = urj_part_find_instruction (part, "CFG_IN");

    /* copy data into shift register */
    dr_data = i->data_register->in->data;
    for (u = 0; u < bs->length; u++)
    {
        /* flip bits */
        dr_data[8*u+0] = (bs->data[u] & 0x80) ? 1 : 0;
        dr_data[8*u+1] = (bs->data[u] & 0x40) ? 1 : 0;
        dr_data[8*u+2] = (bs->data[u] & 0x20) ? 1 : 0;
        dr_data[8*u+3] = (bs->data[u] & 0x10) ? 1 : 0;
        dr_data[8*u+4] = (bs->data[u] & 0x08) ? 1 : 0;
        dr_data[8*u+5] = (bs->data[u] & 0x04) ? 1 : 0;
        dr_data[8*u+6] = (bs->data[u] & 0x02) ? 1 : 0;
        dr_data[8*u+7] = (bs->data[u] & 0x01) ? 1 : 0;
    }

    if (xlx_set_ir_and_shift (chain, part, "JPROGRAM") != URJ_STATUS_OK)
    {
        status = URJ_STATUS_FAIL;
        goto fail_free;
    }

    if (xlx_set_ir_and_shift (chain, part, "CFG_IN") != URJ_STATUS_OK)
    {
        status = URJ_STATUS_FAIL;
        goto fail_free;
    }

    /* wait until device is unconfigured */
    do {
        urj_tap_chain_shift_instructions_mode (chain, 1, 1,
                URJ_CHAIN_EXITMODE_IDLE);
    } while (!(urj_tap_register_get_value (part->active_instruction->out)
                & XILINX_SR_INIT));

    if (xlx_set_ir_and_shift (chain, part, "CFG_IN") != URJ_STATUS_OK)
    {
        status = URJ_STATUS_FAIL;
        goto fail_free;
    }

    urj_tap_chain_shift_data_registers (chain, 0);

    if (xlx_set_ir_and_shift (chain, part, "JSTART") != URJ_STATUS_OK)
    {
        status = URJ_STATUS_FAIL;
        goto fail_free;
    }

    urj_tap_chain_defer_clock (chain, 0, 0, 32);

    urj_tap_reset_bypass (chain);

    urj_tap_chain_flush (chain);

 fail_free:
    xlx_bitstream_free (bs);
 fail:
    return status;
}

static int
xlx_reconfigure (urj_pld_t *pld)
{
    urj_chain_t *chain = pld->chain;
    urj_part_t *part = pld->part;

    urj_tap_reset_bypass (chain);

    if (xlx_set_ir_and_shift (chain, part, "JPROGRAM") != URJ_STATUS_OK)
        return URJ_STATUS_FAIL;

    urj_tap_reset (chain);
    urj_tap_chain_flush (chain);

    return URJ_STATUS_OK;
}

static int
xlx_detect_xc3s (urj_pld_t *pld)
{
    urj_part_t *part = pld->part;
    uint32_t idcode;
    uint32_t family;

    /* get fpga family from idcode */
    idcode = urj_tap_register_get_value (part->id);
    family = (idcode >> 21) & 0x7f;

    switch (family)
    {
        case XILINX_FAMILY_XC3S:
        case XILINX_FAMILY_XC3SE:
        case XILINX_FAMILY_XC3A:
        case XILINX_FAMILY_XC3AN:
        case XILINX_FAMILY_XC3SD:
            return URJ_STATUS_OK;
        default:
            return URJ_STATUS_FAIL;
    }
}

static int
xlx_detect_xc4v (urj_pld_t *pld)
{
    urj_part_t *part = pld->part;
    uint32_t idcode;
    uint32_t family;

    /* get fpga family from idcode */
    idcode = urj_tap_register_get_value (part->id);
    family = (idcode >> 21) & 0x7f;

    switch (family)
    {
        case XILINX_FAMILY_XC4VFX:
        case XILINX_FAMILY_XC4VLX:
        case XILINX_FAMILY_XC4VSX:
            return URJ_STATUS_OK;
        default:
            return URJ_STATUS_FAIL;
    }
}

static int
xlx_detect_xc6s (urj_pld_t *pld)
{
    urj_part_t *part = pld->part;
    uint32_t idcode;
    uint32_t family;

    /* get fpga family from idcode */
    idcode = urj_tap_register_get_value (part->id);
    family = (idcode >> 21) & 0x7f;

    switch (family)
    {
        case XILINX_FAMILY_XC6S:
            return URJ_STATUS_OK;
        default:
            return URJ_STATUS_FAIL;
    }
}

const urj_pld_driver_t urj_pld_xc3s_driver = {
    .name = N_("Xilinx Spartan 3 Family"),
    .detect = xlx_detect_xc3s,
    .print_status = xlx_print_status_xc3s,
    .configure = xlx_configure,
    .reconfigure = xlx_reconfigure,
    .read_register = xlx_read_register_xc3s,
    .write_register = xlx_write_register_xc3s,
    .register_width = 2,
};

const urj_pld_driver_t urj_pld_xc6s_driver = {
    .name = N_("Xilinx Spartan 6 Family"),
    .detect = xlx_detect_xc6s,
    .print_status = xlx_print_status_xc6s,
    .configure = xlx_configure,
    .reconfigure = xlx_reconfigure,
    .read_register = xlx_read_register_xc6s,
    .write_register = xlx_write_register_xc6s,
    .register_width = 2,
};

const urj_pld_driver_t urj_pld_xc4v_driver = {
    .name = N_("Xilinx Virtex 4 Family"),
    .detect = xlx_detect_xc4v,
    .print_status = xlx_print_status_xc4v,
    .configure = xlx_configure,
    .reconfigure = xlx_reconfigure,
    .read_register = xlx_read_register_xc4v,
    .write_register = xlx_write_register_xc4v,
    .register_width = 2,
};
