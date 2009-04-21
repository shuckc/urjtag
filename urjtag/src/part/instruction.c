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
#include <stdio.h>
#include <string.h>

#include "instruction.h"

instruction *
instruction_alloc (const char *name, int len, const char *val)
{
    instruction *i;

    if (!name || !val)
        return NULL;

    i = malloc (sizeof *i);
    if (!i)
        return NULL;

    if (strlen (name) > MAXLEN_INSTRUCTION)
        printf (_("Warning: Instruction name too long\n"));
    strncpy (i->name, name, MAXLEN_INSTRUCTION);
    i->name[MAXLEN_INSTRUCTION] = '\0';

    i->value = register_alloc (len);
    if (!i->value)
    {
        free (i);
        return NULL;
    }
    i->out = register_alloc (len);
    if (!i->out)
    {
        free (i->value);
        free (i);
        return NULL;
    }

    register_init (i->value, val);
    i->data_register = NULL;
    i->next = NULL;

    return i;
}

void
instruction_free (instruction * i)
{
    if (!i)
        return;

    if (i->value)
        register_free (i->value);
    if (i->out)
        register_free (i->out);
    free (i);
}
