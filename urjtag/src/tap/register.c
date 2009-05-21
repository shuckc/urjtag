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

#include <stdlib.h>
#include <string.h>

#include <urjtag/error.h>
#include <urjtag/log.h>
#include <urjtag/tap_register.h>

urj_tap_register_t *
urj_tap_register_alloc (int len)
{
    urj_tap_register_t *tr;

    if (len < 1)
    {
        urj_error_set (URJ_ERROR_INVALID, "len < 1");
        return NULL;
    }

    tr = malloc (sizeof (urj_tap_register_t));
    if (!tr)
    {
        urj_error_set (URJ_ERROR_OUT_OF_MEMORY, "malloc(%zd) fails",
                       sizeof (urj_tap_register_t));
        return NULL;
    }

    tr->data = malloc (len);
    if (!tr->data)
    {
        free (tr);
        urj_error_set (URJ_ERROR_OUT_OF_MEMORY, "malloc(%zd) fails",
                       (size_t) len);
        return NULL;
    }

    memset (tr->data, 0, len);

    tr->string = malloc (len + 1);
    if (!tr->string)
    {
        free (tr->data);
        free (tr);
        urj_error_set (URJ_ERROR_OUT_OF_MEMORY, "malloc(%zd) fails",
                       (size_t) (len + 1));
        return NULL;
    }

    tr->len = len;
    tr->string[len] = '\0';

    return tr;
}

urj_tap_register_t *
urj_tap_register_duplicate (const urj_tap_register_t *tr)
{
    if (!tr)
    {
        urj_error_set (URJ_ERROR_INVALID, "tr == NULL");
        return NULL;
    }

    return urj_tap_register_init (urj_tap_register_alloc (tr->len),
                                  urj_tap_register_get_string (tr));
}

void
urj_tap_register_free (urj_tap_register_t *tr)
{
    if (tr)
    {
        free (tr->data);
        free (tr->string);
    }
    free (tr);
}

urj_tap_register_t *
urj_tap_register_fill (urj_tap_register_t *tr, int val)
{
    if (tr)
        memset (tr->data, val & 1, tr->len);

    return tr;
}

const char *
urj_tap_register_get_string (const urj_tap_register_t *tr)
{
    int i;

    if (!tr)
        return NULL;

    for (i = 0; i < tr->len; i++)
        tr->string[tr->len - 1 - i] = (tr->data[i] & 1) ? '1' : '0';

    return tr->string;
}

uint64_t
urj_tap_register_get_value (const urj_tap_register_t *tr)
{
    int i;
    uint64_t l, b;

    if (!tr)
        return 0;

    l = 0;
    b = 1;
    for (i = 0; i < tr->len; ++i)
    {
        if (tr->data[i] & 1)
            l |= b;
        b <<= 1;
    }

    return l;
}

int
urj_tap_register_all_bits_same_value (const urj_tap_register_t *tr)
{
    int i, value;
    if (!tr)
        return -1;
    if (tr->len < 0)
        return -1;

    /* Return -1 if any of the bits in the register
     * differs from the others; the value otherwise. */

    value = tr->data[0] & 1;

    for (i = 1; i < tr->len; i++)
    {
        if ((tr->data[i] & 1) != value)
            return -1;
    }
    return value;
}

urj_tap_register_t *
urj_tap_register_init (urj_tap_register_t *tr, const char *value)
{
    int i;

    const char *p;

    if (!value || !tr)
        return tr;

    p = strchr (value, '\0');

    for (i = 0; i < tr->len; i++)
    {
        if (p == value)
            tr->data[i] = 0;
        else
        {
            p--;
            tr->data[i] = (*p == '0') ? 0 : 1;
        }
    }

    return tr;
}

int
urj_tap_register_compare (const urj_tap_register_t *tr,
                          const urj_tap_register_t *tr2)
{
    int i;

    if (!tr && !tr2)
        return 0;

    if (!tr || !tr2)
        return 1;

    if (tr->len != tr2->len)
        return 1;

    for (i = 0; i < tr->len; i++)
        if (tr->data[i] != tr2->data[i])
            return 1;

    return 0;
}

int
urj_tap_register_match (const urj_tap_register_t *tr, const char *expr)
{
    int i;
    const char *s;

    if (!tr || !expr || (tr->len != strlen (expr)))
        return 0;

    s = urj_tap_register_get_string (tr);

    for (i = 0; i < tr->len; i++)
        if ((expr[i] != '?') && (expr[i] != s[i]))
            return 0;

    return 1;
}

urj_tap_register_t *
urj_tap_register_inc (urj_tap_register_t *tr)
{
    int i;

    if (!tr)
        return NULL;

    for (i = 0; i < tr->len; i++)
    {
        tr->data[i] ^= 1;

        if (tr->data[i] == 1)
            break;
    }

    return tr;
}

urj_tap_register_t *
urj_tap_register_dec (urj_tap_register_t *tr)
{
    int i;

    if (!tr)
        return NULL;

    for (i = 0; i < tr->len; i++)
    {
        tr->data[i] ^= 1;

        if (tr->data[i] == 0)
            break;
    }

    return tr;
}

urj_tap_register_t *
urj_tap_register_shift_right (urj_tap_register_t *tr, int shift)
{
    int i;

    if (!tr)
        return NULL;

    if (shift < 1)
        return tr;

    for (i = 0; i < tr->len; i++)
    {
        if (i + shift < tr->len)
            tr->data[i] = tr->data[i + shift];
        else
            tr->data[i] = 0;
    }

    return tr;
}

urj_tap_register_t *
urj_tap_register_shift_left (urj_tap_register_t *tr, int shift)
{
    int i;

    if (!tr)
        return NULL;

    if (shift < 1)
        return tr;

    for (i = tr->len - 1; i >= 0; i--)
    {
        if (i - shift >= 0)
            tr->data[i] = tr->data[i - shift];
        else
            tr->data[i] = 0;
    }

    return tr;
}
