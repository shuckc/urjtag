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
 * Written by Uwe Bonnes <bon@elektron.ikp.physik.tu-darmstadt.de>, 2008.
 *
 */
#include <sysdep.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <urjtag/tap.h>
#include <urjtag/tap_register.h>
#include <urjtag/chain.h>
#include <urjtag/cable.h>


int
urj_tap_idcode (urj_chain_t *chain, unsigned int bytes)
{
    int ret;
    unsigned int i, hit, max_bytes;
    urj_tap_register_t *rz;
    urj_tap_register_t *rout;
    urj_tap_register_t *all_rout;
    urj_tap_register_t *rnull;

    ret = URJ_STATUS_FAIL;
    max_bytes = bytes ? bytes : 1000;
    hit = 0;

    /* read in chunks of 8 bits */
    if (chain->cable->driver->quirks & URJ_CABLE_QUIRK_ONESHOT)
    {
        all_rout = urj_tap_register_alloc (8 * max_bytes);
        if (!all_rout)
            return ret;
        rz = urj_tap_register_fill (urj_tap_register_alloc (8 * max_bytes), 0);
    }
    else
    {
        all_rout = NULL;
        rz = urj_tap_register_fill (urj_tap_register_alloc (8), 0);
    }
    rnull = urj_tap_register_fill (urj_tap_register_alloc (8), 0);
    rout = urj_tap_register_alloc (8);

    if (!rz || !rout || !rnull)
        goto done;

    urj_tap_trst_reset (chain);
    urj_tap_capture_dr (chain);

    if (all_rout)
        urj_tap_shift_register (chain, rz, all_rout, 0);

    urj_log (URJ_LOG_LEVEL_NORMAL, _("Read"));
    for (i = 0; i < max_bytes; ++i)
    {
        uint8_t val;

        if (all_rout)
            memcpy (rout->data, &all_rout->data[i * 8], 8 * sizeof (rout->data[0]));
        else
            urj_tap_shift_register (chain, rz, rout, 0);

        val = urj_tap_register_get_value (rout);
        urj_log (URJ_LOG_LEVEL_NORMAL, N_(" %s(0x%x%x)"),
                 urj_tap_register_get_string (rout),
                 (val >> 4) & 0xf, val & 0xf);
        if (!bytes)
        {
            /* Abort Reading when a null IDCODE has been read */
            if (!urj_tap_register_compare (rout, rnull))
                hit++;
            else
                hit = 0;
            if (hit > 3)
                break;
        }
    }
    urj_log (URJ_LOG_LEVEL_NORMAL, _("\n"));
    ret = URJ_STATUS_OK;

 done:
    urj_tap_register_free (rz);
    urj_tap_register_free (rnull);
    urj_tap_register_free (rout);
    urj_tap_register_free (all_rout);

    return ret;
}
