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
#include <stdio.h>

#include <urjtag/part.h>
#include <urjtag/tap_register.h>
#include <urjtag/data_register.h>
#include <urjtag/error.h>

urj_data_register_t *
urj_part_data_register_alloc (const char *name, int len)
{
    urj_data_register_t *dr;

    if (!name)
        return NULL;

    dr = malloc (sizeof *dr);
    if (!dr)
    {
        urj_error_set (URJ_ERROR_OUT_OF_MEMORY, "malloc(%zd) fails",
                       sizeof *dr);
        return NULL;
    }

    if (strlen (name) > URJ_DATA_REGISTER_MAXLEN)
        urj_warning (_("Data register name too long\n"));
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
        // retain error state
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

int
urj_part_data_register_define (urj_part_t *part, const char *name, int len)
{
    urj_data_register_t *dr;

    if (urj_part_find_data_register (part, name) != NULL)
    {
        urj_error_set (URJ_ERROR_ALREADY,
                       _("Data register '%s' already defined"), name);
        return URJ_STATUS_FAIL;
    }

    dr = urj_part_data_register_alloc (name, len);
    if (!dr)
        // retain error state
        return URJ_STATUS_FAIL;

    dr->next = part->data_registers;
    part->data_registers = dr;

    /* Boundary Scan Register */
    if (strcasecmp (dr->name, "BSR") == 0)
    {
        int i;

        part->boundary_length = len;
        part->bsbits = malloc (part->boundary_length * sizeof *part->bsbits);
        if (!part->bsbits)
        {
            urj_error_set (URJ_ERROR_OUT_OF_MEMORY, "malloc(%zd) fails",
                           part->boundary_length * sizeof *part->bsbits);
            return URJ_STATUS_FAIL;
        }
        for (i = 0; i < part->boundary_length; i++)
            part->bsbits[i] = NULL;
    }

    /* Device Identification Register */
    else if (strcasecmp (dr->name, "DIR") == 0)
        urj_tap_register_init (dr->out, urj_tap_register_get_string (part->id));

    return URJ_STATUS_OK;
}

int
urj_part_data_register_realloc (urj_data_register_t *dr, int new_len)
{
    if (urj_tap_register_realloc (dr->in, new_len) == NULL)
        return URJ_STATUS_FAIL;
    if (urj_tap_register_realloc (dr->out, new_len) == NULL)
        return URJ_STATUS_FAIL;

    return URJ_STATUS_OK;
}
