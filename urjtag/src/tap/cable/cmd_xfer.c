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

#include <sysdep.h>

#include <stdlib.h>
#include <string.h>

#include <urjtag/error.h>

#include "generic.h"
#include "generic_usbconn.h"

#include "cmd_xfer.h"


/*****************************************************************************
 * extend_cmd_buffer( cmd )
 *
 * Extends the buffer of the given command if a new byte wouldn't fit into
 * the current buffer size.
 *
 * cmd : pointer to urj_tap_cable_cx_cmd_t
 *
 * Return value:
 * 0 : Error occured, not enough memory
 * 1 : All ok
 *
 ****************************************************************************/
static int
extend_cmd_buffer (urj_tap_cable_cx_cmd_t *cmd)
{
    /* check size of cmd buffer and increase it if not sufficient */
    if (cmd->buf_pos >= cmd->buf_len)
    {
        cmd->buf_len *= 2;
        if (cmd->buf)
            cmd->buf = realloc (cmd->buf, cmd->buf_len);
    }

    if (cmd->buf == NULL)
    {
        urj_error_set (URJ_ERROR_OUT_OF_MEMORY, "realloc(%s,%zd) fails",
                       "cmd->buf", (size_t) cmd->buf_len);
    }

    return cmd->buf ? 1 : 0;
}


/*****************************************************************************
 * urj_tap_cable_cx_cmd_space( cmd, max_len )
 *
 * Return the difference between actually allocated bytes in the buffer of
 * the current last command and max_len.  If there are already more bytes
 * allocated than max_len, this function will return zero.
 *
 * cmd      : pointer to urj_tap_cable_cx_cmd_t struct
 * max_len  : upper limit for the space to allocate
 *
 * Return value:
 * 0 : No space left
 * >0: Number of bytes left
 *
 ****************************************************************************/
int
urj_tap_cable_cx_cmd_space (urj_tap_cable_cx_cmd_root_t *cmd_root,
                            int max_len)
{
    int n;
    urj_tap_cable_cx_cmd_t *cmd = cmd_root->last;

    if (!cmd)
        return max_len;

    n = max_len - cmd->buf_pos;
    if (n < 0)
        return 0;

    return n;
}


/*****************************************************************************
 * urj_tap_cable_cx_cmd_push( cmd, d )
 *
 * Pushes the byte value d to the buffer of the current last command.
 *
 * cmd_root : pointer to urj_tap_cable_cx_cmd_root_t struct
 * d        : new value to be pushed
 *
 * Return value:
 * 0 : Error occured
 * 1 : All ok
 *
 ****************************************************************************/
int
urj_tap_cable_cx_cmd_push (urj_tap_cable_cx_cmd_root_t *cmd_root, uint8_t d)
{
    urj_tap_cable_cx_cmd_t *cmd = cmd_root->last;

    if (!cmd)
        return 0;

    if (!extend_cmd_buffer (cmd))
        return 0;

    cmd->buf[cmd->buf_pos++] = d;

    return 1;
}


/*****************************************************************************
 * urj_tap_cable_cx_cmd_dequeue( cmd_root )
 *
 * Dequeues the first command.
 *
 * cmd_root : pointer to urj_tap_cable_cx_cmd_root_t parameter struct
 *
 * Return value:
 * NULL   : Error occured
 * <>NULL : All ok, pointer to dequeued urj_tap_cable_cx_cmd_t
 *
 ****************************************************************************/
urj_tap_cable_cx_cmd_t *
urj_tap_cable_cx_cmd_dequeue (urj_tap_cable_cx_cmd_root_t *cmd_root)
{
    urj_tap_cable_cx_cmd_t *cmd = cmd_root->first;

    if (cmd)
    {
        if ((cmd_root->first = cmd->next) == NULL)
            cmd_root->last = NULL;
        cmd->next = NULL;
    }

    return cmd;
}


/*****************************************************************************
 * urj_tap_cable_cx_cmd_free( cmd )
 *
 * Frees allocated memory of specified cmd structure.
 *
 * cmd : pointer to urj_tap_cable_cx_cmd_t
 *
 * Return value:
 * none
 *
 ****************************************************************************/
void
urj_tap_cable_cx_cmd_free (urj_tap_cable_cx_cmd_t *cmd)
{
    if (cmd)
    {
        if (cmd->buf)
            free (cmd->buf);
        free (cmd);
    }
}


/*****************************************************************************
 * urj_tap_cable_cx_cmd_queue( cmd_root, to_recv )
 *
 * Allocates a new urj_tap_cable_cx_cmd_t and queues it at the end of the command
 * queue. The value of to_recv will be stored in the new cmd element,
 * set to 0 if this command will not generate receive bytes.
 *
 * cmd_root : pointer to urj_tap_cable_cx_cmd_root_t parameter struct
 * to_recv  : number of receive bytes that this command will generate
 *
 * Return value:
 * NULL   : Error occured
 * <>NULL : All ok, pointer to allocated urj_tap_cable_cx_cmd_t
 *
 ****************************************************************************/
urj_tap_cable_cx_cmd_t *
urj_tap_cable_cx_cmd_queue (urj_tap_cable_cx_cmd_root_t *cmd_root,
                            uint32_t to_recv)
{
    urj_tap_cable_cx_cmd_t *cmd = malloc (sizeof (urj_tap_cable_cx_cmd_t));

    if (cmd)
    {
        cmd->buf_len = 64;
        if ((cmd->buf = malloc (cmd->buf_len)) == NULL)
        {
            free (cmd);
            cmd = NULL;
        }
        else
        {
            cmd->buf_pos = 0;
            cmd->to_recv = to_recv;
            cmd->next = NULL;
            if (!cmd_root->first)
                cmd_root->first = cmd;
            if (cmd_root->last)
                cmd_root->last->next = cmd;
            cmd_root->last = cmd;
        }
    }

    if (cmd == NULL)
    {
        urj_error_set (URJ_ERROR_OUT_OF_MEMORY, "malloc(%zd)/malloc(%zd) fails",
                       sizeof (urj_tap_cable_cx_cmd_t), (size_t) 64);
    }

    return cmd;
}


/*****************************************************************************
 * urj_tap_cable_cx_cmd_init( cmd_root )
 *
 * Initializes the command root structure.
 *
 * cmd_root : pointer to urj_tap_cable_cx_cmd_root_t
 *
 * Return value:
 * none
 *
 ****************************************************************************/
void
urj_tap_cable_cx_cmd_init (urj_tap_cable_cx_cmd_root_t *cmd_root)
{
    cmd_root->first = NULL;
    cmd_root->last = NULL;
}


/*****************************************************************************
 * urj_tap_cable_cx_cmd_deinit( cmd_root )
 *
 * Deinitialzes and frees all elements from the command root structure.
 *
 * cmd_root : pointer to urj_tap_cable_cx_cmd_root_t
 *
 * Return value:
 * none
 *
 ****************************************************************************/
void
urj_tap_cable_cx_cmd_deinit (urj_tap_cable_cx_cmd_root_t *cmd_root)
{
    urj_tap_cable_cx_cmd_t *cmd;
    while (cmd_root->first)
    {
        cmd = urj_tap_cable_cx_cmd_dequeue (cmd_root);
        urj_tap_cable_cx_cmd_free (cmd);
    }
}


/*****************************************************************************
 * urj_tap_cable_cx_xfer( cmd_root, out_cmd, cable, how_much )
 *
 * Unrolls the queued commands and posts their payload to the usbconn driver.
 * NB: urj_tap_usbconn_write will buffer the accumulated payload until urj_tap_usbconn_read
 *     is called.
 *
 * Flushing of the posted payload bytes is triggered when how_much
 * requests to do so or if receive bytes are expected.
 *
 * cmd_root : pointer to urj_tap_cable_cx_cmd_root_t struct
 * out_cmd  : pointer to urj_tap_cable_cx_cmd_t for an optional command that is appended
 *            to send buffer in case commands have been scheduled that
 *            yield return/receive data from the device
 * cable    : current urj_cable_t
 * how_much : urj_cable_flush_amount_t value specifying the flush strategy
 *
 * Return value:
 * none
 *
 ****************************************************************************/
void
urj_tap_cable_cx_xfer (urj_tap_cable_cx_cmd_root_t *cmd_root,
                       const urj_tap_cable_cx_cmd_t *out_cmd,
                       urj_cable_t *cable, urj_cable_flush_amount_t how_much)
{
    urj_tap_cable_cx_cmd_t *cmd = urj_tap_cable_cx_cmd_dequeue (cmd_root);
    uint32_t bytes_to_recv;

    bytes_to_recv = 0;

    while (cmd)
    {
        /* Step 1: copy command bytes buffered for sending them later
           through the usbconn driver */
        bytes_to_recv += cmd->to_recv;
        /* write command data (buffered) */
        urj_tap_usbconn_write (cable->link.usb, cmd->buf, cmd->buf_pos,
                               cmd->to_recv);
        urj_tap_cable_cx_cmd_free (cmd);
        cmd = urj_tap_cable_cx_cmd_dequeue (cmd_root);
    }

    /* it's possible for the caller to define an extra command that is
       appended right before sending commands to the device in case output
       data is expected */
    if (bytes_to_recv && out_cmd)
    {
        urj_tap_usbconn_write (cable->link.usb, out_cmd->buf,
                               out_cmd->buf_pos, out_cmd->to_recv);
        bytes_to_recv += out_cmd->to_recv;
    }

    if (bytes_to_recv || (how_much != URJ_TAP_CABLE_TO_OUTPUT))
    {
        /* Step 2: flush scheduled bytes */
        urj_tap_usbconn_read (cable->link.usb, NULL, 0);

        bytes_to_recv = 0;
    }
}


/*****************************************************************************
 * urj_tap_cable_cx_xfer_recv( cable )
 *
 * Extracts the byte at the current position from the receive buffer.
 *
 * cable : pointer to the current cable struct
 *
 * Return value:
 * Byte value from receive buffer
 *
 ****************************************************************************/
uint8_t
urj_tap_cable_cx_xfer_recv (urj_cable_t *cable)
{
    uint8_t buf;

    if (urj_tap_usbconn_read (cable->link.usb, &buf, 1) == 1)
    {
        return buf;
    }
    else
        return 0;
}


/*
 Local Variables:
 mode:C
 c-default-style:gnu
 indent-tabs-mode:nil
 End:
*/
