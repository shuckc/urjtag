/*
 * $Id$
 *
 * Altera USB-Blaster<tm> Cable Driver
 * Copyright (C) 2006 K. Waschk
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
 * Written by Kolja Waschk, 2006; http://www.ixo.de
 *
 */

/*
 * Note: this driver doesn't seem to utilize FTDI code directly because
 *       it goes through the usbconn layers to access the driver.  the
 *       required status bytes for te FT245 device are automatically added
 *       to the stream so this driver can focus on the other parts.
 */

#include <sysdep.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <urjtag/cable.h>
#include <urjtag/chain.h>
#include <urjtag/cmd.h>

#include "generic.h"
#include "generic_usbconn.h"

#include "usbconn/libftdx.h"

#include "cmd_xfer.h"


#define TCK    0
#define TMS    1
#define TDI    4
#define READ   6
#define SHMODE 7
#define OTHERS ((1<<2)|(1<<3)|(1<<5))

#define TDO    0

#define FIXED_FREQUENCY 12000000L

typedef struct
{
    urj_tap_cable_cx_cmd_root_t cmd_root;
} params_t;

static int
usbblaster_connect (urj_cable_t *cable, const urj_param_t *params[])
{
    params_t *cable_params;

    /* perform urj_tap_cable_generic_usbconn_connect */
    if (urj_tap_cable_generic_usbconn_connect (cable, params) != URJ_STATUS_OK)
        return URJ_STATUS_FAIL;

    cable_params = malloc (sizeof (*cable_params));
    if (!cable_params)
    {
        urj_error_set (URJ_ERROR_OUT_OF_MEMORY, _("malloc(%zd) fails"),
                       sizeof (*cable_params));
        /* NOTE:
         * Call the underlying usbport driver (*free) routine directly
         * not urj_tap_cable_generic_usbconn_free() since it also free's cable->params
         * (which is not established) and cable (which the caller will do)
         */
        cable->link.usb->driver->free (cable->link.usb);
        return URJ_STATUS_FAIL;
    }

    urj_tap_cable_cx_cmd_init (&cable_params->cmd_root);

    /* exchange generic cable parameters with our private parameter set */
    free (cable->params);
    cable->params = cable_params;

    return URJ_STATUS_OK;
}

static void
usbblaster_set_frequency (urj_cable_t *cable, uint32_t new_frequency)
{
    if (new_frequency != FIXED_FREQUENCY)
        urj_warning (_("USB-Blaster frequency is fixed to %ld Hz\n"),
                    FIXED_FREQUENCY);

    cable->frequency = FIXED_FREQUENCY;
}

static int
usbblaster_init (urj_cable_t *cable)
{
    int i;
    params_t *params = cable->params;
    urj_tap_cable_cx_cmd_root_t *cmd_root = &params->cmd_root;

    if (urj_tap_usbconn_open (cable->link.usb) != URJ_STATUS_OK)
        return URJ_STATUS_FAIL;

    urj_tap_cable_cx_cmd_queue (cmd_root, 0);
    for (i = 0; i < 64; i++)
        urj_tap_cable_cx_cmd_push (cmd_root, 0);

    urj_tap_cable_cx_xfer (cmd_root, NULL, cable, URJ_TAP_CABLE_COMPLETELY);

    usbblaster_set_frequency (cable, FIXED_FREQUENCY);

    return URJ_STATUS_OK;
}

static void
usbblaster_cable_free (urj_cable_t *cable)
{
    params_t *params = cable->params;

    urj_tap_cable_cx_cmd_deinit (&params->cmd_root);

    urj_tap_cable_generic_usbconn_free (cable);
}

static void
usbblaster_clock_schedule (urj_cable_t *cable, int tms, int tdi, int n)
{
    params_t *params = cable->params;
    urj_tap_cable_cx_cmd_root_t *cmd_root = &params->cmd_root;
    int i, m;

    tms = tms ? (1 << TMS) : 0;
    tdi = tdi ? (1 << TDI) : 0;

    // urj_log (URJ_LOG_LEVEL_COMM, "clock: %d %d %d\n", tms, tdi, n);

    m = n;
    if (tms == 0 && m >= 8)
    {
        unsigned char tdis = tdi ? 0xFF : 0;

        urj_tap_cable_cx_cmd_queue (cmd_root, 0);
        while (m >= 8)
        {
            int i;
            int chunkbytes = (m >> 3);
            if (chunkbytes > 63)
                chunkbytes = 63;

            if (urj_tap_cable_cx_cmd_space (cmd_root,
                                            URJ_USBCONN_FTDX_MAXSEND)
                < chunkbytes + 1)
            {
                /* no space left for next clocking command
                   transfer queued commands to device and read receive data
                   to internal buffer */
                urj_tap_cable_cx_xfer (cmd_root, NULL, cable,
                                       URJ_TAP_CABLE_COMPLETELY);
                urj_tap_cable_cx_cmd_queue (cmd_root, 0);
            }


            urj_tap_cable_cx_cmd_push (cmd_root,
                                       (1 << SHMODE) | (0 << READ) |
                                       chunkbytes);

            for (i = 0; i < chunkbytes; i++)
            {
                urj_tap_cable_cx_cmd_push (cmd_root, tdis);
            }

            m -= (chunkbytes << 3);
        }
    }

    for (i = 0; i < m; i++)
    {
        urj_tap_cable_cx_cmd_queue (cmd_root, 0);
        urj_tap_cable_cx_cmd_push (cmd_root, OTHERS | (0 << TCK) | tms | tdi);
        urj_tap_cable_cx_cmd_push (cmd_root, OTHERS | (1 << TCK) | tms | tdi);
    }
}

static void
usbblaster_clock (urj_cable_t *cable, int tms, int tdi, int n)
{
    params_t *params = cable->params;

    usbblaster_clock_schedule (cable, tms, tdi, n);
    urj_tap_cable_cx_xfer (&params->cmd_root, NULL, cable,
                           URJ_TAP_CABLE_COMPLETELY);
}

static void
usbblaster_get_tdo_schedule (urj_cable_t *cable)
{
    params_t *params = cable->params;
    urj_tap_cable_cx_cmd_root_t *cmd_root = &params->cmd_root;

    urj_tap_cable_cx_cmd_queue (cmd_root, 1);
    urj_tap_cable_cx_cmd_push (cmd_root, OTHERS);       /* TCK low */
    urj_tap_cable_cx_cmd_push (cmd_root, OTHERS | (1 << READ)); /* TCK low */
}

static int
usbblaster_get_tdo_finish (urj_cable_t *cable)
{
#if 0
    char x = (urj_tap_cable_cx_xfer_recv (cable) & (1 << TDO)) ? 1 : 0;
    urj_log (URJ_LOG_LEVEL_COMM, "GetTDO %d\n", x);
    return x;
#else
    return (urj_tap_cable_cx_xfer_recv (cable) & (1 << TDO)) ? 1 : 0;
#endif
}

static int
usbblaster_get_tdo (urj_cable_t *cable)
{
    params_t *params = cable->params;

    usbblaster_get_tdo_schedule (cable);
    urj_tap_cable_cx_xfer (&params->cmd_root, NULL, cable,
                           URJ_TAP_CABLE_COMPLETELY);
    return usbblaster_get_tdo_finish (cable);
}

static int
usbblaster_set_signal (urj_cable_t *cable, int mask, int val)
{
    return 1;
}

static void
usbblaster_transfer_schedule (urj_cable_t *cable, int len, const char *in,
                              char *out)
{
    params_t *params = cable->params;
    urj_tap_cable_cx_cmd_root_t *cmd_root = &params->cmd_root;
    int in_offset = 0;

    urj_tap_cable_cx_cmd_queue (cmd_root, 0);
    urj_tap_cable_cx_cmd_push (cmd_root, OTHERS);       /* TCK low */

#if 0
    {
        int o;
        urj_log (URJ_LOG_LEVEL_COMM, "%d in: ", len);
        for (o = 0; o < len; o++)
            urj_log (URJ_LOG_LEVEL_COMM, "%c", in[o] ? '1' : '0');
        urj_log (URJ_LOG_LEVEL_COMM, "\n");
    }
#endif

    while (len - in_offset >= 8)
    {
        int i;
        int chunkbytes = ((len - in_offset) >> 3);
        if (chunkbytes > 63)
            chunkbytes = 63;

        if (out)
        {
            urj_tap_cable_cx_cmd_queue (cmd_root, chunkbytes);
            urj_tap_cable_cx_cmd_push (cmd_root,
                                       (1 << SHMODE) | (1 << READ) |
                                       chunkbytes);
        }
        else
        {
            urj_tap_cable_cx_cmd_queue (cmd_root, 0);
            urj_tap_cable_cx_cmd_push (cmd_root,
                                       (1 << SHMODE) | (0 << READ) |
                                       chunkbytes);
        }

        for (i = 0; i < chunkbytes; i++)
        {
            int j;
            unsigned char b = 0;
            for (j = 1; j < 256; j <<= 1)
                if (in[in_offset++])
                    b |= j;
            urj_tap_cable_cx_cmd_push (cmd_root, b);
        }
    }

    while (len > in_offset)
    {
        char tdi = in[in_offset++] ? 1 : 0;

        urj_tap_cable_cx_cmd_queue (cmd_root, out ? 1 : 0);
        urj_tap_cable_cx_cmd_push (cmd_root, OTHERS | (tdi << TDI));    /* TCK low */
        urj_tap_cable_cx_cmd_push (cmd_root,
                                   OTHERS | ((out) ? (1 << READ) : 0) | (1 <<
                                                                         TCK)
                                   | (tdi << TDI));
    }
}

static int
usbblaster_transfer_finish (urj_cable_t *cable, int len, char *out)
{
    params_t *params = cable->params;
    urj_tap_cable_cx_cmd_root_t *cmd_root = &params->cmd_root;
    int out_offset = 0;

    if (out == NULL)
        return 0;

    while (len - out_offset >= 8)
    {
        int i;
        int chunkbytes = ((len - out_offset) >> 3);
        if (chunkbytes > 63)
            chunkbytes = 63;

        if (out)
        {
            urj_tap_cable_cx_xfer (cmd_root, NULL, cable,
                                   URJ_TAP_CABLE_COMPLETELY);

            for (i = 0; i < chunkbytes; i++)
            {
                int j;
                unsigned char b = urj_tap_cable_cx_xfer_recv (cable);
#if 0
                urj_log (URJ_LOG_LEVEL_COMM, "read byte: %02X\n", b);
#endif

                for (j = 1; j < 256; j <<= 1)
                    out[out_offset++] = (b & j) ? 1 : 0;
            }
        }
    }

    while (len > out_offset)
        out[out_offset++] =
            (urj_tap_cable_cx_xfer_recv (cable) & (1 << TDO)) ? 1 : 0;

#if 0
    {
        int o;
        urj_log (URJ_LOG_LEVEL_COMM, "%d out: ", len);
        for (o = 0; o < len; o++)
            urj_log (URJ_LOG_LEVEL_COMM, "%c", out[o] ? '1' : '0');
        urj_log (URJ_LOG_LEVEL_COMM, "\n");
    }
#endif

    return 0;
}

static int
usbblaster_transfer (urj_cable_t *cable, int len, const char *in, char *out)
{
    params_t *params = cable->params;

    usbblaster_transfer_schedule (cable, len, in, out);
    urj_tap_cable_cx_xfer (&params->cmd_root, NULL, cable,
                           URJ_TAP_CABLE_COMPLETELY);
    return usbblaster_transfer_finish (cable, len, out);
}

static void
usbblaster_flush (urj_cable_t *cable, urj_cable_flush_amount_t how_much)
{
    params_t *params = cable->params;

    if (how_much == URJ_TAP_CABLE_OPTIONALLY)
        return;

    if (cable->todo.num_items == 0)
        urj_tap_cable_cx_xfer (&params->cmd_root, NULL, cable, how_much);

    while (cable->todo.num_items > 0)
    {
        int i, j, n;

        for (j = i = cable->todo.next_item, n = 0; n < cable->todo.num_items;
             n++)
        {

            switch (cable->todo.data[i].action)
            {
            case URJ_TAP_CABLE_CLOCK:
                usbblaster_clock_schedule (cable,
                                           cable->todo.data[i].arg.clock.tms,
                                           cable->todo.data[i].arg.clock.tdi,
                                           cable->todo.data[i].arg.clock.n);
                break;

            case URJ_TAP_CABLE_GET_TDO:
                usbblaster_get_tdo_schedule (cable);
                break;

            case URJ_TAP_CABLE_TRANSFER:
                usbblaster_transfer_schedule (cable,
                                              cable->todo.data[i].arg.
                                              transfer.len,
                                              cable->todo.data[i].arg.
                                              transfer.in,
                                              cable->todo.data[i].arg.
                                              transfer.out);
                break;

            default:
                break;
            }

            i++;
            if (i >= cable->todo.max_items)
                i = 0;
        }

        urj_tap_cable_cx_xfer (&params->cmd_root, NULL, cable, how_much);

        while (j != i)
        {
            switch (cable->todo.data[j].action)
            {
            case URJ_TAP_CABLE_GET_TDO:
                {
                    int m;
                    m = urj_tap_cable_add_queue_item (cable, &cable->done);
                    cable->done.data[m].action = URJ_TAP_CABLE_GET_TDO;
                    cable->done.data[m].arg.value.val =
                        usbblaster_get_tdo_finish (cable);
                    break;
                }
            case URJ_TAP_CABLE_GET_SIGNAL:
                {
                    int m =
                        urj_tap_cable_add_queue_item (cable, &cable->done);
                    cable->done.data[m].action = URJ_TAP_CABLE_GET_SIGNAL;
                    cable->done.data[m].arg.value.sig =
                        cable->todo.data[j].arg.value.sig;
                    if (cable->todo.data[j].arg.value.sig == URJ_POD_CS_TRST)
                        cable->done.data[m].arg.value.val = 1;
                    else
                        cable->done.data[m].arg.value.val = -1; // not supported yet
                    break;
                }
            case URJ_TAP_CABLE_TRANSFER:
                {
                    int r = usbblaster_transfer_finish (cable,
                                                        cable->todo.data[j].
                                                        arg.transfer.len,
                                                        cable->todo.data[j].
                                                        arg.transfer.out);
                    free (cable->todo.data[j].arg.transfer.in);
                    if (cable->todo.data[j].arg.transfer.out)
                    {
                        int m = urj_tap_cable_add_queue_item (cable,
                                                              &cable->done);
                        if (m < 0)
                        {
                            // retain error state
                            // urj_log (URJ_LOG_LEVEL_NORMAL, "out of memory!\n");
                        }
                        cable->done.data[m].action = URJ_TAP_CABLE_TRANSFER;
                        cable->done.data[m].arg.xferred.len =
                            cable->todo.data[j].arg.transfer.len;
                        cable->done.data[m].arg.xferred.res = r;
                        cable->done.data[m].arg.xferred.out =
                            cable->todo.data[j].arg.transfer.out;
                    }
                }
            default:
                break;
            }

            j++;
            if (j >= cable->todo.max_items)
                j = 0;
            cable->todo.num_items--;
        }

        cable->todo.next_item = i;
    }
}

const urj_cable_driver_t urj_tap_cable_usbblaster_driver = {
    "UsbBlaster",
    N_("Altera USB-Blaster Cable"),
    URJ_CABLE_DEVICE_USB,
    { .usb = usbblaster_connect, },
    urj_tap_cable_generic_disconnect,
    usbblaster_cable_free,
    usbblaster_init,
    urj_tap_cable_generic_usbconn_done,
    usbblaster_set_frequency,
    usbblaster_clock,
    usbblaster_get_tdo,
    usbblaster_transfer,
    usbblaster_set_signal,
    urj_tap_cable_generic_get_signal,
//      urj_tap_cable_generic_flush_one_by_one,
//      urj_tap_cable_generic_flush_using_transfer,
    usbblaster_flush,
    ftdx_usbcable_help
};
URJ_DECLARE_FTDX_CABLE(0x09FB, 0x6001, "", "UsbBlaster", usbblaster)
URJ_DECLARE_FTDX_CABLE(0x09FB, 0x6002, "", "UsbBlaster", cubic_cyclonium)
URJ_DECLARE_FTDX_CABLE(0x09FB, 0x6003, "", "UsbBlaster", nios_eval)
URJ_DECLARE_FTDX_CABLE(0x16C0, 0x06AD, "", "UsbBlaster", usb_jtag)
