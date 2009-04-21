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

#ifndef REGISTER_H
#define	REGISTER_H

#define	tap_register_t	tap_register
typedef struct tap_register
{
    char *data;                 /* (public, r/w) register data */
    int len;                    /* (public, r/o) register length */
    char *string;               /* (private) string representation of register data */
} tap_register_t;

tap_register *register_alloc (int len);
tap_register *register_duplicate (const tap_register_t * tr);
void register_free (tap_register_t * tr);
tap_register *register_fill (tap_register_t * tr, int val);
const char *register_get_string (const tap_register_t * tr);
int register_all_bits_same_value (const tap_register_t * tr);
tap_register *register_init (tap_register_t * tr, const char *value);
int register_compare (const tap_register_t * tr, const tap_register_t * tr2);
int register_match (const tap_register_t * tr, const char *expr);
tap_register *register_inc (tap_register_t * tr);
tap_register *register_dec (tap_register_t * tr);
tap_register *register_shift_right (tap_register_t * tr, int shift);
tap_register *register_shift_left (tap_register_t * tr, int shift);

#endif /* REGISTER_H */
