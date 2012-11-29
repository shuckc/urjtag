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
#include <string.h>

#include <urjtag/error.h>
#include <urjtag/params.h>

static const char *
urj_param_key_string(const urj_param_list_t *params, int key)
{
    int         i;

    for (i = 0; i < params->n; i++)
        if (key == params->list[i].key)
            return params->list[i].string;

    return "<no such bus parameter key>";
}

static int
urj_param_parse_key(const urj_param_list_t *params, const char *p)
{
    int i;
    const char *eq;

    eq = strchr(p, '=');
    if (eq == NULL)
        eq = p + strlen(p);

    for (i = 0; i < params->n; i++)
        if (strncasecmp(params->list[i].string, p, eq - p) == 0)
            return params->list[i].key;

    urj_error_set (URJ_ERROR_SYNTAX, "unrecognized param key '%s'", p);
    return -1;
}

static urj_param_type_t
urj_param_type_of(const urj_param_list_t *params, int key)
{
    int i;

    for (i = 0; i < params->n; i++)
        if (params->list[i].key == key)
            return params->list[i].type;

    urj_error_set (URJ_ERROR_INVALID, "unknown key %d", key);
    return -1;
}

static urj_param_t *
urj_param_increase (const urj_param_t ***bp)
{
    size_t      n;
    const urj_param_t **scan;
    urj_param_t *new_p;

    scan = *bp;
    for (n = 0; scan[n] != NULL; n++)
    {
        // count
    }

    scan = realloc (*bp, (n + 2) * sizeof *scan);
    if (scan == NULL)
    {
        urj_error_set (URJ_ERROR_OUT_OF_MEMORY, "realloc(%s,%zd) fails",
                       "*bp", (n + 2) * sizeof *scan);
        return NULL;
    }

    *bp = scan;

    new_p = malloc (sizeof *new_p);
    if (new_p == NULL)
    {
        urj_error_set (URJ_ERROR_OUT_OF_MEMORY, "malloc(%zd) fails",
                       sizeof *new_p);
        return NULL;
    }

    (*bp)[n] = new_p;
    (*bp)[n + 1] = NULL;

    return new_p;
}

static int
urj_param_decrease (const urj_param_t ***bp)
{
    size_t      n;
    const urj_param_t **scan;

    scan = *bp;
    for (n = 0; scan[n] != NULL; n++)
    {
        // count
    }

    if (n > 0)
    {
        --n;
        free ((void *) (*bp)[n]);       // Yes, I know why I cast
        (*bp)[n] = NULL;
    }

    return URJ_STATUS_OK;
}

int
urj_param_push_string (const urj_param_t ***bp, int key, const char *val)
{
    urj_param_t *new_p = urj_param_increase (bp);

    if (new_p == NULL)
        return URJ_STATUS_FAIL;

    new_p->type = URJ_PARAM_TYPE_STRING;
    new_p->key  = key;
    new_p->value.string = val;

    return URJ_STATUS_OK;
}

int
urj_param_push_lu (const urj_param_t ***bp, int key, long unsigned val)
{
    urj_param_t *new_p = urj_param_increase (bp);

    if (new_p == NULL)
        return URJ_STATUS_FAIL;

    new_p->type = URJ_PARAM_TYPE_LU;
    new_p->key  = key;
    new_p->value.lu = val;

    return URJ_STATUS_OK;
}

int
urj_param_push_bool (const urj_param_t ***bp, int key, int val)
{
    urj_param_t *new_p = urj_param_increase (bp);

    if (new_p == NULL)
        return URJ_STATUS_FAIL;

    new_p->type = URJ_PARAM_TYPE_BOOL;
    new_p->key  = key;
    new_p->value.enabled = val;

    return URJ_STATUS_OK;
}

static int
parse_param_lu(const char *eq, long unsigned *lu)
{
    eq += 1;

    /* Handle hex values as well as decimal.  While the C library will take
     * care of this for us if we used the 'i' conversion modifier, that takes
     * us into the signed/unsigned world.  Unfortunately, the 'u' conversion
     * modifier doesn't handle hex values transparently.  So do it ourselves.
     */
    if (strncmp(eq, "0x", 2))
    {
        /* Detect non-numeric chars. */
        char c;
        if (sscanf(eq, "%lu%c", lu, &c) == 1)
            return URJ_STATUS_OK;
    }
    else
    {
        if (sscanf(eq, "%lx", lu) == 1)
            return URJ_STATUS_OK;
    }

    urj_error_set (URJ_ERROR_SYNTAX,
                   "%s: could not parse number (hex values start with 0x)",
                   eq);
    return URJ_STATUS_FAIL;
}

static const char *
parse_param_string(const char *eq)
{
    return eq + 1;
}

static int
parse_param_bool(const char *eq, int *enabled)
{
    if (eq == NULL)
    {
        *enabled = 1;
        return URJ_STATUS_OK;
    }

    if (sscanf(eq, "%d", enabled) == 1 && (*enabled == 0 || *enabled == 1))
        return URJ_STATUS_OK;

    urj_error_set (URJ_ERROR_SYNTAX, "need unsigned int, not '%s'", eq + 1);
    return URJ_STATUS_FAIL;
}

int
urj_param_push (const urj_param_list_t *params, const urj_param_t ***bp,
                const char *p)
{
    int key;
    urj_param_t *new_p;
    int r = URJ_STATUS_OK;
    const char *eq;
    urj_param_type_t type;

    key = urj_param_parse_key(params, p);
    if (key == -1)
        return URJ_STATUS_FAIL;

    type = urj_param_type_of(params, key);
    if (type == -1)
        return URJ_STATUS_FAIL;

    eq = strchr(p, '=');
    if (type != URJ_PARAM_TYPE_BOOL && eq == NULL)
    {
        urj_error_set (URJ_ERROR_SYNTAX,
                       "param should be of the form 'key=value', not '%s'", p);
        return URJ_STATUS_FAIL;
    }

    new_p = urj_param_increase (bp);
    if (new_p == NULL)
        return URJ_STATUS_FAIL;

    new_p->type = type;
    new_p->key  = key;
    switch (new_p->type)
    {
    case URJ_PARAM_TYPE_LU:
        r = parse_param_lu(eq, &new_p->value.lu);
        break;
    case URJ_PARAM_TYPE_STRING:
        new_p->value.string = parse_param_string(eq);
        if (new_p->value.string == NULL)
            r = URJ_STATUS_FAIL;
        break;
    case URJ_PARAM_TYPE_BOOL:
        r = parse_param_bool(eq, &new_p->value.enabled);
        break;
    }

    if (r == URJ_STATUS_FAIL)
        urj_param_decrease (bp);

    return r;
}

const char *
urj_param_string(const urj_param_list_t *params, const urj_param_t *p)
{
#define PARAM_BUF_SIZE  256
    static char buf[PARAM_BUF_SIZE];
    size_t size;

    snprintf(buf, sizeof buf, "%s=", urj_param_key_string(params, p->key));
    size = strlen(buf);

    switch (p->type)
    {
    case URJ_PARAM_TYPE_LU:
        snprintf(buf + size, sizeof buf - size, "%lu", p->value.lu);
        break;
    case URJ_PARAM_TYPE_STRING:
        snprintf(buf + size, sizeof buf - size, "%s", p->value.string);
        break;
    case URJ_PARAM_TYPE_BOOL:
        snprintf(buf + size, sizeof buf - size, "%s",
                 p->value.enabled ? "on" : "off");
        break;
    default:
        return "urj_param_string(): <unimplemented>";
    }

    return buf;
}

int
urj_param_init (const urj_param_t ***bp)
{
    *bp = calloc (1, sizeof **bp);
    if (*bp == NULL)
    {
        urj_error_set (URJ_ERROR_OUT_OF_MEMORY, "calloc(%zd,%zd) fails",
                       (size_t) 1, sizeof **bp);
        return URJ_STATUS_FAIL;
    }

    *bp[0] = NULL;

    return URJ_STATUS_OK;
}

int
urj_param_init_list (const urj_param_t ***bp, char *params[],
                     const urj_param_list_t *param_list)
{
    int ret;
    size_t i;

    ret = urj_param_init (bp);
    if (ret != URJ_STATUS_OK)
        return ret;

    for (i = 0; params[i] != NULL; ++i)
    {
        ret = urj_param_push (param_list, bp, params[i]);
        if (ret != URJ_STATUS_OK)
        {
            urj_param_clear (bp);
            return ret;
        }
    }

    return URJ_STATUS_OK;
}

int
urj_param_clear (const urj_param_t ***bp)
{
    const urj_param_t **scan;

    for (scan = *bp; *scan != NULL; scan++)
        free ((void *) *scan);

    free (*bp);

    return URJ_STATUS_OK;
}

size_t
urj_param_num (const urj_param_t *p[])
{
    size_t n;

    if (p == NULL)
        return 0;

    for (n = 0; p[n] != NULL; n++)
    {
        /* advance n */
    }

    return n;
}
