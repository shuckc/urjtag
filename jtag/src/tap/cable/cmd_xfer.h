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

#ifndef CMD_XFER_H
#define CMD_XFER_H

#include "sysdep.h"

#include <cable.h>

/* description of a command
   the buffer can contain one or more commands if receive count
   is zero for all of them */
typedef struct cx_cmd cx_cmd_t;
struct cx_cmd {
  cx_cmd_t *next;
  uint32_t  buf_len;
  uint32_t  buf_pos;
  uint8_t  *buf;
  uint32_t  to_recv;
};

struct cx_cmd_root {
  cx_cmd_t *first;
  cx_cmd_t *last;
};
typedef struct cx_cmd_root cx_cmd_root_t;

int cx_cmd_push( cx_cmd_root_t *cmd_root, uint8_t d);
cx_cmd_t *cx_cmd_dequeue( cx_cmd_root_t *cmd_root );
void cx_cmd_free( cx_cmd_t *cmd );
cx_cmd_t *cx_cmd_queue( cx_cmd_root_t *cmd_root, uint32_t to_recv );
void cx_cmd_init( cx_cmd_root_t *cmd_root );
void cx_cmd_deinit( cx_cmd_root_t *cmd_root );

void cx_xfer( cx_cmd_root_t *cmd_root, const cx_cmd_t *out_cmd,
              cable_t *cable, cable_flush_amount_t how_much );
uint8_t cx_xfer_recv( cable_t *cable );

#endif /* CMD_XFER_H */
