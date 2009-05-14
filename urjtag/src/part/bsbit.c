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

#include <stdlib.h>
#include <string.h>

#include <urjtag/chain.h>
#include <urjtag/part.h>
#include <urjtag/data_register.h>
#include <urjtag/tap_register.h>
#include <urjtag/bssignal.h>
#include <urjtag/bsbit.h>

int
urj_part_bsbit_alloc_control (urj_part_t *part, int bit, const char *name,
                              int type, int safe,
                              int ctrl_num, int ctrl_val, int ctrl_state)
{
    urj_bsbit_t *b;
    urj_data_register_t *bsr;
    urj_part_signal_t *signal;

    bsr = urj_part_find_data_register (part, "BSR");
    if (bsr == NULL)
    {
        urj_error_set(URJ_ERROR_NOTFOUND,
                      _("missing Boundary Scan Register (BSR)"));
        return URJ_STATUS_FAIL;
    }

    if (bit >= bsr->in->len)
    {
        urj_error_set(URJ_ERROR_INVALID, _("invalid boundary bit number"));
        return URJ_STATUS_FAIL;
    }
    if (part->bsbits[bit] != NULL)
    {
        urj_error_set(URJ_ERROR_ALREADY, _("duplicate bit declaration"));
        return URJ_STATUS_FAIL;
    }
    if (ctrl_num != -1 && ctrl_num >= bsr->in->len)
    {
        urj_error_set(URJ_ERROR_INVALID, _("invalid control bit number"));
        return URJ_STATUS_FAIL;
    }

    signal = urj_part_find_signal (part, name);

    bsr->in->data[bit] = safe;

    b = malloc (sizeof *b);
    if (!b)
    {
        urj_error_set (URJ_ERROR_OUT_OF_MEMORY, "malloc(%zd) fails", sizeof *b);
        return URJ_STATUS_FAIL;
    }

    b->name = strdup (name);
    if (!b->name)
    {
        free (b);
        urj_error_set (URJ_ERROR_OUT_OF_MEMORY, "strdup(%s) fails", name);
        return URJ_STATUS_FAIL;
    }

    b->bit = bit;
    b->type = type;
    b->signal = signal;
    b->safe = (safe == 1);
    b->control = -1;

    part->bsbits[bit] = b;

    if (signal != NULL)
    {
        switch (type)
        {
        case URJ_BSBIT_INPUT:
            signal->input = b;
            break;
        case URJ_BSBIT_OUTPUT:
            signal->output = b;
            break;
        case URJ_BSBIT_BIDIR:
            signal->input = b;
            signal->output = b;
            break;
        }
    }

    if (ctrl_num != -1)
    {
        b->control = ctrl_num;
        b->control_value = ctrl_val;
        b->control_state = ctrl_state;
    }

    return URJ_STATUS_OK;
}

int
urj_part_bsbit_alloc (urj_part_t *part, int bit, const char *name, int type,
                      int safe)
{
    return urj_part_bsbit_alloc_control (part, bit, name, type, safe,
                                         -1, -1, -1);
}

void
urj_part_bsbit_free (urj_bsbit_t *b)
{
    if (!b)
        return;

    free (b->name);
    free (b);
}
