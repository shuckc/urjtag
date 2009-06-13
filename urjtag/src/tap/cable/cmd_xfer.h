/*
 * $Id$
 *
 * Generic command buffer handler.
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
 * Written by Arnim Laeuger, 2008.
 *
 */

#ifndef URJ_TAP_CABLE_CMD_XFER_H
#define URJ_TAP_CABLE_CMD_XFER_H

#include <sysdep.h>

#include <urjtag/cable.h>

/* description of a command
   the buffer can contain one or more commands if receive count
   is zero for all of them */
typedef struct URJ_TAP_CABLE_CX_CMD urj_tap_cable_cx_cmd_t;
struct URJ_TAP_CABLE_CX_CMD
{
    urj_tap_cable_cx_cmd_t *next;
    uint32_t buf_len;
    uint32_t buf_pos;
    uint8_t *buf;
    uint32_t to_recv;
};

struct URJ_TAP_CABLE_CX_CMD_ROOT
{
    urj_tap_cable_cx_cmd_t *first;
    urj_tap_cable_cx_cmd_t *last;
};
typedef struct URJ_TAP_CABLE_CX_CMD_ROOT urj_tap_cable_cx_cmd_root_t;

int urj_tap_cable_cx_cmd_space (urj_tap_cable_cx_cmd_root_t *cmd_root,
                                int max_len);
int urj_tap_cable_cx_cmd_push (urj_tap_cable_cx_cmd_root_t *cmd_root,
                               uint8_t d);
urj_tap_cable_cx_cmd_t
    *urj_tap_cable_cx_cmd_dequeue (urj_tap_cable_cx_cmd_root_t *cmd_root);
void urj_tap_cable_cx_cmd_free (urj_tap_cable_cx_cmd_t *cmd);
urj_tap_cable_cx_cmd_t
    *urj_tap_cable_cx_cmd_queue (urj_tap_cable_cx_cmd_root_t *cmd_root,
                                 uint32_t to_recv);
void urj_tap_cable_cx_cmd_init (urj_tap_cable_cx_cmd_root_t *cmd_root);
void urj_tap_cable_cx_cmd_deinit (urj_tap_cable_cx_cmd_root_t *cmd_root);

void urj_tap_cable_cx_xfer (urj_tap_cable_cx_cmd_root_t *cmd_root,
                            const urj_tap_cable_cx_cmd_t *out_cmd,
                            urj_cable_t *cable,
                            urj_cable_flush_amount_t how_much);
uint8_t urj_tap_cable_cx_xfer_recv (urj_cable_t *cable);

#endif /* URJ_TAP_CABLE_CMD_XFER_H */
