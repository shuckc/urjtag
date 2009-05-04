/*
 * $Id: urjtag.c 1539 2009-05-01 12:02:08Z rfhh $
 *
 * Copyright (C) 2009 Rutger Hofman, VU Amsterdam
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
 * Written by Rutger Hofman
 */

#include <urjtag/sysdep.h>

#include <urjtag/error.h>
#include <urjtag/jtag.h>

urj_error_state_t urj_error_state;
int urj_debug_mode = 0;
int urj_big_endian = 0;

const urj_error_state_t *
urj_error_get (void)
{
    return &urj_error_state;
}

urj_error_t
urj_error_get_reset (void)
{
    urj_error_t e = urj_error_state.errnum;

    urj_error_state.errnum = URJ_ERROR_OK;

    return e;
}

const char *
urj_error_describe (void)
{
    static char msg[URJ_ERROR_MSG_LEN + 1024 + 256 + 20];

    snprintf (msg, sizeof msg, "%s:%d %s(): %s", urj_error_state.file,
              urj_error_state.line, urj_error_state.function,
              urj_error_state.msg);

    return msg;
}


