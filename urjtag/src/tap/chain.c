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

#include <sysdep.h>

#include <stdlib.h>
#include <string.h>

#include <urjtag/cable.h>
#include <urjtag/part.h>
#include <urjtag/part_instruction.h>
#include <urjtag/tap_state.h>
#include <urjtag/tap.h>
#include <urjtag/data_register.h>
#include <urjtag/cmd.h>
#include <urjtag/bsdl.h>

#include <urjtag/chain.h>

urj_chain_t *
urj_tap_chain_alloc (void)
{
    urj_chain_t *chain = malloc (sizeof (urj_chain_t));
    if (!chain)
    {
        urj_error_set (URJ_ERROR_OUT_OF_MEMORY, "malloc(%zd) fails",
                       sizeof (urj_chain_t));
        return NULL;
    }

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

int
urj_tap_chain_connect (urj_chain_t *chain, const char *drivername, char *params[])
{
    urj_cable_t *cable;
    int j, paramc;
    const urj_param_t **cable_params;
    const urj_cable_driver_t *driver;

    urj_cable_parport_devtype_t devtype;
    const char *devname;
    int param_start;

    param_start = 0;
    paramc = urj_cmd_params (params);

    driver = urj_tap_cable_find (drivername);
    if (!driver)
    {
        urj_error_set (URJ_ERROR_INVALID,
                       "unknown cable driver '%s'", drivername);
        return URJ_STATUS_FAIL;
    }

    if (driver->device_type == URJ_CABLE_DEVICE_PARPORT)
    {
        if (paramc < 2)
        {
            urj_error_set (URJ_ERROR_SYNTAX,
                           "parallel cable requires >= 4 parameters, got %d", paramc);
            return URJ_STATUS_FAIL;
        }
        for (j = 0; j < URJ_CABLE_PARPORT_N_DEVS; j++)
            if (strcasecmp (params[0],
                            urj_cable_parport_devtype_string (j)) == 0)
                break;
        if (j == URJ_CABLE_PARPORT_N_DEVS)
        {
            urj_error_set (URJ_ERROR_INVALID,
                           "unknown parallel port device type '%s'",
                           params[0]);
            return URJ_STATUS_FAIL;
        }

        devtype = j;
        devname = params[1];
        param_start = 2;
    }
    else
    {
        /* Silence gcc uninitialized warnings */
        devtype = -1;
        devname = NULL;
    }

    if (urj_param_init_list (&cable_params, &params[param_start],
                             &urj_cable_param_list) != URJ_STATUS_OK)
        return URJ_STATUS_FAIL;

    switch (driver->device_type)
    {
    case URJ_CABLE_DEVICE_PARPORT:
        cable = urj_tap_cable_parport_connect (chain, driver, devtype, devname,
                                               cable_params);
        break;
    case URJ_CABLE_DEVICE_USB:
        cable = urj_tap_cable_usb_connect (chain, driver, cable_params);
        break;
    case URJ_CABLE_DEVICE_OTHER:
        cable = urj_tap_cable_other_connect (chain, driver, cable_params);
        break;
    default:
        cable = NULL;
        break;
    }

    urj_param_clear (&cable_params);

    if (cable == NULL)
        return URJ_STATUS_FAIL;

    chain->cable->chain = chain;
    return URJ_STATUS_OK;
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

int
urj_tap_chain_clock (urj_chain_t *chain, int tms, int tdi, int n)
{
    int i;

    if (!chain || !chain->cable)
    {
        urj_error_set (URJ_ERROR_NO_CHAIN, "no chain or no part");
        return URJ_STATUS_FAIL;
    }

    urj_tap_cable_clock (chain->cable, tms, tdi, n);

    for (i = 0; i < n; i++)
        urj_tap_state_clock (chain, tms);

    return URJ_STATUS_OK;
}

int
urj_tap_chain_defer_clock (urj_chain_t *chain, int tms, int tdi, int n)
{
    int i;

    if (!chain || !chain->cable)
    {
        urj_error_set (URJ_ERROR_NO_CHAIN, "no chain or no part");
        return URJ_STATUS_FAIL;
    }

    urj_tap_cable_defer_clock (chain->cable, tms, tdi, n);

    for (i = 0; i < n; i++)
        urj_tap_state_clock (chain, tms);

    return URJ_STATUS_OK;
}

int
urj_tap_chain_set_trst (urj_chain_t *chain, int trst)
{
    int old_val = urj_tap_cable_set_signal (chain->cable, URJ_POD_CS_TRST,
                                            trst ? URJ_POD_CS_TRST : 0);
    int old_trst = (old_val & URJ_POD_CS_TRST) ? 1 : 0;

    urj_tap_state_set_trst (chain, old_trst, trst);

    return trst;
}

int
urj_tap_chain_get_trst (urj_chain_t *chain)
{
    return urj_tap_cable_get_signal (chain->cable, URJ_POD_CS_TRST);
}

int
urj_tap_chain_set_pod_signal (urj_chain_t *chain, int mask, int val)
{
    int old_val = urj_tap_cable_set_signal (chain->cable, mask, val);
    int old_trst = (old_val & URJ_POD_CS_TRST) ? 1 : 0;
    int new_trst =
        (((old_val & ~mask) | (val & mask)) & URJ_POD_CS_TRST) ? 1 : 0;

    urj_tap_state_set_trst (chain, old_trst, new_trst);

    return old_val;
}

int
urj_tap_chain_get_pod_signal (urj_chain_t *chain, urj_pod_sigsel_t sig)
{
    return urj_tap_cable_get_signal (chain->cable, sig);
}

int
urj_tap_chain_shift_instructions_mode (urj_chain_t *chain,
                                       int capture_output, int capture,
                                       int chain_exit)
{
    int i;
    urj_parts_t *ps;

    if (!chain || !chain->parts)
    {
        urj_error_set (URJ_ERROR_NO_CHAIN, "no chain or no part");
        return URJ_STATUS_FAIL;
    }

    ps = chain->parts;

    for (i = 0; i < ps->len; i++)
    {
        if (ps->parts[i]->active_instruction == NULL)
        {
            urj_error_set (URJ_ERROR_NO_ACTIVE_INSTRUCTION,
                           _("Part %d without active instruction"), i);
            return URJ_STATUS_FAIL;
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
                capture_output ? ps->parts[i]->active_instruction->out
                    : NULL,
                (i + 1) == ps->len ? chain_exit : URJ_CHAIN_EXITMODE_SHIFT);
    }

    if (capture_output)
    {
        for (i = 0; i < ps->len; i++)
        {
            urj_tap_shift_register_output (chain,
                    ps->parts[i]->active_instruction->value,
                    ps->parts[i]->active_instruction->out,
                    (i + 1) == ps->len ? chain_exit
                        : URJ_CHAIN_EXITMODE_SHIFT);
        }
    }
    else
    {
        /* give the cable driver a chance to flush if it's considered useful */
        urj_tap_cable_flush (chain->cable, URJ_TAP_CABLE_TO_OUTPUT);
    }

    return URJ_STATUS_OK;
}

int
urj_tap_chain_shift_instructions (urj_chain_t *chain)
{
    return urj_tap_chain_shift_instructions_mode (chain, 0, 1,
                                                  URJ_CHAIN_EXITMODE_IDLE);
}

int
urj_tap_chain_shift_data_registers_mode (urj_chain_t *chain,
                                         int capture_output, int capture,
                                         int chain_exit)
{
    int i;
    urj_parts_t *ps;

    if (!chain || !chain->parts)
    {
        urj_error_set (URJ_ERROR_NO_CHAIN, "no chain or no part");
        return URJ_STATUS_FAIL;
    }

    ps = chain->parts;

    for (i = 0; i < ps->len; i++)
    {
        if (ps->parts[i]->active_instruction == NULL)
        {
            urj_error_set (URJ_ERROR_NO_ACTIVE_INSTRUCTION,
                           _("Part %d without active instruction"), i);
            return URJ_STATUS_FAIL;
        }
        if (ps->parts[i]->active_instruction->data_register == NULL)
        {
            urj_error_set (URJ_ERROR_NO_DATA_REGISTER,
                           _("Part %d without data register"), i);
            return URJ_STATUS_FAIL;
        }
    }

    if (capture)
        urj_tap_capture_dr (chain);

    /* new implementation: split into defer + retrieve part
       shift the data register of each part in the chain one by one */

    for (i = 0; i < ps->len; i++)
    {
        urj_tap_defer_shift_register (chain,
                ps->parts[i]->active_instruction->data_register->in,
                capture_output ?
                    ps->parts[i]->active_instruction->data_register->out
                    : NULL,
                (i + 1) == ps->len ? chain_exit : URJ_CHAIN_EXITMODE_SHIFT);
    }

    if (capture_output)
    {
        for (i = 0; i < ps->len; i++)
        {
            urj_tap_shift_register_output (chain,
                    ps->parts[i]->active_instruction->data_register->in,
                    ps->parts[i]->active_instruction->data_register->out,
                    (i + 1) == ps->len ? chain_exit : URJ_CHAIN_EXITMODE_SHIFT);
        }
    }
    else
    {
        /* give the cable driver a chance to flush if it's considered useful */
        urj_tap_cable_flush (chain->cable, URJ_TAP_CABLE_TO_OUTPUT);
    }

    return URJ_STATUS_OK;
}

int
urj_tap_chain_shift_data_registers (urj_chain_t *chain, int capture_output)
{
    return urj_tap_chain_shift_data_registers_mode (chain, capture_output, 1,
                                                    URJ_CHAIN_EXITMODE_IDLE);
}

void
urj_tap_chain_flush (urj_chain_t *chain)
{
    if (chain->cable != NULL)
        urj_tap_cable_flush (chain->cable, URJ_TAP_CABLE_COMPLETELY);
}

urj_part_t *
urj_tap_chain_active_part (urj_chain_t *chain)
{
    if (chain == NULL)
    {
        urj_error_set (URJ_ERROR_NO_CHAIN, "no JTAG chain");
        return NULL;
    }

    if (!chain->parts)
    {
        urj_error_set (URJ_ERROR_NOTFOUND, _("Run \"detect\" first"));
        return NULL;
    }
    if (chain->active_part >= chain->parts->len)
    {
        urj_error_set (URJ_ERROR_ILLEGAL_STATE,
                       _("active part no %d exceeds chain length %d"),
                       chain->active_part, chain->parts->len);
        return NULL;
    }

    return chain->parts->parts[chain->active_part];
}

void
urj_tap_chain_wait_ready (urj_chain_t *chain)
{
    urj_part_t *part;

    if (!chain || !chain->parts)
        return;

    part = chain->parts->parts[chain->main_part];
    if (part->params && part->params->wait_ready)
        part->params->wait_ready (chain);
}

