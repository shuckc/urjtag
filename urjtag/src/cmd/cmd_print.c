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
 * Written by Marcel Telka <marcel@telka.sk>, 2003.
 *
 */

#include <sysdep.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef HAVE_WCHAR_H
#include <wchar.h>
#else
# define mbstowcs(dst,src,n) 0
# define wcslen(str) strlen(str)
#endif

#include <urjtag/error.h>
#include <urjtag/chain.h>
#include <urjtag/part.h>
#include <urjtag/bus.h>
#include <urjtag/data_register.h>
#include <urjtag/part_instruction.h>
#include <urjtag/bssignal.h>
#include <urjtag/bsbit.h>
#include <urjtag/jtag.h>

#include <urjtag/cmd.h>

#include "cmd.h"

static int
cmd_print_run (urj_chain_t *chain, char *params[])
{
#define FORMAT_LENGTH   128
    char format[FORMAT_LENGTH];
#if HAVE_SWPRINTF
    wchar_t wformat[FORMAT_LENGTH];
#endif /* HAVE_SWPRINTF */
    wchar_t wheader[FORMAT_LENGTH];
    char header[FORMAT_LENGTH];
    int i;
    int noheader = 0;

    if (urj_cmd_params (params) > 2)
    {
        urj_error_set (URJ_ERROR_SYNTAX,
                       "%s: #parameters should be <= %d, not %d",
                       params[0], 2, urj_cmd_params (params));
        return URJ_STATUS_FAIL;
    }

    if (urj_cmd_test_cable (chain) != URJ_STATUS_OK)
        return URJ_STATUS_FAIL;

    if (!chain->parts)
    {
        urj_error_set (URJ_ERROR_ILLEGAL_STATE, "Run \"detect\" first");
        return URJ_STATUS_FAIL;
    }

    if (urj_cmd_params (params) == 2)
    {
        if (strcasecmp (params[1], "bus") == 0)
            noheader = 1;

        if (strcasecmp (params[1], "signals") == 0)
        {

            urj_log (URJ_LOG_LEVEL_NORMAL, "Signals:\n");
            urj_part_t *part;
            urj_part_signal_t *s;
            part = chain->parts->parts[chain->active_part];
            for (s = part->signals; s != NULL; s = s->next)
            {
                urj_part_salias_t *sa;
                if (s->pin)
                    urj_log (URJ_LOG_LEVEL_NORMAL, "%s %s", s->name, s->pin);
                else
                    urj_log (URJ_LOG_LEVEL_NORMAL, "%s", s->name);
                if (s->input)
                    urj_log (URJ_LOG_LEVEL_NORMAL, "\tinput=%s",
                             s->input->name);
                if (s->output)
                    urj_log (URJ_LOG_LEVEL_NORMAL, "\toutput=%s",
                             s->output->name);

                for (sa = part->saliases; sa != NULL; sa = sa->next)
                {
                    if (s == sa->signal)
                        urj_log (URJ_LOG_LEVEL_NORMAL, "\tsalias=%s", sa->name);
                }
                urj_log (URJ_LOG_LEVEL_NORMAL, "\n");
            }
            return URJ_STATUS_OK;
        }

        if (strcasecmp (params[1], "instructions") == 0)
        {
            urj_part_t *part;
            urj_part_instruction_t *inst;

            snprintf (format, sizeof format, _(" Active %%-%ds %%-%ds\n"),
                      URJ_INSTRUCTION_MAXLEN_INSTRUCTION,
                      URJ_DATA_REGISTER_MAXLEN);
#if HAVE_SWPRINTF
            if (mbstowcs (wformat, format, sizeof format) == -1)
                // @@@@ RFHH throw urj_error?
                printf (_("(%d) String conversion failed!\n"), __LINE__);
            swprintf (wheader, sizeof format, wformat, _("Instruction"), _("Register"));
            if (wcstombs (header, wheader, sizeof format) == -1)
                // @@@@ RFHH throw urj_error?
                printf (_("(%d) String conversion failed!\n"), __LINE__);
#else /* HAVE_SWPRINTF */
            snprintf (header, sizeof format, format, _("Instruction"), _("Register"));
            if (mbstowcs (wheader, header, sizeof format) == -1)
                // @@@@ RFHH throw urj_error?
                printf (_("(%d) String conversion failed!\n"), __LINE__);
#endif /* HAVE_SWPRINTF */
            urj_log (URJ_LOG_LEVEL_NORMAL, "%s", header);

            for (i = 0; i < wcslen (wheader); i++)
                urj_log (URJ_LOG_LEVEL_NORMAL, "%c", '-');
            urj_log (URJ_LOG_LEVEL_NORMAL, "%c", '\n');

            snprintf (format, sizeof format, _("   %%c    %%-%ds %%-%ds\n"),
                      URJ_INSTRUCTION_MAXLEN_INSTRUCTION,
                      URJ_DATA_REGISTER_MAXLEN);

            part = chain->parts->parts[chain->active_part];
            for (inst = part->instructions; inst != NULL; inst = inst->next)
            {
                urj_log (URJ_LOG_LEVEL_NORMAL, format,
                         (inst == part->active_instruction) ? 'X' : ' ',
                         inst->name, inst->data_register->name);
            }
            return URJ_STATUS_OK;
        }
    }

    if (noheader == 0)
    {
        snprintf (format, sizeof format,
                  _(" No. %%-%ds %%-%ds %%-%ds %%-%ds %%-%ds\n"),
                  URJ_PART_MANUFACTURER_MAXLEN, URJ_PART_PART_MAXLEN,
                  URJ_PART_STEPPING_MAXLEN,
                  URJ_INSTRUCTION_MAXLEN_INSTRUCTION,
                  URJ_DATA_REGISTER_MAXLEN);
#if HAVE_SWPRINTF
        if (mbstowcs (wformat, format, sizeof format) == -1)
            // @@@@ RFHH throw urj_error?
            printf (_("(%d) String conversion failed!\n"), __LINE__);
        swprintf (wheader, sizeof format, wformat, _("Manufacturer"), _("Part"),
                  _("Stepping"), _("Instruction"), _("Register"));
        if (wcstombs (header, wheader, sizeof format) == -1)
            // @@@@ RFHH throw urj_error?
            printf (_("(%d) String conversion failed!\n"), __LINE__);
#else /* HAVE_SWPRINTF */
        snprintf (header, sizeof format, format, _("Manufacturer"), _("Part"),
                  _("Stepping"), _("Instruction"), _("Register"));
        if (mbstowcs (wheader, header, sizeof format) == -1)
            // @@@@ RFHH throw urj_error?
            printf (_("(%d) String conversion failed!\n"), __LINE__);
#endif /* HAVE_SWPRINTF */
        urj_log (URJ_LOG_LEVEL_NORMAL, "%s", header);

        for (i = 0; i < wcslen (wheader); i++)
            urj_log (URJ_LOG_LEVEL_NORMAL, "%c", '-');
        urj_log (URJ_LOG_LEVEL_NORMAL, "%c", '\n');
    }

    if (urj_cmd_params (params) == 1)
    {
        int r = URJ_STATUS_OK;

        if (chain->parts->len > chain->active_part)
        {
            if (chain->parts->parts[chain->active_part]->alias)
                urj_log (URJ_LOG_LEVEL_NORMAL, _(" %3d %s "),
                         chain->active_part,
                         chain->parts->parts[chain->active_part]->alias);
            else
                urj_log (URJ_LOG_LEVEL_NORMAL, _(" %3d "), chain->active_part);

            urj_part_print (URJ_LOG_LEVEL_NORMAL,
                            chain->parts->parts[chain->active_part]);
        }
        if (urj_bus != NULL)
        {
            int i;
            uint64_t a;
            urj_bus_area_t area;

            for (i = 0; i < urj_buses.len; i++)
                if (urj_buses.buses[i] == urj_bus)
                    break;
            urj_log (URJ_LOG_LEVEL_NORMAL, _("\nActive bus:\n*%d: "), i);
            URJ_BUS_PRINTINFO (URJ_LOG_LEVEL_NORMAL, urj_bus);

            for (a = 0; a < UINT64_C (0x100000000);
                 a = area.start + area.length)
            {
                r = URJ_BUS_AREA (urj_bus, a, &area);
                if (r != URJ_STATUS_OK)
                {
                    urj_log (URJ_LOG_LEVEL_NORMAL,
                             _("Error in bus area discovery at 0x%08llX\n"),
                             (long long unsigned int) a);
                    break;
                }
                if (area.width != 0)
                {
                    if (area.description != NULL)
                        urj_log (URJ_LOG_LEVEL_NORMAL,
                                 _("\tstart: 0x%08lX, length: 0x%08llX, data width: %d bit, (%s)\n"),
                                 (long unsigned) area.start,
                                 (long long unsigned int) area.length,
                                 area.width, _(area.description));
                    else
                        urj_log (URJ_LOG_LEVEL_NORMAL,
                                 _("\tstart: 0x%08lX, length: 0x%08llX, data width: %d bit\n"),
                                 (long unsigned) area.start,
                                 (long long unsigned int) area.length,
                                 area.width);
                }
            }
        }

        return r;
    }

    if (strcasecmp (params[1], "chain") == 0)
    {
        urj_part_parts_print (URJ_LOG_LEVEL_NORMAL, chain->parts, chain->active_part);
        return URJ_STATUS_OK;
    }

    for (i = 0; i < urj_buses.len; i++)
    {
        if (urj_buses.buses[i] == urj_bus)
            urj_log (URJ_LOG_LEVEL_NORMAL, _("*%d: "), i);
        else
            urj_log (URJ_LOG_LEVEL_NORMAL, _("%d: "), i);
        URJ_BUS_PRINTINFO (URJ_LOG_LEVEL_NORMAL, urj_buses.buses[i]);
    }

    return URJ_STATUS_OK;
}

static void
cmd_print_help (void)
{
    urj_log (URJ_LOG_LEVEL_NORMAL,
             _("Usage: %s [chain|bus|signals|instructions]\n"
               "Display JTAG chain status.\n"
               "\n"
               "Display list of the parts connected to the JTAG chain including\n"
               "part number and current (active) instruction and data register.\n"),
             "print");
}

static void
cmd_print_complete (urj_chain_t *chain, char ***matches, size_t *match_cnt,
                    char * const *tokens, const char *text, size_t text_len,
                    size_t token_point)
{
    static const char * const status[] = {
        "chain",
        "bus",
        "signals",
        "instructions",
    };

    if (token_point != 1)
        return;

    urj_completion_mayben_add_matches (matches, match_cnt, text, text_len,
                                       status);
}

const urj_cmd_t urj_cmd_print = {
    "print",
    N_("display JTAG chain list/status"),
    cmd_print_help,
    cmd_print_run,
    cmd_print_complete,
};
