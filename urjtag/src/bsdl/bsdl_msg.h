/*
 * $Id$
 *
 * Copyright (C) 2008, Arnim Laeuger
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
 * Written by Arnim Laeuger <arniml@users.sourceforge.net>, 2007.
 *
 */

#ifndef URJ_BSDL_MSG_H
#define URJ_BSDL_MSG_H

#include <urjtag/log.h>
#include <urjtag/error.h>

#include <urjtag/bsdl_mode.h>

#include "bsdl_types.h"

#define urj_bsdl_msg(proc_mode, ...)                            \
    do {                                                        \
        if (proc_mode & URJ_BSDL_MODE_MSG_NOTE) {               \
            urj_log (URJ_LOG_LEVEL_NORMAL, "-N- ");             \
            urj_log (URJ_LOG_LEVEL_NORMAL, __VA_ARGS__);}       \
    } while (0)

#define urj_bsdl_warn(proc_mode, ...)                      \
    do {                                                   \
        if (proc_mode & URJ_BSDL_MODE_MSG_WARN) {          \
            urj_log (URJ_LOG_LEVEL_WARNING, "-W- ");       \
            urj_log (URJ_LOG_LEVEL_WARNING, __VA_ARGS__);} \
    } while (0)

#define urj_bsdl_err(proc_mode, ...)                            \
    do {                                                        \
        if (proc_mode & URJ_BSDL_MODE_MSG_ERR) {                \
            urj_log (URJ_LOG_LEVEL_ERROR, "-E- ");              \
            urj_log (URJ_LOG_LEVEL_ERROR, __VA_ARGS__);}        \
    } while (0)

#define urj_bsdl_err_set(proc_mode, err, ...)  \
    do {                                       \
        if (proc_mode & URJ_BSDL_MODE_MSG_ERR) \
            urj_error_set (err, __VA_ARGS__);  \
    } while (0)

#define urj_bsdl_ftl_set(proc_mode, err, ...)    \
    do {                                         \
        if (proc_mode & URJ_BSDL_MODE_MSG_FATAL) \
            urj_error_set (err, __VA_ARGS__);    \
    } while (0)

#endif /* URJ_BSDL_MSG_H */
