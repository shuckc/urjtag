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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#ifndef HAVE_GETDELIM

#define GETDELIM_BUFFER 128

ssize_t
getdelim (char **lineptr, size_t *n, int delimiter, FILE *stream)
{
    char *p;
    int c;
    size_t len = 0;

    if (!lineptr || !n)
        return -1;

    /* allocate initial buffer */
    if (!*lineptr || !*n)
    {
        char *np;
        np = realloc (*lineptr, GETDELIM_BUFFER);
        if (!np)
            return -1;
        *n = GETDELIM_BUFFER;
        *lineptr = np;
    }

    p = *lineptr;

    /* read characters from stream */
    while ((c = fgetc (stream)) != EOF)
    {
        if (len >= *n)
        {
            char *np = realloc (*lineptr, *n * 2);
            if (!np)
                return -1;
            p = np + (p - *lineptr);
            *lineptr = np;
            *n *= 2;
        }
        *p++ = (char) c;
        len++;
        if (delimiter == c)
            break;
    }

    /* end of file without any bytes read */
    if ((c == EOF) && (len == 0))
        return -1;

    /* trailing '\0' */
    if (len >= *n)
    {
        char *np = realloc (*lineptr, *n + 1);
        if (!np)
            return -1;
        p = np + (p - *lineptr);
        *lineptr = np;
        *n += 1;
    }
    *p = '\0';

    return len;
}

#endif /* HAVE_GETDELIM */
