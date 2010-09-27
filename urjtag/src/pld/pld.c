/* 
 * $Id$
 *
 * Support for Programmable Logic Devices (PLD)
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

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <urjtag/pld.h>
#include <urjtag/chain.h>
#include <urjtag/part.h>
#include <urjtag/tap_register.h>
#include "xilinx.h"

const urj_pld_driver_t * const urj_pld_drivers[] = {
    &urj_pld_xc3s_driver,
    &urj_pld_xc6s_driver,
    &urj_pld_xc4v_driver,
    NULL
};

static const urj_pld_driver_t *pld_driver = NULL;
static urj_pld_t pld;

static int
set_pld_driver (urj_chain_t *chain, urj_part_t *part)
{
    int i;
    uint32_t idcode;

    pld_driver = NULL;
    pld.chain = chain;
    pld.part = part;

    for (i = 0; urj_pld_drivers[i] != NULL; i++)
    {
        if (urj_pld_drivers[i]->detect (&pld) == URJ_STATUS_OK)
        {
            pld_driver = urj_pld_drivers[i];
            return URJ_STATUS_OK;
        }
    }

    idcode = urj_tap_register_get_value (part->id);
    urj_log (URJ_LOG_LEVEL_ERROR,
             _("No PLD driver for device with ID %08x\n"),
             idcode);

    urj_error_set (URJ_ERROR_UNSUPPORTED, _("PLD not supported"));

    return URJ_STATUS_FAIL;
}

int
urj_pld_configure (urj_chain_t *chain, FILE *pld_file)
{
    urj_part_t *part;

    part = urj_tap_chain_active_part (chain);

    if (part == NULL)
        return URJ_STATUS_FAIL;

    if (set_pld_driver (chain, part) != URJ_STATUS_OK)
        return URJ_STATUS_FAIL;

    if (pld_driver->configure == NULL)
    {
        urj_error_set (URJ_ERROR_UNSUPPORTED,
                       _("PLD doesn't support this operation"));
        return URJ_STATUS_FAIL;
    }

    return pld_driver->configure (&pld, pld_file);
}

int
urj_pld_reconfigure (urj_chain_t *chain)
{
    urj_part_t *part;

    part = urj_tap_chain_active_part (chain);

    if (part == NULL)
        return URJ_STATUS_FAIL;

    if (set_pld_driver (chain, part) != URJ_STATUS_OK)
        return URJ_STATUS_FAIL;

    if (pld_driver->reconfigure == NULL)
    {
        urj_error_set (URJ_ERROR_UNSUPPORTED,
                       _("PLD doesn't support this operation"));
        return URJ_STATUS_FAIL;
    }

    return pld_driver->reconfigure (&pld);
}

int
urj_pld_print_status (urj_chain_t *chain)
{
    urj_part_t *part;

    part = urj_tap_chain_active_part (chain);

    if (part == NULL)
        return URJ_STATUS_FAIL;

    if (set_pld_driver (chain, part) != URJ_STATUS_OK)
        return URJ_STATUS_FAIL;

    if (pld_driver->print_status == NULL)
    {
        urj_error_set (URJ_ERROR_UNSUPPORTED,
                       _("PLD doesn't support this operation"));
        return URJ_STATUS_FAIL;
    }

    return pld_driver->print_status (&pld);
}

int
urj_pld_read_register (urj_chain_t *chain, uint32_t reg)
{
    urj_part_t *part;
    uint32_t value;

    part = urj_tap_chain_active_part (chain);

    if (part == NULL)
        return URJ_STATUS_FAIL;

    if (set_pld_driver (chain, part) != URJ_STATUS_OK)
        return URJ_STATUS_FAIL;

    if (pld_driver->read_register == NULL)
    {
        urj_error_set (URJ_ERROR_UNSUPPORTED,
                       _("PLD doesn't support this operation"));
        return URJ_STATUS_FAIL;
    }

    if (pld_driver->read_register (&pld, reg, &value) != URJ_STATUS_OK)
        return URJ_STATUS_FAIL;

    urj_log (URJ_LOG_LEVEL_NORMAL, N_("REG[%d]=0x%0*x\n"),
            reg, pld_driver->register_width * 2, value);

    return URJ_STATUS_OK;
}

int
urj_pld_write_register (urj_chain_t *chain, uint32_t reg, uint32_t data)
{
    urj_part_t *part;

    part = urj_tap_chain_active_part (chain);

    if (part == NULL)
        return URJ_STATUS_FAIL;

    if (set_pld_driver (chain, part) != URJ_STATUS_OK)
        return URJ_STATUS_FAIL;

    if (pld_driver->write_register == NULL)
    {
        urj_error_set (URJ_ERROR_UNSUPPORTED,
                       _("PLD doesn't support this operation"));
        return URJ_STATUS_FAIL;
    }

    return pld_driver->write_register (&pld, reg, data);
}
