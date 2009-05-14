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
#include <urjtag/bssignal.h>
#include <urjtag/part.h>

urj_part_signal_t *
urj_part_signal_alloc (const char *name)
{
    urj_part_signal_t *s = malloc (sizeof *s);
    if (!s)
    {
        urj_error_set (URJ_ERROR_OUT_OF_MEMORY, "malloc(%zd) fails", sizeof *s);
        return NULL;
    }

    s->name = strdup (name);
    if (!s->name)
    {
        free (s);
        urj_error_set (URJ_ERROR_OUT_OF_MEMORY, "strdup(%s) fails", name);
        return NULL;
    }
    s->pin = NULL;              /* djf hack pin number */
    s->next = NULL;
    s->input = NULL;
    s->output = NULL;

    return s;
}

void
urj_part_signal_free (urj_part_signal_t *s)
{
    if (!s)
        return;
    free (s->name);
    free (s);
}

urj_part_salias_t *
urj_part_salias_alloc (const char *name, const urj_part_signal_t *signal)
{
    urj_part_salias_t *sa = malloc (sizeof *sa);

    if (sa == NULL)
    {
        urj_error_set (URJ_ERROR_OUT_OF_MEMORY, "malloc(%zd) fails",
                       sizeof *sa);
        return NULL;
    }

    sa->name = strdup (name);
    if (sa->name == NULL)
    {
        free (sa);
        urj_error_set (URJ_ERROR_OUT_OF_MEMORY, "strdup(%s) fails", name);
        return NULL;
    }
    sa->next = NULL;
    sa->signal = (urj_part_signal_t *) signal;

    return sa;
}

void
urj_part_salias_free (urj_part_salias_t *salias)
{
    if (salias == NULL)
        return;
    free (salias->name);
    free (salias);
}

urj_part_signal_t *
urj_part_signal_define_pin (urj_chain_t *chain, const char *signal_name,
                            const char *pin_name)
{
    urj_part_t *part;
    urj_part_signal_t *s;

    part = urj_tap_chain_active_part (chain);

    if (urj_part_find_signal (part, signal_name) != NULL)
    {
        urj_error_set (URJ_ERROR_ALREADY,
                       _("Signal '%s' already defined"), signal_name);
        return NULL;
    }

    s = urj_part_signal_alloc (signal_name);
    if (!s)
        return NULL;

    if (pin_name != NULL)
    {                           /* Add pin number */
        /* Allocate the space for the pin number & copy it */
        s->pin = strdup (pin_name);
        if (s->pin == NULL)
        {
            urj_part_signal_free (s);
            urj_error_set (URJ_ERROR_OUT_OF_MEMORY, "strdup(%s) fails",
                           pin_name);
            return NULL;
        }
    }

    s->next = part->signals;
    part->signals = s;

    return s;
}

urj_part_signal_t *
urj_part_signal_define (urj_chain_t *chain, const char *signal_name)
{
    return urj_part_signal_define_pin(chain, signal_name, NULL);
}

int
urj_part_signal_redefine_pin (urj_chain_t *chain, urj_part_signal_t *s,
                              const char *pin_name)
{
    /* @@@@ RFHH Check s != NULL */
    free(s->pin);

    /* Allocate the space for the pin number & copy it */
    s->pin = strdup (pin_name);
    if (s->pin == NULL)
    {
        urj_error_set (URJ_ERROR_OUT_OF_MEMORY, "strdup(%s) fails", pin_name);
        return URJ_STATUS_FAIL;
    }

    return URJ_STATUS_OK;
}
