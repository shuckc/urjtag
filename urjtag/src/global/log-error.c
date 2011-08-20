/*
 * $Id$
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

#include <sysdep.h>

#include <stdarg.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>

#include <urjtag/log.h>
#include <urjtag/error.h>
#include <urjtag/jtag.h>

urj_error_state_t urj_error_state;

static int stderr_vprintf (const char *fmt, va_list ap);
static int stdout_vprintf (const char *fmt, va_list ap);

urj_log_state_t urj_log_state =
    {
        .level = URJ_LOG_LEVEL_NORMAL,
        .out_vprintf = stdout_vprintf,
        .err_vprintf = stderr_vprintf,
    };

static int
stderr_vprintf(const char *fmt, va_list ap)
{
    return vfprintf (stderr, fmt, ap);
}

static int
stdout_vprintf(const char *fmt, va_list ap)
{
    int r = vfprintf (stdout, fmt, ap);

    fflush (stdout);

    return r;
}

static int
log_printf (int (*p) (const char *, va_list), const char *fmt, ...)
{
    va_list ap;
    int r;

    va_start (ap, fmt);
    r = (*p) (fmt, ap);
    va_end (ap);

    return r;
}

int
urj_do_log (urj_log_level_t level, const char *file, size_t line,
            const char *func, const char *fmt, ...)
{
    int (*p) (const char *fmt, va_list ap);
    va_list ap;
    int r = 0;

    if (level < urj_log_state.level)
        return 0;

    if (level < URJ_LOG_LEVEL_WARNING)
        p = urj_log_state.out_vprintf;
    else
        p = urj_log_state.err_vprintf;

    if (level == URJ_LOG_LEVEL_WARNING || level == URJ_LOG_LEVEL_ERROR
        || level <= URJ_LOG_LEVEL_DETAIL)
        r += log_printf (p, "%s: ", urj_log_level_string (level));

    if (urj_log_state.level <= URJ_LOG_LEVEL_DEBUG)
        r += log_printf (p, "%s:%i %s(): ", file, line, func);

    va_start (ap, fmt);
    r += (*p) (fmt, ap);
    va_end (ap);

    return r;
}

urj_error_t
urj_error_get (void)
{
    return urj_error_state.errnum;
}

void
urj_error_reset (void)
{
    urj_error_state.errnum = URJ_ERROR_OK;
}

static const struct {
    const urj_log_level_t level;
    const char *name;
} levels[] = {
#define L(LVL, lvl) { URJ_LOG_LEVEL_##LVL, #lvl, }
    L(ALL, all),
    L(COMM, comm),
    L(DEBUG, debug),
    L(DETAIL, detail),
    L(NORMAL, normal),
    L(WARNING, warning),
    L(ERROR, error),
    L(SILENT, silent),
#undef L
};

const char *
urj_log_level_string (urj_log_level_t level)
{
    size_t i;

    for (i = 0; i < ARRAY_SIZE (levels); ++i)
        if (levels[i].level == level)
            return levels[i].name;

    return "unknown";
}

urj_log_level_t
urj_string_log_level (const char *slevel)
{
    size_t i;

    for (i = 0; i < ARRAY_SIZE (levels); ++i)
        if (!strcmp (levels[i].name, slevel))
            return levels[i].level;

    return -1;
}

const char *
urj_error_string (urj_error_t err)
{
    switch (err)
    {
    case URJ_ERROR_OK:                  return "no error";
    case URJ_ERROR_ALREADY:             return "already defined";
    case URJ_ERROR_OUT_OF_MEMORY:       return "out of memory";
    case URJ_ERROR_NO_CHAIN:            return "no chain";
    case URJ_ERROR_NO_PART:             return "no part";
    case URJ_ERROR_NO_ACTIVE_INSTRUCTION: return "no active instruction";
    case URJ_ERROR_NO_DATA_REGISTER:    return "no data register";
    case URJ_ERROR_INVALID:             return "invalid parameter";
    case URJ_ERROR_NOTFOUND:            return "not found";
    case URJ_ERROR_NO_BUS_DRIVER:       return "no bus driver";
    case URJ_ERROR_BUFFER_EXHAUSTED:    return "buffer exhausted";
    case URJ_ERROR_ILLEGAL_STATE:       return "illegal state";
    case URJ_ERROR_ILLEGAL_TRANSITION:  return "illegal state transition";
    case URJ_ERROR_OUT_OF_BOUNDS:       return "out of bounds";
    case URJ_ERROR_TIMEOUT:             return "timeout";
    case URJ_ERROR_UNSUPPORTED:         return "unsupported";
    case URJ_ERROR_SYNTAX:              return "syntax";
    case URJ_ERROR_FILEIO:              return "file I/O";

    case URJ_ERROR_IO:                  return "I/O error from OS";
    case URJ_ERROR_FTD:                 return "ftdi/ftd2xx error";
    case URJ_ERROR_USB:                 return "libusb error";

    case URJ_ERROR_BUS:                 return "bus";
    case URJ_ERROR_BUS_DMA:             return "bus DMA";

    case URJ_ERROR_FLASH:               return "flash";
    case URJ_ERROR_FLASH_DETECT:        return "flash detect";
    case URJ_ERROR_FLASH_PROGRAM:       return "flash program";
    case URJ_ERROR_FLASH_ERASE:         return "flash erase";
    case URJ_ERROR_FLASH_LOCK:          return "flash lock";
    case URJ_ERROR_FLASH_UNLOCK:        return "flash unlock";

    case URJ_ERROR_BSDL_VHDL:           return "vhdl subsystem";
    case URJ_ERROR_BSDL_BSDL:           return "bsdl subsystem";

    case URJ_ERROR_BFIN:                return "blackfin";

    case URJ_ERROR_PLD:                 return "pld subsystem";

    case URJ_ERROR_UNIMPLEMENTED:       return "unimplemented";

    case URJ_ERROR_FIRMWARE:            return "firmware";
    }

    return "UNDEFINED ERROR";
}

const char *
urj_error_describe (void)
{
    static char msg[URJ_ERROR_MSG_LEN + 1024 + 256 + 20];

    if (urj_error_state.errnum == URJ_ERROR_IO)
    {
        snprintf (msg, sizeof msg, "%s: %s %s",
                  "system error", strerror(urj_error_state.sys_errno),
                  urj_error_state.msg);
    }
    else
    {
        snprintf (msg, sizeof msg, "%s: %s",
                  urj_error_string (urj_error_state.errnum),
                  urj_error_state.msg);
    }

    return msg;
}

void
urj_log_error_describe (urj_log_level_t level)
{
    if (urj_error_get () == URJ_ERROR_OK)
        return;

    urj_do_log (level,
                urj_error_state.file, urj_error_state.line,
                urj_error_state.function,
                "%s\n", urj_error_describe ());

    urj_error_reset ();
}
