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

#include <urjtag/error.h>
#include <urjtag/part.h>
#include <urjtag/bssignal.h>
#include <urjtag/tap_register.h>
#include <urjtag/part_instruction.h>
#include <urjtag/data_register.h>
#include <urjtag/bsbit.h>

urj_part_init_t *urj_part_inits = NULL;

/* part */

urj_part_t *
urj_part_alloc (const urj_tap_register_t *id)
{
    urj_part_t *p = malloc (sizeof *p);
    if (!p)
    {
        urj_error_set (URJ_ERROR_OUT_OF_MEMORY, "malloc(%zd) fails", sizeof *p);
        return NULL;
    }

    p->alias = NULL;            /* djf */
    /* @@@@ RFHH check result */
    p->id = urj_tap_register_duplicate (id);
    p->manufacturer[0] = '\0';
    p->part[0] = '\0';
    p->stepping[0] = '\0';
    p->signals = NULL;
    p->saliases = NULL;
    p->instruction_length = 0;
    p->instructions = NULL;
    p->active_instruction = NULL;
    p->data_registers = NULL;
    p->boundary_length = 0;
    p->bsbits = NULL;
    p->params = NULL;

    return p;
}

void
urj_part_free (urj_part_t *p)
{
    int i;

    if (!p)
        return;

    /* id */
    free (p->id);

    if (p->alias)
        free (p->alias);        /* djf */

    /* signals */
    while (p->signals)
    {
        urj_part_signal_t *s = p->signals;
        p->signals = s->next;
        urj_part_signal_free (s);
    }

    /* saliases */
    while (p->saliases)
    {
        urj_part_salias_t *sa = p->saliases;
        p->saliases = sa->next;
        urj_part_salias_free (sa);
    }

    /* instructions */
    while (p->instructions)
    {
        urj_part_instruction_t *i = p->instructions;
        p->instructions = i->next;
        urj_part_instruction_free (i);
    }

    /* data registers */
    while (p->data_registers)
    {
        urj_data_register_t *dr = p->data_registers;
        p->data_registers = dr->next;
        urj_part_data_register_free (dr);
    }

    /* bsbits */
    for (i = 0; i < p->boundary_length; i++)
        urj_part_bsbit_free (p->bsbits[i]);
    free (p->bsbits);

    if (p->params && p->params->free)
        p->params->free (p->params->data);
    free (p->params);

    free (p);
}

urj_part_instruction_t *
urj_part_find_instruction (urj_part_t *p, const char *iname)
{
    urj_part_instruction_t *i;

    if (!p || !iname)
    {
        urj_error_set (URJ_ERROR_INVALID, "NULL part or instruction name");
        return NULL;
    }

    i = p->instructions;
    while (i)
    {
        if (strcasecmp (iname, i->name) == 0)
            break;
        i = i->next;
    }

    return i;
}

urj_data_register_t *
urj_part_find_data_register (urj_part_t *p, const char *drname)
{
    urj_data_register_t *dr;

    if (!p || !drname)
    {
        urj_error_set (URJ_ERROR_INVALID, "NULL part or data register name");
        return NULL;
    }

    dr = p->data_registers;
    while (dr)
    {
        if (strcasecmp (drname, dr->name) == 0)
            break;
        dr = dr->next;
    }

    return dr;
}

urj_part_signal_t *
urj_part_find_signal (urj_part_t *p, const char *signalname)
{
    urj_part_signal_t *s;
    urj_part_salias_t *sa;

    if (!p || !signalname)
    {
        urj_error_set (URJ_ERROR_INVALID, "NULL part or signal name");
        return NULL;
    }

    s = p->signals;
    while (s)
    {
        if (strcasecmp (signalname, s->name) == 0)
            return s;
        s = s->next;
    }

    sa = p->saliases;
    while (sa)
    {
        if (strcasecmp (signalname, sa->name) == 0)
            return sa->signal;
        sa = sa->next;
    }

    return NULL;
}

void
urj_part_set_instruction (urj_part_t *p, const char *iname)
{
    if (p)
        p->active_instruction = urj_part_find_instruction (p, iname);
}

int
urj_part_set_signal (urj_part_t *p, urj_part_signal_t *s, int out, int val)
{
    urj_data_register_t *bsr;

    if (!p || !s)
    {
        urj_error_set (URJ_ERROR_INVALID, "NULL part or signal");
        return URJ_STATUS_FAIL;
    }

    /* search for Boundary Scan Register */
    bsr = urj_part_find_data_register (p, "BSR");
    if (!bsr)
    {
        urj_error_set (URJ_ERROR_NOTFOUND,
                       _("Boundary Scan Register (BSR) not found"));
        return URJ_STATUS_FAIL;
    }

    /* setup signal */
    if (out)
    {
        int control;
        if (!s->output)
        {
            urj_error_set (URJ_ERROR_INVALID,
                           _("signal '%s' cannot be set as output"), s->name);
            return URJ_STATUS_FAIL;
        }
        bsr->in->data[s->output->bit] = val & 1;

        control = p->bsbits[s->output->bit]->control;
        if (control >= 0)
            bsr->in->data[control] =
                p->bsbits[s->output->bit]->control_value ^ 1;
    }
    else
    {
        if (!s->input)
        {
            urj_error_set (URJ_ERROR_INVALID,
                           _("signal '%s' cannot be set as input"), s->name);
            return URJ_STATUS_FAIL;
        }
        if (s->output)
            bsr->in->data[s->output->control] =
                p->bsbits[s->output->bit]->control_value;
    }

    return URJ_STATUS_OK;
}

int
urj_part_get_signal (urj_part_t *p, const urj_part_signal_t *s)
{
    urj_data_register_t *bsr;

    if (!p || !s)
    {
        urj_error_set (URJ_ERROR_INVALID, "NULL part or signal");
        return -1;
    }

    /* search for Boundary Scan Register */
    bsr = urj_part_find_data_register (p, "BSR");
    if (!bsr)
    {
        urj_error_set (URJ_ERROR_NOTFOUND,
                       _("Boundary Scan Register (BSR) not found"));
        return -1;
    }

    if (!s->input)
    {
        urj_error_set (URJ_ERROR_INVALID,
                       _("signal '%s' is not input signal"), s->name);
        return -1;
    }

    return bsr->out->data[s->input->bit];
}

int
urj_part_print (urj_log_level_t ll, urj_part_t *p)
{
    const char *instruction = NULL;
    const char *dr = NULL;
    char format[100];

    if (!p)
    {
        urj_error_set (URJ_ERROR_INVALID, "NULL part");
        return URJ_STATUS_FAIL;
    }

    snprintf (format, 100, _("%%-%ds %%-%ds %%-%ds %%-%ds %%-%ds\n"),
              URJ_PART_MANUFACTURER_MAXLEN, URJ_PART_PART_MAXLEN,
              URJ_PART_STEPPING_MAXLEN, URJ_INSTRUCTION_MAXLEN_INSTRUCTION,
              URJ_DATA_REGISTER_MAXLEN);

    if (p->active_instruction)
    {
        instruction = p->active_instruction->name;
        if (p->active_instruction->data_register != NULL)
            dr = p->active_instruction->data_register->name;
    }
    if (instruction == NULL)
        instruction = _("(none)");
    if (dr == NULL)
        dr = _("(none)");
    urj_log (ll, format, p->manufacturer, p->part, p->stepping, instruction,
             dr);

    return URJ_STATUS_OK;
}


int
urj_part_instruction_length_set (urj_part_t *part, int length)
{
    if (part->instructions != NULL)
    {
        urj_error_set (URJ_ERROR_ALREADY,
                       _("instruction length is already set and used"));
        return URJ_STATUS_FAIL;
    }

    part->instruction_length = length;

    return URJ_STATUS_OK;
}


urj_part_instruction_t *
urj_part_instruction_define (urj_part_t *part, const char *instruction,
                             const char *code, const char *data_register)
{
    urj_part_instruction_t *i;
    urj_data_register_t *dr;

    if (strlen (code) != part->instruction_length)
    {
        urj_error_set (URJ_ERROR_INVALID,
                       _("invalid instruction length"));
        return NULL;
    }

    if (urj_part_find_instruction (part, instruction) != NULL)
    {
        urj_error_set (URJ_ERROR_ALREADY,
                       _("Instruction '%s' already defined"), instruction);
        return NULL;
    }

    dr = urj_part_find_data_register (part, data_register);
    if (dr == NULL)
    {
        urj_error_set(URJ_ERROR_NOTFOUND,
                      _("unknown data register '%s'"), data_register);
        return NULL;
    }

    i = urj_part_instruction_alloc (instruction, part->instruction_length,
                                    code);
    if (!i)
    {
        /* retain error state */
        return NULL;
    }

    i->next = part->instructions;
    part->instructions = i;

    i->data_register = dr;

    return i;
}


/* parts */

urj_parts_t *
urj_part_parts_alloc (void)
{
    urj_parts_t *ps = malloc (sizeof *ps);
    if (!ps)
    {
        urj_error_set (URJ_ERROR_OUT_OF_MEMORY, "malloc(%zd) fails",
                       sizeof *ps);
        return NULL;
    }

    ps->len = 0;
    ps->parts = NULL;

    return ps;
}

void
urj_part_parts_free (urj_parts_t *ps)
{
    int i;

    if (!ps)
        return;

    for (i = 0; i < ps->len; i++)
        urj_part_free (ps->parts[i]);

    free (ps->parts);
    free (ps);
}

int
urj_part_parts_add_part (urj_parts_t *ps, urj_part_t *p)
{
    urj_part_t **np = realloc (ps->parts, (ps->len + 1) * sizeof *ps->parts);

    if (!np)
    {
        urj_error_set (URJ_ERROR_OUT_OF_MEMORY, "realloc(%s,%zd) fails",
                       "ps->parts", (ps->len + 1) * sizeof *ps->parts);
        return URJ_STATUS_FAIL;
    }

    ps->parts = np;
    ps->parts[ps->len++] = p;

    return URJ_STATUS_OK;
}

int
urj_part_parts_set_instruction (urj_parts_t *ps, const char *iname)
{
    int i;

    if (!ps)
    {
        urj_error_set (URJ_ERROR_INVALID, "NULL parts");
        return URJ_STATUS_FAIL;
    }

    for (i = 0; i < ps->len; i++)
        ps->parts[i]->active_instruction =
            urj_part_find_instruction (ps->parts[i], iname);

    return URJ_STATUS_OK;
}

int
urj_part_parts_print (urj_log_level_t ll, urj_parts_t *ps, int active_part)
{
    int i;

    if (!ps)
    {
        urj_error_set (URJ_ERROR_INVALID, "NULL parts");
        return URJ_STATUS_FAIL;
    }

    for (i = 0; i < ps->len; i++)
    {
        urj_part_t *p = ps->parts[i];

        if (!p)
            continue;

        urj_log (ll, "%s%3d ", i == active_part ? "*" : " ", i);
        urj_part_print (ll, p);
    }

    return URJ_STATUS_OK;
}

void
urj_part_init_register (char *part, urj_part_init_func_t init)
{
    urj_part_init_t *pi;

    pi = (urj_part_init_t *) malloc (sizeof (urj_part_init_t));
    strncpy (pi->part, part, URJ_PART_PART_MAXLEN);
    pi->init = init;
    pi->next = urj_part_inits;
    urj_part_inits = pi;
}

urj_part_init_func_t
urj_part_find_init (char *part)
{
    urj_part_init_t *pi;

    for (pi = urj_part_inits; pi; pi = pi->next)
        if (strcmp (pi->part, part) == 0)
            return pi->init;

    return NULL;
}
