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

#include <stdarg.h>

#include <urjtag/log.h>
#include <urjtag/error.h>
#include <urjtag/jtag.h>

urj_error_state_t urj_error_state;
int urj_debug_mode = 0;
int urj_big_endian = 0;

static int stderr_vprintf (const char *fmt, va_list ap);

urj_log_state_t urj_log_state =
    {
        .level = URJ_LOG_LEVEL_NORMAL,
        .out_vprintf = vprintf,
        .err_vprintf = stderr_vprintf,
    };

static int
stderr_vprintf(const char *fmt, va_list ap)
{
    return vfprintf (stderr, fmt, ap);
}

int
urj_log (urj_log_level_t level, const char *fmt, ...)
{
    va_list ap;
    int r;

    if (level < urj_log_state.level)
        return 0;

    va_start (ap, fmt);
    if (level < URJ_LOG_LEVEL_WARNINGS)
        r = urj_log_state.out_vprintf (fmt, ap);
    else
        r = urj_log_state.err_vprintf (fmt, ap);
    va_end (ap);

    return r;
}

void
urj_error_reset (void)
{
    urj_error_state.errnum = URJ_ERROR_OK;
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

const char *
urj_error_string (urj_error_t err)
{
    switch (err)
    {
    case URJ_ERROR_OK:                  return "no error";
    case URJ_ERROR_ALREADY:             return "already defined";
    case URJ_ERROR_OUT_OF_MEMORY:       return "out of memory";
    case URJ_ERROR_NO_ACTIVE_PART:      return "no active part";
    case URJ_ERROR_INVALID:             return "invalid parameter";
    case URJ_ERROR_NOTFOUND:            return "not found";
    case URJ_ERROR_IO:                  return "I/O error from OS";
    case URJ_ERROR_NO_BUS_DRIVER:       return "no bus driver";
    case URJ_ERROR_BUFFER_EXHAUSTED:    return "buffer exhausted";
    }

    return "UNDEFINED ERROR";
}
