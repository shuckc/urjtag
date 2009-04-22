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

#include "sysdep.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "data_register.h"

urj_data_register_t *
urj_part_data_register_alloc (const char *name, int len)
{
    urj_data_register_t *dr;

    if (!name)
        return NULL;

    dr = malloc (sizeof *dr);
    if (!dr)
        return NULL;

    if (strlen (name) > URJ_DATA_REGISTER_MAXLEN)
        printf (_("Warning: Data register name too long\n"));
    strncpy (dr->name, name, URJ_DATA_REGISTER_MAXLEN);
    dr->name[URJ_DATA_REGISTER_MAXLEN] = '\0';

    if (len > 0)
    {
        dr->in = urj_tap_register_alloc (len);
        dr->out = urj_tap_register_alloc (len);
    }
    else
    {
        dr->in = urj_tap_register_alloc (1);
        dr->out = urj_tap_register_alloc (1);
    }
    if (!dr->in || !dr->out)
    {
        free (dr->in);
        free (dr->out);
        free (dr->name);
        free (dr);
        return NULL;
    }

    dr->next = NULL;

    return dr;
}

void
urj_part_data_register_free (urj_data_register_t *dr)
{
    if (!dr)
        return;

    urj_tap_register_free (dr->in);
    urj_tap_register_free (dr->out);
    free (dr);
}
