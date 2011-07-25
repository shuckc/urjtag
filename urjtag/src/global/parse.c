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
 * Modified by Ajith Kumar P.C <ajithpc@kila.com>, 20/09/2006.
 * Modified by Ville Voipio <vv@iki.fi>, 7-May-2008
 *
 */

#include <sysdep.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include <urjtag/log.h>
#include <urjtag/error.h>
#include <urjtag/chain.h>
#include <urjtag/parse.h>
#include <urjtag/cmd.h>
#include <urjtag/jtag.h>
#include <urjtag/bsdl.h>

int
urj_tokenize_line (const char *line, char ***tokens, size_t *token_cnt)
{
    size_t l, i;
    int escape, quote_single, quote_double;
    char **a;
    const char *c;
    char *d, *sline;

    if (!line || !tokens || !token_cnt)
    {
        urj_error_set (URJ_ERROR_INVALID, "NULL input(s)");
        return URJ_STATUS_FAIL;
    }

    *token_cnt = 0;

    l = strlen (line);
    if (l == 0)
        return URJ_STATUS_OK;

    /* allocate as many chars as in the input line; this will be enough in all cases */
    sline = malloc (l + 1);
    if (sline == NULL)
    {
        urj_error_set (URJ_ERROR_OUT_OF_MEMORY, "malloc(%zd) fails",
                       (size_t) (l + 1));
        return URJ_STATUS_FAIL;
    }

    /* count and copy the tokens */
    escape = quote_single = quote_double = 0;
    c = line;
    d = sline;
    while (1)
    {
        /* eat up leading spaces */
        while (isspace (*c))
            c++;

        /* if the command ends here (either by NUL or comment) */
        if (*c == '\0' || *c == '#')
            break;

        /* copy the meat (non-space, non-NUL), consider escape and quotes */
        while ((!isspace (*c)
                || escape
                || quote_single
                || quote_double) && *c != '\0')
        {
            if (*c == '\'' && !escape && !quote_double)
            {
                quote_single ^= 1;
                c++;
            }
            else if (*c == '"' && !escape && !quote_single)
            {
                quote_double ^= 1;
                c++;
            }
            else if (*c == '\\' && !escape)
            {
                escape = 1;
                c++;
            }
            else
            {
                escape = 0;
                *d++ = *c++;
            }
        }
        /* mark the end to the destination string */
        *d++ = '\0';
        ++*token_cnt;
    }

    if (*token_cnt == 0)
    {
        free (sline);
        return URJ_STATUS_OK;
    }

    /* allocate the token pointer table */
    l = (*token_cnt + 1) * sizeof (*a);
    *tokens = a = malloc (l);
    if (a == NULL)
    {
        urj_error_set (URJ_ERROR_OUT_OF_MEMORY, "malloc(%zd) fails", l);
        return URJ_STATUS_FAIL;
    }

    /* find the starting points for the tokens */
    d = sline;
    for (i = 0; i < *token_cnt; i++)
    {
        a[i] = d;
        while (*d++ != '\0')
            ;
    }
    a[*token_cnt] = NULL;

    return URJ_STATUS_OK;
}

void
urj_tokens_free (char **tokens)
{
    free (tokens[0]);
    free (tokens);
}

int
urj_parse_line (urj_chain_t *chain, const char *line)
{
    int r;
    size_t tcnt;
    char **a;

    r = urj_tokenize_line (line, &a, &tcnt);
    if (r != URJ_STATUS_OK || tcnt == 0)
        return r;

    r = urj_cmd_run (chain, a);
    urj_log (URJ_LOG_LEVEL_DEBUG, "Return in urj_parse_line r=%d line={%s}\n",
             r, line);

    urj_tokens_free (a);

    return r;
}

int
urj_parse_stream (urj_chain_t *chain, FILE *f)
{
    char *inputline, *p;
    size_t len;
    int go;

    /* read the stream line-by-line until EOF or "quit" */
    inputline = NULL;
    do
    {
        if (getline (&inputline, &len, f) == -1)
        {
            if (!feof (f))
            {
                go = URJ_STATUS_FAIL;
                urj_warning ("getline() failed\n");
                break;
            }

            /* If we hit the end of the file, just return.  Do not
             * "quit" or fail as this will break things like nested
             * file includes.  */
            go = URJ_STATUS_OK;
            break;
        }
        else
        {
            p = strchr (inputline, '\n');
            if (p)
                *p = '\0';
        }

        go = urj_parse_line (chain, inputline);
        if (go == URJ_STATUS_FAIL)
        {
            urj_log (URJ_LOG_LEVEL_ERROR, "when parsing command '%s'\n", inputline);
            urj_log_error_describe (URJ_LOG_LEVEL_ERROR);
        }
        urj_tap_chain_flush (chain);
    }
    while (go != URJ_STATUS_MUST_QUIT);

    free (inputline);

    return go;
}

int
urj_parse_file (urj_chain_t *chain, const char *filename)
{
    FILE *f;
    int go;

    f = fopen (filename, FOPEN_R);
    if (!f)
    {
        urj_error_IO_set ("Cannot open file '%s' to parse", filename);
        return URJ_STATUS_FAIL;
    }

    go = urj_parse_stream (chain, f);

    fclose (f);
    urj_log (URJ_LOG_LEVEL_DEBUG, "File Closed go=%d\n", go);

    return go;
}

int
urj_parse_include (urj_chain_t *chain, const char *filename, int ignore_path)
{
    char *path = NULL;
    int r = URJ_STATUS_OK;

    if (! ignore_path)
    {
        /* If "filename" begins with a slash, or dots followed by a slash,
         * assume that user wants to ignore the search path after all */
        const char *slashdots = filename;

#ifdef __MINGW32__
        if (isalpha (*slashdots) && slashdots[1] == ':')
            slashdots += 2;
#endif
        while (*slashdots == '.')
            slashdots++;
        ignore_path = (*slashdots == '/' || *slashdots == '\\');
    }

    if (! ignore_path)
    {
        const char *jtag_data_dir = urj_get_data_dir ();
        size_t len;

        path = malloc (len = strlen (jtag_data_dir) + strlen (filename) + 2);
        if (path == NULL)
        {
            urj_error_set (URJ_ERROR_OUT_OF_MEMORY, "malloc(%zd) fails", len);
            return URJ_STATUS_FAIL;
        }
        snprintf (path, len, "%s/%s", jtag_data_dir, filename);

        filename = path;
    }

#ifdef ENABLE_BSDL
    /* perform a test read to check for BSDL syntax */
    if (urj_bsdl_read_file (chain, filename, URJ_BSDL_MODE_INCLUDE1, NULL) >= 0)
    {
        // @@@@ RFHH ToDo: let urj_bsdl_read_file also return URJ_STATUS_...
        /* it seems to be a proper BSDL file, so re-read and execute */
        if (urj_bsdl_read_file (chain, filename, URJ_BSDL_MODE_INCLUDE2,
                                NULL) < 0)
            // retain errno
            r = URJ_STATUS_FAIL;
    }
    else
#endif
    {
        r = urj_parse_file (chain, filename);
    }

    free (path);

    return r;
}
