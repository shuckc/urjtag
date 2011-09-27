/*
 * $Id$
 *
 * Copyright (C) 2002 ETC s.r.o.
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
 * Written by Marcel Telka <marcel@telka.sk>, 2002.
 *
 */

#include <sysdep.h>

#include <stdio.h>

#include <urjtag/log.h>
#include <urjtag/cable.h>
#include <urjtag/part.h>
#include <urjtag/tap_register.h>
#include <urjtag/tap.h>
#include <urjtag/tap_state.h>
#include <urjtag/chain.h>

void
urj_tap_reset (urj_chain_t *chain)
{
    urj_tap_state_reset (chain);

    urj_tap_chain_clock (chain, 1, 0, 5);       /* Test-Logic-Reset */
    urj_tap_chain_clock (chain, 0, 0, 1);       /* Run-Test/Idle */
}

void
urj_tap_trst_reset (urj_chain_t *chain)
{
    urj_tap_chain_set_trst (chain, 0);
    urj_tap_chain_set_trst (chain, 1);

    urj_tap_reset (chain);
}

int
urj_tap_reset_bypass (urj_chain_t *chain)
{
    urj_tap_reset (chain);

    /* set all parts in the chain to BYPASS instruction if the total
       instruction register length of the chain is already known */
    if (chain->total_instr_len > 0)
    {
        urj_tap_register_t *ir = urj_tap_register_fill (
                        urj_tap_register_alloc (chain->total_instr_len), 1);
        if (!ir)
            return URJ_STATUS_FAIL;

        urj_tap_capture_ir (chain);
        urj_tap_shift_register (chain, ir, NULL, URJ_CHAIN_EXITMODE_IDLE);
        urj_tap_register_free (ir);

        urj_part_parts_set_instruction (chain->parts, "BYPASS");
    }

    return URJ_STATUS_OK;
}

void
urj_tap_defer_shift_register (urj_chain_t *chain,
                              const urj_tap_register_t *in,
                              urj_tap_register_t *out, int tap_exit)
{
    int i;

    if (!(urj_tap_state (chain) & URJ_TAP_STATE_SHIFT))
        urj_log (URJ_LOG_LEVEL_NORMAL, _("%s: Invalid state: %2X\n"), __func__,
                urj_tap_state (chain));

    /* Capture-DR, Capture-IR, Shift-DR, Shift-IR, Exit2-DR or Exit2-IR state */
    if (urj_tap_state (chain) & URJ_TAP_STATE_CAPTURE)
        urj_tap_chain_defer_clock (chain, 0, 0, 1);     /* save last TDO bit :-) */

    i = in->len;
    if (tap_exit)
        i--;
    if (out && out->len < i)
        i = out->len;

    if (out)
        urj_tap_cable_defer_transfer (chain->cable, i, in->data, out->data);
    else
        urj_tap_cable_defer_transfer (chain->cable, i, in->data, NULL);

    for (; i < in->len; i++)
    {
        if (out != NULL && (i < out->len))
            out->data[i] = urj_tap_cable_defer_get_tdo (chain->cable);
        urj_tap_chain_defer_clock (chain, (tap_exit != URJ_CHAIN_EXITMODE_SHIFT && ((i + 1) == in->len)) ? 1 : 0, in->data[i], 1);      /* Shift (& Exit1) */
    }

    /* Shift-DR, Shift-IR, Exit1-DR or Exit1-IR state */
    if (tap_exit == URJ_CHAIN_EXITMODE_IDLE)
    {
        urj_tap_chain_defer_clock (chain, 1, 0, 1);     /* Update-DR or Update-IR */
        urj_tap_chain_defer_clock (chain, 0, 0, 1);     /* Run-Test/Idle */
        urj_tap_chain_wait_ready (chain);
    }
    else if (tap_exit == URJ_CHAIN_EXITMODE_UPDATE)
        urj_tap_chain_defer_clock (chain, 1, 0, 1);     /* Update-DR or Update-IR */
}

void
urj_tap_shift_register_output (urj_chain_t *chain,
                               const urj_tap_register_t *in,
                               urj_tap_register_t *out, int tap_exit)
{
    if (out != NULL)
    {
        int j;

        j = in->len;
        if (tap_exit)
            j--;
        if (out && out->len < j)
            j = out->len;

        /* Asking for the result of the cable transfer
         * actually flushes the queue */

        (void) urj_tap_cable_transfer_late (chain->cable, out->data);
        for (; j < in->len && j < out->len; j++)
            out->data[j] = urj_tap_cable_get_tdo_late (chain->cable);
    }
}

void
urj_tap_shift_register (urj_chain_t *chain, const urj_tap_register_t *in,
                        urj_tap_register_t *out, int tap_exit)
{
    urj_tap_defer_shift_register (chain, in, out, tap_exit);
    urj_tap_shift_register_output (chain, in, out, tap_exit);
}

void
urj_tap_capture_dr (urj_chain_t *chain)
{
    if ((urj_tap_state (chain) & (URJ_TAP_STATE_RESET | URJ_TAP_STATE_IDLE))
        != URJ_TAP_STATE_IDLE)
        urj_log (URJ_LOG_LEVEL_NORMAL, _("%s: Invalid state: %2X\n"), __func__,
                 urj_tap_state (chain));

    /* Run-Test/Idle or Update-DR or Update-IR state */
    urj_tap_chain_defer_clock (chain, 1, 0, 1); /* Select-DR-Scan */
    urj_tap_chain_defer_clock (chain, 0, 0, 1); /* Capture-DR */
}

void
urj_tap_capture_ir (urj_chain_t *chain)
{
    if ((urj_tap_state (chain) & (URJ_TAP_STATE_RESET | URJ_TAP_STATE_IDLE))
        != URJ_TAP_STATE_IDLE)
        urj_log (URJ_LOG_LEVEL_NORMAL, _("%s: Invalid state: %2X\n"), __func__,
                 urj_tap_state (chain));

    /* Run-Test/Idle or Update-DR or Update-IR state */
    urj_tap_chain_defer_clock (chain, 1, 0, 2); /* Select-DR-Scan, then Select-IR-Scan */
    urj_tap_chain_defer_clock (chain, 0, 0, 1); /* Capture-IR */
}
