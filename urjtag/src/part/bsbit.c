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

#include <stdlib.h>
#include <string.h>

#include "bsbit.h"

urj_bsbit_t *
urj_part_bsbit_alloc (int bit, const char *name, int type, urj_part_signal_t *signal, int safe)
{
    urj_bsbit_t *b = malloc (sizeof *b);
    if (!b)
        return NULL;

    b->name = strdup (name);
    if (!b->name)
    {
        free (b);
        return NULL;
    }

    b->bit = bit;
    b->type = type;
    b->signal = NULL;
    b->safe = safe;
    b->control = -1;

    if (signal != NULL)
    {
        b->signal = signal;
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

    return b;
}

void
urj_part_bsbit_free (urj_bsbit_t *b)
{
    if (!b)
        return;

    free (b->name);
    free (b);
}
