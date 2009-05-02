/*
 * $Id: idcode.c 1120 2008-03-15 02:27:13Z jiez $
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
#include <urjtag/sysdep.h>

#include <stdio.h>
#include <stdlib.h>

#include <urjtag/tap.h>
#include <urjtag/tap_register.h>
#include <urjtag/chain.h>


void
urj_tap_idcode (urj_chain_t *chain, unsigned int bytes)
{
    int i;
    int hit = 0;
    urj_tap_register_t *rz;
    urj_tap_register_t *rout;
    urj_tap_register_t *rnull;

    urj_tap_chain_set_trst (chain, 0);
    urj_tap_chain_set_trst (chain, 1);

    urj_tap_reset (chain);
    urj_tap_capture_dr (chain);

    /* read in chunks of 8 bytes */
    rz = urj_tap_register_fill (urj_tap_register_alloc (8), 0);
    rnull = urj_tap_register_fill (urj_tap_register_alloc (8), 0);
    rout = urj_tap_register_alloc (8);

    if (!rz || !rout || !rnull)
    {
        printf (_("Allocation failed\n"));
    }
    printf (_("Read"));
    for (i = 0; i < ((bytes) ? bytes : 1000); i++)
    {
        urj_tap_shift_register (chain, rz, rout, 0);
        printf (_(" %s"), urj_tap_register_get_string (rout));
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
    urj_tap_register_free (rz);
    urj_tap_register_free (rnull);
    urj_tap_register_free (rout);
    printf (_("\n"));
}
