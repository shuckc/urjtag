/*
 * $Id$
 *
 * Copyright (C) 2002, 2003 ETC s.r.o.
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
 * Written by Marcel Telka <marcel@telka.sk>, 2002, 2003.
 *
 */

#include <sysdep.h>

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#ifdef HAVE_LIBREADLINE
#include <readline/readline.h>
#endif

#include <urjtag/error.h>
#include <urjtag/chain.h>
#include <urjtag/parse.h>
#include <urjtag/cmd.h>
#include <urjtag/jtag.h>

#include "cmd.h"

const urj_cmd_t * const urj_cmds[] = {
#define _URJ_CMD(cmd) &urj_cmd_##cmd,
#include "cmd_list.h"
    NULL                        /* last must be NULL */
};

/*
 * @param text match commands whose prefix equals <code>text</code>. Rotates
 *      through the registered commands. The prefix length is set when the
 *      rotating state is reset. This is the behavior as dictated by readline.
 */
static const urj_cmd_t *
urj_cmd_find (const char *text, ssize_t last_idx)
{
    static size_t len;
    const urj_cmd_t *ret;

    if (last_idx == -1)
        len = strlen (text);

    while (urj_cmds[++last_idx])
    {
        ret = urj_cmds[last_idx];
        if (!strncmp (ret->name, text, len))
            return ret;
    }

    return NULL;
}

/* These three funcs are meant to be used by sub-command completers */
void
urj_completion_add_match (char ***matches, size_t *cnt, char *match)
{
    *matches = realloc (*matches, sizeof (**matches) * (*cnt + 2));
    (*matches)[(*cnt)++] = match;
}

void
urj_completion_add_match_dupe (char ***matches, size_t *cnt, const char *match)
{
    urj_completion_add_match (matches, cnt, strdup (match));
}

void
urj_completion_mayben_add_match (char ***matches, size_t *cnt, const char *text,
                                 size_t text_len, const char *match)
{
    if (!strncmp (text, match, text_len))
        urj_completion_add_match_dupe (matches, cnt, match);
}

void
urj_completion_mayben_add_matches_num (char ***matches, size_t *cnt,
                                       const char *text, size_t text_len,
                                       const char * const *matchs, size_t num)
{
    size_t n;

    for (n = 0; n < num; ++n)
        urj_completion_mayben_add_match (matches, cnt, text, text_len,
                                         matchs[n]);
}

void
urj_completion_mayben_add_param_list (char ***matches, size_t *cnt,
                                      const char *text, size_t text_len,
                                      urj_param_list_t param_list)
{
    size_t i;

    for (i = 0; i < param_list.n; ++i)
        urj_completion_mayben_add_match (matches, cnt, text, text_len,
                                         param_list.list[i].string);
}

void urj_completion_mayben_add_file (char ***matches, size_t *cnt,
                                     const char *text, size_t text_len,
                                     bool search)
{
#ifdef HAVE_LIBREADLINE
    int state;
    size_t implicit_len;
    char *match, *search_text;

    /* Use the search path if path isn't explicitly relative/absolute */
    if (search && text[0] != '/' && text[0] != '.')
    {
        const char *jtag_data_dir = urj_get_data_dir ();
        implicit_len = strlen (jtag_data_dir) + 1;

        search_text = malloc (implicit_len + text_len + 1);
        if (!search_text)
            return;

        sprintf (search_text, "%s/%s", jtag_data_dir, text);
        text = search_text;
        text_len += implicit_len;
    }
    else
    {
        implicit_len = 0;
        search_text = NULL;
    }

    state = 0;
    while (1)
    {
        match = rl_filename_completion_function (text, state++);
        if (!match)
            break;
        urj_completion_add_match_dupe (matches, cnt, match + implicit_len);
        free (match);
    }

    free (search_text);
#endif
}

void
urj_completion_maybe_add_match (char ***matches, size_t *cnt, const char *text,
                                const char *match)
{
    urj_completion_mayben_add_match (matches, cnt, text, strlen (text), match);
}

static size_t
urt_completion_find_token_point (const char *line, int point)
{
    const char *cs = line;
    size_t token_point = 0;

    /* Skip all leading whitespace first to make 2nd loop easier */
    while (isspace (*cs))
        ++cs;

    while (*cs)
    {
        if (point <= (cs - line))
            break;

        ++cs;

        if (isspace (*cs))
        {
            ++token_point;
            while (isspace (*cs))
                ++cs;
        }
    }

    return token_point;
}

char **
urj_cmd_complete (urj_chain_t *chain, const char *line, int point)
{
    char **tokens, **ret;
    size_t token_cnt, token_point, ret_cnt;
    const urj_cmd_t *cmd;
    const char *name;

    /* Split up the current line to make completion easier */
    if (urj_tokenize_line (line, &tokens, &token_cnt))
        return NULL;
    if (token_cnt == 0)
        name = "";
    else
        name = tokens[0];

    ret = NULL;
    ret_cnt = 0;

    /* Figure out which token we're pointing to */
    token_point = urt_completion_find_token_point (line, point);

    /* Are we completing the command itself ?  Re-use the 'help' ... */
    if (token_point == 0)
        name = "help";

    /* Figure out options for which command we want to complete */
    cmd = urj_cmd_find (name, -1);
    if (cmd && cmd->complete)
    {
        if (token_cnt)
            name = tokens[token_point] ? : "";
        else
            name = "";

        cmd->complete (chain, &ret, &ret_cnt, tokens, name,
                       strlen (name), token_point);

        if (ret_cnt)
            ret[ret_cnt] = NULL;
    }

    if (token_cnt)
        urj_tokens_free (tokens);

    return ret;
}

int
urj_cmd_test_cable (urj_chain_t *chain)
{
    if (chain->cable)
        return URJ_STATUS_OK;

    urj_error_set (URJ_ERROR_ILLEGAL_STATE,
                   "Cable not configured. Please use '%s' command first!",
                   "cable");
    return URJ_STATUS_FAIL;
}

/* Remainder copied from libbrux/cmd/cmd.c */

int
urj_cmd_run (urj_chain_t *chain, char *params[])
{
    int i, pidx;
    size_t len;

    if (!params[0])
        return URJ_STATUS_OK;

    pidx = -1;
    len = strlen (params[0]);

    for (i = 0; urj_cmds[i]; ++i)
    {
        if (strcasecmp (urj_cmds[i]->name, params[0]) == 0)
        {
          run_cmd:
            i = urj_cmds[i]->run (chain, params);
            if (i != URJ_STATUS_OK && urj_error_get () == URJ_ERROR_SYNTAX)
            {
                char *help_params[3] = { "help", params[0], NULL };
                urj_cmd_run( chain, help_params );
            }
            return i;
        }
        else if (strncasecmp (urj_cmds[i]->name, params[0], len) == 0)
        {
            if (pidx == -1)
                pidx = i;
            else
                pidx = -2;
        }
    }

    switch (pidx)
    {
    case -2:
        urj_log (URJ_LOG_LEVEL_NORMAL, _("%s: Ambiguous command\n"), params[0]);
        break;
    case -1:
        urj_log (URJ_LOG_LEVEL_NORMAL, _("%s: unknown command\n"), params[0]);
        break;
    default:
        i = pidx;
        goto run_cmd;
    }

    return URJ_STATUS_OK;
}

int
urj_cmd_params (char * const params[])
{
    int i = 0;

    while (params[i])
        i++;

    return i;
}

int
urj_cmd_get_number (const char *s, long unsigned *i)
{
    int n;
    int r;
    size_t l;

    if (!s || !i)
    {
        urj_error_set (URJ_ERROR_INVALID, "NULL string or int pointer");
        return URJ_STATUS_FAIL;
    }

    l = strlen (s);

    n = -1;
    r = sscanf (s, "0x%lx%n", i, &n);
    if (r == 1 && n == l)
        return URJ_STATUS_OK;

    n = -1;
    r = sscanf (s, "%lu%n", i, &n);
    if (r == 1 && n == l)
        return URJ_STATUS_OK;

    urj_error_set (URJ_ERROR_SYNTAX, "not a number: '%s'", s);

    return URJ_STATUS_FAIL;
}
