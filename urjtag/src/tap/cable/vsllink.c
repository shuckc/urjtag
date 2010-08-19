/*
 * $Id: vsllink.c
 *
 * Versaloon cable driver
 *
 * Copyright (C) 2010 SimonQian
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
 */

#include <sysdep.h>

#include <string.h>
#include <stdlib.h>

#include "generic.h"
#include "generic_usbconn.h"

#include <urjtag/usbconn.h>

/* ---------------------------------------------------------------------- */


#include <urjtag/cable.h>
#include <urjtag/chain.h>

#include "usbconn/libusb.h"

#define VERSALOON_INP              0x82
#define VERSALOON_OUTP             0x03
#define VERSALOON_USB_TIMEOUT      1000

typedef struct
{
    /* Global USB buffers */
    unsigned char *usb_buffer;
    uint32_t usb_buffer_size;

    int tap_length;
    uint8_t *tms_buffer;
    uint8_t *tdi_buffer;
    uint32_t tap_buffer_size;

    int last_tdo;
}
vsllink_usbconn_data_t;

/* Constants for VSLLink command */
#define VERSALOON_GET_INFO     0x00
#define VERSALOON_GET_TVCC     0x01
#define VERSALOON_USB_TO_XXX_CMD_START    0x20
#define USB_TO_GPIO            (VERSALOON_USB_TO_XXX_CMD_START + 0x03)
#define USB_TO_JTAG_RAW        (VERSALOON_USB_TO_XXX_CMD_START + 0x27)
#define USB_TO_DELAY           (VERSALOON_USB_TO_XXX_CMD_START + 0x41)
#define USB_TO_ALL             (VERSALOON_USB_TO_XXX_CMD_START + 0x5F)

#define USB_TO_XXX_CMDSHIFT    3
#define USB_TO_XXX_INIT        (0x00 << USB_TO_XXX_CMDSHIFT)
#define USB_TO_XXX_FINI        (0x01 << USB_TO_XXX_CMDSHIFT)
#define USB_TO_XXX_CONFIG      (0x02 << USB_TO_XXX_CMDSHIFT)
#define USB_TO_XXX_IN_OUT      (0x05 << USB_TO_XXX_CMDSHIFT)
#define USB_TO_XXX_IN          (0x06 << USB_TO_XXX_CMDSHIFT)
#define USB_TO_XXX_OUT         (0x07 << USB_TO_XXX_CMDSHIFT)

#define GPIO_SRST              (1 << 0)
#define GPIO_TRST              (1 << 1)

/* VSLlink lowlevel functions */
static int vsllink_usb_message (urj_usbconn_libusb_param_t *params, int, int, int);
static void vsllink_free (urj_cable_t *cable);

/***************************************************************************/

static void
vsllink_tap_append_step (vsllink_usbconn_data_t *data, int tms, int tdi)
{
    int index = data->tap_length >> 3;

    if (index < data->tap_buffer_size)
    {
        int bit_index = data->tap_length & 7;
        uint8_t bit = 1 << bit_index;

        if (bit_index == 0)
        {
            data->tms_buffer[index] = 0;
            data->tdi_buffer[index] = 0;
        }

        if (tms)
            data->tms_buffer[index] |= bit;
        if (tdi)
            data->tdi_buffer[index] |= bit;

        data->tap_length++;
    }
    else
    {
        urj_log (URJ_LOG_LEVEL_ERROR, _("tap buffer overflowed\n"));
    }
}

/* Send a tap sequence to the device, and receive the answer */

static int
vsllink_tap_execute (urj_usbconn_libusb_param_t *params)
{
    vsllink_usbconn_data_t *data = params->data;
    int byte_length;
    int in_length, out_length;
    int result;

    if (data->tap_length > 0)
    {
        byte_length = (data->tap_length + 7) >> 3;

        out_length = 0;
        data->usb_buffer[out_length++] = USB_TO_JTAG_RAW;
        data->usb_buffer[out_length++] = ((10 + 2 * byte_length) >> 0) & 0xFF;
        data->usb_buffer[out_length++] = ((10 + 2 * byte_length) >> 8) & 0xFF;
        data->usb_buffer[out_length++] = USB_TO_XXX_IN_OUT;
        data->usb_buffer[out_length++] = ((4 + 2 * byte_length) >> 0) & 0xFF;
        data->usb_buffer[out_length++] = ((4 + 2 * byte_length) >> 8) & 0xFF;
        data->usb_buffer[out_length++] = (data->tap_length >>  0) & 0xFF;
        data->usb_buffer[out_length++] = (data->tap_length >>  8) & 0xFF;
        data->usb_buffer[out_length++] = (data->tap_length >> 16) & 0xFF;
        data->usb_buffer[out_length++] = (data->tap_length >> 24) & 0xFF;
        memcpy (&data->usb_buffer[out_length], data->tdi_buffer, byte_length);
        out_length += byte_length;
        memcpy (&data->usb_buffer[out_length], data->tms_buffer, byte_length);
        out_length += byte_length;
        in_length = 1 + byte_length;
        result = vsllink_usb_message (params, out_length, in_length,
                                      VERSALOON_USB_TIMEOUT);

        if (result == (1 + byte_length))
        {
            if (data->usb_buffer[0] == 0)
            {
                int bit_index = (data->tap_length - 1) & 7;
                uint8_t bit = 1 << bit_index;

                data->last_tdo = (data->usb_buffer[byte_length] & bit) ? 1 : 0;
            }
            else
            {
                urj_log (URJ_LOG_LEVEL_ERROR,
                         _("tap execute failure (%d)\n"),
                         data->usb_buffer[0]);

                return URJ_STATUS_FAIL;
            }
        }
        else
        {
            urj_log (URJ_LOG_LEVEL_ERROR,
                     _("wrong result %d, expected %d\n"),
                     result, 1 + byte_length);

            return URJ_STATUS_FAIL;
        }

        data->tap_length = 0;
    }

    return URJ_STATUS_OK;
}

/* ---------------------------------------------------------------------- */

/* Send a message and receive the reply. */

static int
vsllink_usb_message (urj_usbconn_libusb_param_t *params, int out_length,
                     int in_length, int timeout)
{
    vsllink_usbconn_data_t *data = params->data;
    int result, actual;

    result = libusb_bulk_transfer (params->handle,
                                   VERSALOON_OUTP,
                                   data->usb_buffer,
                                   out_length,
                                   &actual,
                                   timeout);
    if (actual == out_length)
    {
        result = libusb_bulk_transfer (params->handle,
                                       VERSALOON_INP,
                                       data->usb_buffer,
                                       data->usb_buffer_size,
                                       &actual,
                                       timeout);

        if (in_length == 0 || actual == in_length)
        {
            return actual;
        }
        else
        {
            urj_log (URJ_LOG_LEVEL_ERROR,
                     _("usb read failure (requested=%d, result=%d)\n"),
                     in_length, result);

            return -1;
        }
    }
    else
    {
        urj_log (URJ_LOG_LEVEL_ERROR,
                 _("usb write failure (requested=%d, result=%d)\n"),
                 out_length, result);

        return -1;
    }
}

/* ---------------------------------------------------------------------- */

static int
vsllink_init (urj_cable_t *cable)
{
    int result, in_length, out_length;
    int retry;
    urj_usbconn_libusb_param_t *params;
    vsllink_usbconn_data_t *data;

    params = cable->link.usb->params;
    params->data = malloc (sizeof (*data));
    if (params->data == NULL)
    {
        urj_error_set (URJ_ERROR_OUT_OF_MEMORY, _("malloc(%zd) fails"),
                       sizeof (vsllink_usbconn_data_t));
        return URJ_STATUS_FAIL;
    }
    data = params->data;
    memset (data, 0, sizeof (*data));

    if (urj_tap_usbconn_open (cable->link.usb) != URJ_STATUS_OK)
    {
        free (data);
        return URJ_STATUS_FAIL;
    }

    /* malloc temporary buffer */
    data->usb_buffer_size = 256;
    data->usb_buffer = malloc (data->usb_buffer_size);
    if (data->usb_buffer == NULL)
    {
        free (data);
        data = NULL;
        params->data = NULL;
        urj_error_set (URJ_ERROR_OUT_OF_MEMORY, _("malloc(%d) fails"),
                       data->usb_buffer_size);
        return URJ_STATUS_FAIL;
    }

    /* disable cdc device */
    result = usb_control_msg (params->handle,
                              USB_TYPE_VENDOR | USB_RECIP_INTERFACE,
                              0, 0, 0, NULL, 0,
                              VERSALOON_USB_TIMEOUT);
    if (result < 0)
    {
        urj_log (URJ_LOG_LEVEL_ERROR, _("fail to disable cdc in Versaloon\n"));
        vsllink_free (cable);
        return URJ_STATUS_FAIL;
    }

    /* connect to versaloon */
    in_length = 0;
    for (retry = 0; retry < 3; retry++)
    {
        data->usb_buffer[0] = VERSALOON_GET_INFO;
        result = vsllink_usb_message (params, 1, in_length, 100);
        if (result >= 3)
            break;
    }
    if (retry == 3)
    {
        vsllink_free (cable);
        return URJ_STATUS_FAIL;
    }

    data->usb_buffer[result] = '\0';
    data->usb_buffer_size = data->usb_buffer[0] + (data->usb_buffer[1] << 8);
    if (data->usb_buffer_size < 64)
    {
        vsllink_free (cable);
        return URJ_STATUS_FAIL;
    }
    urj_log (URJ_LOG_LEVEL_NORMAL, _("%s(buffer size %d bytes)\n"),
             &data->usb_buffer[2], data->usb_buffer_size);

    /* free temporary buffer and malloc new buffer */
    free (data->usb_buffer);
    data->tap_buffer_size = (data->usb_buffer_size - 64) / 2;
    data->usb_buffer = malloc (data->usb_buffer_size);
    data->tms_buffer = malloc (data->tap_buffer_size);
    data->tdi_buffer = malloc (data->tap_buffer_size);
    if (data->usb_buffer == NULL ||
        data->tms_buffer == NULL ||
        data->tdi_buffer == NULL)
    {
        urj_error_set (URJ_ERROR_OUT_OF_MEMORY, _("malloc() fails"));
        vsllink_free (cable);
        return URJ_STATUS_FAIL;
    }

    data->usb_buffer[0] = VERSALOON_GET_TVCC;
    in_length = 2;
    result = vsllink_usb_message (params, 1, in_length, 100);
    if (result < 0)
    {
        vsllink_free (cable);
        return URJ_STATUS_FAIL;
    }

    out_length = 0;
    data->usb_buffer[out_length++] = USB_TO_ALL;
    data->usb_buffer[out_length++] = 0x24;
    data->usb_buffer[out_length++] = 0x00;
    data->usb_buffer[out_length++] = USB_TO_DELAY;
    data->usb_buffer[out_length++] = 0x05;
    data->usb_buffer[out_length++] = 0x00;
    data->usb_buffer[out_length++] = 0x64;
    data->usb_buffer[out_length++] = 0x80;
    data->usb_buffer[out_length++] = USB_TO_JTAG_RAW;
    data->usb_buffer[out_length++] = 0x0C;
    data->usb_buffer[out_length++] = 0x00;
    data->usb_buffer[out_length++] = USB_TO_XXX_INIT;
    data->usb_buffer[out_length++] = 0x01;
    data->usb_buffer[out_length++] = 0x00;
    data->usb_buffer[out_length++] = 0x00;
    data->usb_buffer[out_length++] = USB_TO_XXX_CONFIG;
    data->usb_buffer[out_length++] = 0x02;
    data->usb_buffer[out_length++] = 0x00;
    data->usb_buffer[out_length++] = 0x01;
    data->usb_buffer[out_length++] = 0x00;
    data->usb_buffer[out_length++] = USB_TO_GPIO;
    data->usb_buffer[out_length++] = 0x10;
    data->usb_buffer[out_length++] = 0x00;
    data->usb_buffer[out_length++] = USB_TO_XXX_INIT;
    data->usb_buffer[out_length++] = 0x01;
    data->usb_buffer[out_length++] = 0x00;
    data->usb_buffer[out_length++] = 0x00;
    data->usb_buffer[out_length++] = USB_TO_XXX_CONFIG;
    data->usb_buffer[out_length++] = 0x06;
    data->usb_buffer[out_length++] = 0x00;
    data->usb_buffer[out_length++] = 0x03;
    data->usb_buffer[out_length++] = 0x00;
    data->usb_buffer[out_length++] = 0x02;
    data->usb_buffer[out_length++] = 0x00;
    data->usb_buffer[out_length++] = 0x03;
    data->usb_buffer[out_length++] = 0x00;
    in_length = 7;
    result = vsllink_usb_message (params, out_length, in_length, 500);
    if ((result < 0)
            /* ack to USB_TO_DELAY */
        || (data->usb_buffer[0] != 0)
            /* ack to USB_TO_JTAG_RAW->UB_TO_XXX_INIT */
        || (data->usb_buffer[1] != 0) || (data->usb_buffer[2] < 1)
            /* ack to USB_TO_JTAG_RAW->UB_TO_XXX_CONFIG */
        || (data->usb_buffer[3] != 0)
            /* ack to USB_TO_GPIO->UB_TO_XXX_INIT */
        || (data->usb_buffer[4] != 0) || (data->usb_buffer[5] < 1)
            /* ack to USB_TO_GPIO->UB_TO_XXX_CONFIG */
        || (data->usb_buffer[6] != 0))
    {
        vsllink_free (cable);
        return URJ_STATUS_FAIL;
    }

    data->tap_length = 0;
    urj_log (URJ_LOG_LEVEL_NORMAL, _("Versaloon JTAG Interface ready\n"));

    return URJ_STATUS_OK;
}

/* ---------------------------------------------------------------------- */

static void
vsllink_done (urj_cable_t *cable)
{
    int result;
    int in_length, out_length;
    urj_usbconn_libusb_param_t *params = cable->link.usb->params;
    vsllink_usbconn_data_t *data = params->data;

    out_length = 0;
    data->usb_buffer[out_length++] = USB_TO_ALL;
    data->usb_buffer[out_length++] = 0x11;
    data->usb_buffer[out_length++] = 0x00;
    data->usb_buffer[out_length++] = USB_TO_JTAG_RAW;
    data->usb_buffer[out_length++] = 0x07;
    data->usb_buffer[out_length++] = 0x00;
    data->usb_buffer[out_length++] = USB_TO_XXX_FINI;
    data->usb_buffer[out_length++] = 0x01;
    data->usb_buffer[out_length++] = 0x00;
    data->usb_buffer[out_length++] = 0x00;
    data->usb_buffer[out_length++] = USB_TO_GPIO;
    data->usb_buffer[out_length++] = 0x07;
    data->usb_buffer[out_length++] = 0x00;
    data->usb_buffer[out_length++] = USB_TO_XXX_FINI;
    data->usb_buffer[out_length++] = 0x01;
    data->usb_buffer[out_length++] = 0x00;
    data->usb_buffer[out_length++] = 0x00;
    in_length = 2;
    result = vsllink_usb_message (params, out_length, in_length, 100);
    if (result < 0 ||
        data->usb_buffer[0] != 0 ||
        data->usb_buffer[1] != 0)
    {
        /* failure, go on anyway */
    }

    urj_tap_cable_generic_usbconn_done (cable);
}

static void
vsllink_free (urj_cable_t *cable)
{
    urj_usbconn_libusb_param_t *params = cable->link.usb->params;
    vsllink_usbconn_data_t *data = params->data;

    if (data != NULL)
    {
        free (data->usb_buffer);
        free (data->tms_buffer);
        free (data->tdi_buffer);
        free (data);
    }

    urj_tap_cable_generic_usbconn_free (cable);
}

/* ---------------------------------------------------------------------- */

static void
vsllink_set_frequency (urj_cable_t *cable, uint32_t frequency)
{
    int result;
    int in_length, out_length;
    uint16_t kHz = frequency / 1E3;
    urj_usbconn_libusb_param_t *params = cable->link.usb->params;
    vsllink_usbconn_data_t *data = params->data;

    out_length = 0;
    data->usb_buffer[out_length++] = USB_TO_JTAG_RAW;
    data->usb_buffer[out_length++] = 0x08;
    data->usb_buffer[out_length++] = 0x00;
    data->usb_buffer[out_length++] = USB_TO_XXX_CONFIG;
    data->usb_buffer[out_length++] = 0x02;
    data->usb_buffer[out_length++] = 0x00;
    data->usb_buffer[out_length++] = (kHz >> 0) & 0xFF;
    data->usb_buffer[out_length++] = (kHz >> 8) & 0xFF;
    in_length = 1;
    result = vsllink_usb_message (params, out_length, in_length, 100);
    if ((result < 0)
            /* ack to USB_TO_JTAG_RAW->UB_TO_XXX_CONFIG */
        || (data->usb_buffer[0] != 0))
    {
        urj_log (URJ_LOG_LEVEL_ERROR,
                _("Versaloon setting JTAG speed failed (%d)\n"), result);
    }
}

/* ---------------------------------------------------------------------- */

static void
vsllink_clock (urj_cable_t *cable, int tms, int tdi, int n)
{
    int i;
    urj_usbconn_libusb_param_t *params = cable->link.usb->params;
    vsllink_usbconn_data_t *data = params->data;

    for (i = 0; i < n; i++)
    {
        vsllink_tap_append_step (data, tms, tdi);
    }
    vsllink_tap_execute (params);
}

/* ---------------------------------------------------------------------- */

static int
vsllink_get_tdo (urj_cable_t *cable)
{
    urj_usbconn_libusb_param_t *params = cable->link.usb->params;
    vsllink_usbconn_data_t *data = params->data;

    return data->last_tdo;
}

/* ---------------------------------------------------------------------- */

static void
vsllink_copy_out_data (vsllink_usbconn_data_t *data, int len, int offset,
                       char *buf)
{
    int i;
    for (i = 0; i < len; i++)
    {
        int bit = (1 << (i & 7));
        int byte = i >> 3;
        buf[offset + i] = (data->usb_buffer[1 + byte] & bit) ? 1 : 0;
    }
}

static int
vsllink_transfer (urj_cable_t *cable, int len, const char *in, char *out)
{
    int i, j;
    urj_usbconn_libusb_param_t *params = cable->link.usb->params;
    vsllink_usbconn_data_t *data = params->data;

    for (j = 0, i = 0; i < len; i++)
    {
        vsllink_tap_append_step (data, 0, in[i]);

        if (data->tap_length >= 8 * data->tap_buffer_size)
        {
            vsllink_tap_execute (params);
            if (out)
                vsllink_copy_out_data (data, i - j, j, out);
            j = i;
        }
    }
    if (data->tap_length > 0)
    {
        vsllink_tap_execute (params);
        if (out)
            vsllink_copy_out_data (data, i - j, j, out);
    }

    return i;
}

/* ---------------------------------------------------------------------- */

static int
vsllink_set_signal (urj_cable_t *cable, int mask, int val)
{
    return 1;
}

const urj_cable_driver_t urj_tap_cable_vsllink_driver = {
    "vsllink",
    N_("Versaloon Link -- http://www.versaloon.com."),
    URJ_CABLE_DEVICE_USB,
    { .usb = urj_tap_cable_generic_usbconn_connect, },
    urj_tap_cable_generic_disconnect,
    vsllink_free,
    vsllink_init,
    vsllink_done,
    vsllink_set_frequency,
    vsllink_clock,
    vsllink_get_tdo,
    vsllink_transfer,
    vsllink_set_signal,
    urj_tap_cable_generic_get_signal,
    urj_tap_cable_generic_flush_using_transfer,
    urj_tap_cable_generic_usbconn_help
};
URJ_DECLARE_USBCONN_CABLE (0x0483, 0x5740, "libusb", "vsllink", vsllink)
