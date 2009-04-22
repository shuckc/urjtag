/*
 * $Id$
 *
 * Copyright (C) 2003 ETC s.r.o.
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
 * Written by Marcel Telka <marcel@telka.sk>, 2003.
 *
 */

#include "sysdep.h"

#include <stdlib.h>
#include <string.h>

#include "chain.h"
#include "tap_state.h"
#include "tap.h"

#include "bsdl.h"

urj_chain_t *
urj_tap_chain_alloc (void)
{
    urj_chain_t *chain = malloc (sizeof (urj_chain_t));
    if (!chain)
        return NULL;

    chain->cable = NULL;
    chain->parts = NULL;
    chain->total_instr_len = 0;
    chain->active_part = 0;
    URJ_BSDL_GLOBS_INIT (chain->bsdl);
    urj_tap_state_init (chain);

    return chain;
}

void
urj_tap_chain_free (urj_chain_t *chain)
{
    if (!chain)
        return;

    urj_tap_chain_disconnect (chain);

    urj_part_parts_free (chain->parts);
    free (chain);
}

void
urj_tap_chain_disconnect (urj_chain_t *chain)
{
    if (!chain->cable)
        return;

    urj_tap_state_done (chain);
    urj_tap_cable_done (chain->cable);
    urj_tap_cable_free (chain->cable);
    chain->cable = NULL;
}

void
urj_tap_chain_clock (urj_chain_t *chain, int tms, int tdi, int n)
{
    int i;

    if (!chain || !chain->cable)
        return;

    urj_tap_cable_clock (chain->cable, tms, tdi, n);

    for (i = 0; i < n; i++)
        urj_tap_state_clock (chain, tms);
}

void
urj_tap_chain_defer_clock (urj_chain_t *chain, int tms, int tdi, int n)
{
    int i;

    if (!chain || !chain->cable)
        return;

    urj_tap_cable_defer_clock (chain->cable, tms, tdi, n);

    for (i = 0; i < n; i++)
        urj_tap_state_clock (chain, tms);
}

int
urj_tap_chain_set_trst (urj_chain_t *chain, int trst)
{
    int old_val =
        urj_tap_cable_set_signal (chain->cable, URJ_POD_CS_TRST, trst ? URJ_POD_CS_TRST : 0);
    int old_trst = (old_val & URJ_POD_CS_TRST) ? 1 : 0;
    urj_tap_state_set_trst (chain, old_trst, trst);
    return trst;
}

int
urj_tap_chain_get_trst (urj_chain_t *chain)
{
    return (urj_tap_cable_get_signal (chain->cable, URJ_POD_CS_TRST));
}

int
urj_tap_chain_set_pod_signal (urj_chain_t *chain, int mask, int val)
{
    int old_val = urj_tap_cable_set_signal (chain->cable, mask, val);
    int old_trst = (old_val & URJ_POD_CS_TRST) ? 1 : 0;
    int new_trst = (((old_val & ~mask) | (val & mask)) & URJ_POD_CS_TRST) ? 1 : 0;
    urj_tap_state_set_trst (chain, old_trst, new_trst);
    return old_val;
}

int
urj_tap_chain_get_pod_signal (urj_chain_t *chain, urj_pod_sigsel_t sig)
{
    return (urj_tap_cable_get_signal (chain->cable, sig));
}

void
urj_tap_chain_shift_instructions_mode (urj_chain_t *chain, int capture_output,
                               int capture, int chain_exit)
{
    int i;
    urj_parts_t *ps;

    if (!chain || !chain->parts)
        return;

    ps = chain->parts;

    for (i = 0; i < ps->len; i++)
    {
        if (ps->parts[i]->active_instruction == NULL)
        {
            printf (_("%s(%d) Part %d without active instruction\n"),
                    __FILE__, __LINE__, i);
            return;
        }
    }

    if (capture)
        urj_tap_capture_ir (chain);

    /* new implementation: split into defer + retrieve part
       shift the data register of each part in the chain one by one */

    for (i = 0; i < ps->len; i++)
    {
        urj_tap_defer_shift_register (chain,
                                  ps->parts[i]->active_instruction->value,
                                  capture_output ? ps->parts[i]->
                                  active_instruction->out : NULL,
                                  (i + 1) ==
                                  ps->len ? chain_exit : URJ_CHAIN_EXITMODE_SHIFT);
    }

    if (capture_output)
    {
        for (i = 0; i < ps->len; i++)
        {
            urj_tap_shift_register_output (chain,
                                       ps->parts[i]->active_instruction->
                                       value,
                                       ps->parts[i]->active_instruction->out,
                                       (i + 1) ==
                                       ps->len ? chain_exit : URJ_CHAIN_EXITMODE_SHIFT);
        }
    }
    else
    {
        /* give the cable driver a chance to flush if it's considered useful */
        urj_tap_cable_flush (chain->cable, URJ_TAP_CABLE_TO_OUTPUT);
    }
}

void
urj_tap_chain_shift_instructions (urj_chain_t *chain)
{
    urj_tap_chain_shift_instructions_mode (chain, 0, 1, URJ_CHAIN_EXITMODE_IDLE);
}

void
urj_tap_chain_shift_data_registers_mode (urj_chain_t *chain, int capture_output,
                                 int capture, int chain_exit)
{
    int i;
    urj_parts_t *ps;

    if (!chain || !chain->parts)
        return;

    ps = chain->parts;

    for (i = 0; i < ps->len; i++)
    {
        if (ps->parts[i]->active_instruction == NULL)
        {
            printf (_("%s(%d) Part %d without active instruction\n"),
                    __FILE__, __LINE__, i);
            return;
        }
        if (ps->parts[i]->active_instruction->data_register == NULL)
        {
            printf (_("%s(%d) Part %d without data register\n"), __FILE__,
                    __LINE__, i);
            return;
        }
    }

    if (capture)
        urj_tap_capture_dr (chain);

    /* new implementation: split into defer + retrieve part
       shift the data register of each part in the chain one by one */

    for (i = 0; i < ps->len; i++)
    {
        urj_tap_defer_shift_register (chain,
                                  ps->parts[i]->active_instruction->
                                  data_register->in,
                                  capture_output ? ps->parts[i]->
                                  active_instruction->data_register->
                                  out : NULL,
                                  (i + 1) ==
                                  ps->len ? chain_exit : URJ_CHAIN_EXITMODE_SHIFT);
    }

    if (capture_output)
    {
        for (i = 0; i < ps->len; i++)
        {
            urj_tap_shift_register_output (chain,
                                       ps->parts[i]->active_instruction->
                                       data_register->in,
                                       ps->parts[i]->active_instruction->
                                       data_register->out,
                                       (i + 1) ==
                                       ps->len ? chain_exit : URJ_CHAIN_EXITMODE_SHIFT);
        }
    }
    else
    {
        /* give the cable driver a chance to flush if it's considered useful */
        urj_tap_cable_flush (chain->cable, URJ_TAP_CABLE_TO_OUTPUT);
    }
}

void
urj_tap_chain_shift_data_registers (urj_chain_t *chain, int capture_output)
{
    urj_tap_chain_shift_data_registers_mode (chain, capture_output, 1, URJ_CHAIN_EXITMODE_IDLE);
}

void
urj_tap_chain_flush (urj_chain_t *chain)
{
    if (chain->cable != NULL)
        urj_tap_cable_flush (chain->cable, URJ_TAP_CABLE_COMPLETELY);
}
