/src*
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
 * Written by Marcel Telka <marcel@telka.sk>, 2003.
 *
 */

#include <config.h>

#include "sysdep.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef HAVE_WCHAR_H
#include <wchar.h>
#else
typedef char wchar_t;
# define mbstowcs(dst,src,n) 0
# define wcslen(str) strlen(str)
#endif

#include "part.h"
#include "jtag.h"

#include "cmd.h"

static int
cmd_print_run (chain_t *chain, char *params[])
{
    char format[128];
#if HAVE_SWPRINTF
    wchar_t wformat[128];
#endif /* HAVE_SWPRINTF */
    wchar_t wheader[128];
    char header[128];
    int i;
    int noheader = 0;

    if (cmd_params (params) > 2)
        return -1;

    if (!cmd_test_cable (chain))
        return 1;

    if (!chain->parts)
    {
        printf (_("Run \"detect\" first.\n"));
        return 1;
    }

    if (cmd_params (params) == 2)
    {
        if (strcasecmp (params[1], "bus") == 0)
            noheader = 1;

        if (strcasecmp (params[1], "signals") == 0)
        {

            printf ("Signals:\n");
            part_t *part;
            signal_t *s;
            part = chain->parts->parts[chain->active_part];
            for (s = part->signals; s != NULL; s = s->next)
            {
                salias_t *sa;
                if (s->pin)
                    printf ("%s %s", s->name, s->pin);
                else
                    printf ("%s", s->name);
                if (s->input)
                    printf ("\tinput=%s", s->input->name);
                if (s->output)
                    printf ("\toutput=%s", s->output->name);

                for (sa = part->saliases; sa != NULL; sa = sa->next)
                {
                    if (s == sa->signal)
                        printf ("\tsalias=%s", sa->name);
                }
                printf ("\n");
            }
            return (1);
        }

        if (strcasecmp (params[1], "instructions") == 0)
        {
            part_t *part;
            instruction_t *inst;

            snprintf (format, 128, _(" Active %%-%ds %%-%ds"),
                      MAXLEN_INSTRUCTION, MAXLEN_DATA_REGISTER);
#if HAVE_SWPRINTF
            if (mbstowcs (wformat, format, 128) == -1)
                printf (_("(%d) String conversion failed!\n"), __LINE__);
            swprintf (wheader, 128, wformat, _("Instruction"), _("Register"));
            if (wcstombs (header, wheader, 128) == -1)
                printf (_("(%d) String conversion failed!\n"), __LINE__);
#else /* HAVE_SWPRINTF */
            snprintf (header, 128, format, _("Instruction"), _("Register"));
            if (mbstowcs (wheader, header, 128) == -1)
                printf (_("(%d) String conversion failed!\n"), __LINE__);
#endif /* HAVE_SWPRINTF */
            puts (header);

            for (i = 0; i < wcslen (wheader); i++)
                putchar ('-');
            putchar ('\n');

            snprintf (format, 128, _("   %%c    %%-%ds %%-%ds\n"),
                      MAXLEN_INSTRUCTION, MAXLEN_DATA_REGISTER);

            part = chain->parts->parts[chain->active_part];
            for (inst = part->instructions; inst != NULL; inst = inst->next)
            {
                printf (format,
                        inst == part->active_instruction
                        ? 'X' : ' ', inst->name, inst->data_register->name);
            }
            return (1);
        }
    }

    if (noheader == 0)
    {
        snprintf (format, 128, _(" No. %%-%ds %%-%ds %%-%ds %%-%ds %%-%ds"),
                  MAXLEN_MANUFACTURER, MAXLEN_PART, MAXLEN_STEPPING,
                  MAXLEN_INSTRUCTION, MAXLEN_DATA_REGISTER);
#if HAVE_SWPRINTF
        if (mbstowcs (wformat, format, 128) == -1)
            printf (_("(%d) String conversion failed!\n"), __LINE__);
        swprintf (wheader, 128, wformat, _("Manufacturer"), _("Part"),
                  _("Stepping"), _("Instruction"), _("Register"));
        if (wcstombs (header, wheader, 128) == -1)
            printf (_("(%d) String conversion failed!\n"), __LINE__);
#else /* HAVE_SWPRINTF */
        snprintf (header, 128, format, _("Manufacturer"), _("Part"),
                  _("Stepping"), _("Instruction"), _("Register"));
        if (mbstowcs (wheader, header, 128) == -1)
            printf (_("(%d) String conversion failed!\n"), __LINE__);
#endif /* HAVE_SWPRINTF */
        puts (header);

        for (i = 0; i < wcslen (wheader); i++)
            putchar ('-');
        putchar ('\n');
    }

    if (cmd_params (params) == 1)
    {
        if (chain->parts->len > chain->active_part)
        {
            if (chain->parts->parts[chain->active_part]->alias)
                printf (_(" %3d %s "), chain->active_part,
                        chain->parts->parts[chain->active_part]->alias);
            else
                printf (_(" %3d "), chain->active_part);

            part_print (chain->parts->parts[chain->active_part]);
        }
        if (bus != NULL)
        {
            int i;
            uint64_t a;
            bus_area_t area;

            for (i = 0; i < buses.len; i++)
                if (buses.buses[i] == bus)
                    break;
            printf (_("\nActive bus:\n*%d: "), i);
            bus_printinfo (bus);

            for (a = 0; a < UINT64_C (0x100000000);
                 a = area.start + area.length)
            {
                if (bus_area (bus, a, &area) != URJTAG_STATUS_OK)
                {
                    printf (_("Error in bus area discovery at 0x%08llX\n"),
                            (long long unsigned int) a);
                    break;
                }
                if (area.width != 0)
                {
                    if (area.description != NULL)
                        printf (_
                                ("\tstart: 0x%08X, length: 0x%08llX, data width: %d bit, (%s)\n"),
                                area.start,
                                (long long unsigned int) area.length,
                                area.width, _(area.description));
                    else
                        printf (_
                                ("\tstart: 0x%08X, length: 0x%08llX, data width: %d bit\n"),
                                area.start,
                                (long long unsigned int) area.length,
                                area.width);
                }
            }
        }

        return 1;
    }

    if (strcasecmp (params[1], "chain") == 0)
    {
        parts_print (chain->parts);
        return 1;
    }

    for (i = 0; i < buses.len; i++)
    {
        if (buses.buses[i] == bus)
            printf (_("*%d: "), i);
        else
            printf (_("%d: "), i);
        bus_printinfo (buses.buses[i]);
    }

    return 1;
}

static void
cmd_print_help (void)
{
    printf (_("Usage: %s [chain|bus|signals|instructions]\n"
              "Display JTAG chain status.\n"
              "\n"
              "Display list of the parts connected to the JTAG chain including\n"
              "part number and current (active) instruction and data register.\n"),
            "print");
}

cmd_t cmd_print = {
    "print",
    N_("display JTAG chain list/status"),
    cmd_print_help,
    cmd_print_run
};
