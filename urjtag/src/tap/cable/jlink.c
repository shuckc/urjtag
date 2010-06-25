/*
 * $Id$
 *
 * Segger J-Link cable driver
 *
 * Large portions of code were taken from the OpenOCD driver written by
 * Juergen Stuber, which in turn was based on Dominic Rath's and Benedikt
 * Sauter's usbprog.c. Therefore most of this code is actually
 *
 * Copyright (C) 2007 Juergen Stuber
 *
 * Modified to work in UrJTAG by K. Waschk in 2008.
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

#define JLINK_WRITE_ENDPOINT 0x02
#define JLINK_READ_ENDPOINT  (0x01 | LIBUSB_ENDPOINT_IN)

#define JLINK_USB_TIMEOUT 100

#define JLINK_IN_BUFFER_SIZE  2064
#define JLINK_OUT_BUFFER_SIZE 2064

/* We use the maximal value observed */
#define JLINK_TAP_BUFFER_SIZE 390

typedef struct
{
    /* Global USB buffers */
    unsigned char usb_in_buffer[JLINK_IN_BUFFER_SIZE];
    unsigned char usb_out_buffer[JLINK_OUT_BUFFER_SIZE];

    int tap_length;
    uint8_t tms_buffer[JLINK_TAP_BUFFER_SIZE];
    uint8_t tdi_buffer[JLINK_TAP_BUFFER_SIZE];

    int last_tdo;
}
jlink_usbconn_data_t;

/* Constants for JLink command */
#define JLINK_DO_TRST_CYCLE                0x02
#define JLINK_SPEED_COMMAND                0x05
#define JLINK_GET_STATUS_COMMAND           0x07
#define JLINK_GET_A                        0xc1
#define JLINK_GET_B                        0xc2
#define JLINK_SET_TMS_LOW_COMMAND          0xc9
#define JLINK_SET_TMS_HIGH_COMMAND         0xca
#define JLINK_SET_TDI_LOW_COMMAND          0xcb
#define JLINK_SET_TDI_HIGH_COMMAND         0xcc
#define JLINK_TAP_SEQUENCE_COMMAND         0xcd
#define JLINK_SET_SRST_LOW_COMMAND         0xdc
#define JLINK_SET_SRST_HIGH_COMMAND        0xdd
#define JLINK_SET_TRST_LOW_COMMAND         0xde
#define JLINK_SET_TRST_HIGH_COMMAND        0xdf

#define JLINK_MAX_SPEED 12000

/* Queue command functions */
static void urj_tap_cable_jlink_reset (urj_usbconn_libusb_param_t *params,
                                       int trst, int srst);
static void jlink_simple_command (urj_usbconn_libusb_param_t *params,
                                  uint8_t command);


/* J-Link tap buffer functions */
static void jlink_tap_init (jlink_usbconn_data_t *data);
static int jlink_tap_execute (urj_usbconn_libusb_param_t *params);
static void jlink_tap_append_step (jlink_usbconn_data_t *data, int, int);

/* Jlink lowlevel functions */
static int jlink_usb_message (urj_usbconn_libusb_param_t *params, int, int);
/** @return number of bytes written; -1 on error */
static int jlink_usb_write (urj_usbconn_libusb_param_t *params, unsigned int);
/** @return number of bytes read; -1 on error */
static int jlink_usb_read (urj_usbconn_libusb_param_t *params);

static void jlink_debug_buffer (unsigned char *buffer, int length);

/* API functions */

void urj_tap_cable_jlink_set_frequency (urj_cable_t *cable,
                                        uint32_t frequency);

/***************************************************************************/
/* J-Link tap functions */

void
urj_tap_cable_jlink_reset (urj_usbconn_libusb_param_t *params, int trst,
                           int srst)
{
    urj_log (URJ_LOG_LEVEL_DETAIL, "trst: %i, srst: %i\n", trst, srst);

    /* Signals are active low */
    if (trst == 0)
    {
        jlink_simple_command (params, JLINK_SET_TRST_HIGH_COMMAND);
    }
    else if (trst == 1)
    {
        jlink_simple_command (params, JLINK_SET_TRST_LOW_COMMAND);
    }

    if (srst == 0)
    {
        jlink_simple_command (params, JLINK_SET_SRST_HIGH_COMMAND);
    }
    else if (srst == 1)
    {
        jlink_simple_command (params, JLINK_SET_SRST_LOW_COMMAND);
    }
}


static void
jlink_simple_command (urj_usbconn_libusb_param_t *params, uint8_t command)
{
    int result;
    jlink_usbconn_data_t *data = params->data;

    urj_log (URJ_LOG_LEVEL_DETAIL, "simple_command: 0x%02x\n", command);

    data->usb_out_buffer[0] = command;
    result = jlink_usb_write (params, 1);

    if (result != 1)
    {
        urj_log (URJ_LOG_LEVEL_ERROR, "J-Link command 0x%02x failed (%d)\n",
                 command, result);
    }
}

static int
jlink_get_status (urj_usbconn_libusb_param_t *params)
{
    int result;
    jlink_usbconn_data_t *data = params->data;

    jlink_simple_command (params, 0x07);

    result = jlink_usb_read (params);

    if (result == 8)
    {
        int vref = data->usb_in_buffer[0] + (data->usb_in_buffer[1] << 8);
        // @@@@ RFHH how can we handle this gracefully in liburjtag?
        urj_log (URJ_LOG_LEVEL_NORMAL,
                 "Vref = %d.%d TCK=%d TDI=%d TDO=%d TMS=%d TRES=%d TRST=%d\n",
                 vref / 1000, vref % 1000,
                 data->usb_in_buffer[2],
                 data->usb_in_buffer[3],
                 data->usb_in_buffer[4],
                 data->usb_in_buffer[5],
                 data->usb_in_buffer[6], data->usb_in_buffer[7]);
        if (vref < 1500)
        {
            urj_error_set (URJ_ERROR_USB,
                           "Vref too low. Possibly the target isn't powered or disconnected?");
            result = -1;
        }
    }
    else
    {
        urj_error_set (URJ_ERROR_USB,
                       "J-Link command 0x07 (get status) failed (%d)\n",
                       result);
        result = -1;
    }

    return result;
}


/***************************************************************************/

static void
jlink_tap_init (jlink_usbconn_data_t *data)
{
    data->tap_length = 0;
}

static void
jlink_tap_append_step (jlink_usbconn_data_t *data, int tms, int tdi)
{
    int index = data->tap_length >> 3;

    if (index < JLINK_TAP_BUFFER_SIZE)
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
        urj_log (URJ_LOG_LEVEL_ERROR, "jlink_tap_append_step, overflow\n");
    }
}

/* Send a tap sequence to the device, and receive the answer */

static int
jlink_tap_execute (urj_usbconn_libusb_param_t *params)
{
    jlink_usbconn_data_t *data = params->data;
    int byte_length;
    int tms_offset;
    int tdi_offset;
    int i;
    int result;

    if (data->tap_length > 0)
    {
        byte_length = (data->tap_length + 7) >> 3;
        data->usb_out_buffer[0] = JLINK_TAP_SEQUENCE_COMMAND;
        data->usb_out_buffer[1] = (data->tap_length >> 0) & 0xff;
        data->usb_out_buffer[2] = (data->tap_length >> 8) & 0xff;

        tms_offset = 3;
        for (i = 0; i < byte_length; i++)
        {
            data->usb_out_buffer[tms_offset + i] = data->tms_buffer[i];
        }

        tdi_offset = tms_offset + byte_length;
        for (i = 0; i < byte_length; i++)
        {
            data->usb_out_buffer[tdi_offset + i] = data->tdi_buffer[i];
        }

        result = jlink_usb_message (params, 3 + 2 * byte_length, byte_length);

        if (result == byte_length)
        {
            int bit_index = (data->tap_length - 1) & 7;
            uint8_t bit = 1 << bit_index;

            data->last_tdo =
                ((data->usb_in_buffer[byte_length - 1]) & bit) ? 1 : 0;
        }
        else
        {
            urj_log (URJ_LOG_LEVEL_ERROR,
                     "jlink_tap_execute, wrong result %d, expected %d\n",
                     result, byte_length);

            return -2;
        }

        jlink_tap_init (data);
    }
    return 0;
}

/* ---------------------------------------------------------------------- */

/* Send a message and receive the reply. */
static int
jlink_usb_message (urj_usbconn_libusb_param_t *params, int out_length,
                   int in_length)
{
    int result;

    result = jlink_usb_write (params, out_length);
    if (result == out_length)
    {
        result = jlink_usb_read (params);
        if (result == in_length)
        {
            return result;
        }
        else
        {
            urj_log (URJ_LOG_LEVEL_ERROR,
                     "usb_bulk_read failed (requested=%d, result=%d)\n",
                     in_length, result);

            return -1;
        }
    }
    else
    {
        urj_log (URJ_LOG_LEVEL_ERROR,
                 "usb_bulk_write failed (requested=%d, result=%d)\n",
                 out_length, result);

        return -1;
    }
}

/* ---------------------------------------------------------------------- */

/* Write data from out_buffer to USB. */
static int
jlink_usb_write (urj_usbconn_libusb_param_t *params, unsigned int out_length)
{
    int result, actual;
    jlink_usbconn_data_t *data;

    data = params->data;

    if (out_length > JLINK_OUT_BUFFER_SIZE)
    {
        urj_log (URJ_LOG_LEVEL_ERROR,
                 "jlink_jtag_write illegal out_length=%d (max=%d)\n",
                 out_length, JLINK_OUT_BUFFER_SIZE);

        return -1;
    }

    result = libusb_bulk_transfer (params->handle,
                                   JLINK_WRITE_ENDPOINT,
                                   data->usb_out_buffer,
                                   out_length, &actual,
                                   JLINK_USB_TIMEOUT);

    urj_log (URJ_LOG_LEVEL_DETAIL,
             "jlink_usb_write, out_length = %d, result = %d, actual = %d\n",
             out_length, result, actual);
    jlink_debug_buffer (data->usb_out_buffer, out_length);

    return actual;
}

/* ---------------------------------------------------------------------- */

/* Read data from USB into in_buffer. */
static int
jlink_usb_read (urj_usbconn_libusb_param_t *params)
{
    jlink_usbconn_data_t *data = params->data;

    int result, actual;

    result = libusb_bulk_transfer (params->handle,
                                   JLINK_READ_ENDPOINT,
                                   data->usb_in_buffer,
                                   JLINK_IN_BUFFER_SIZE,
                                   &actual,
                                   JLINK_USB_TIMEOUT);

    urj_log (URJ_LOG_LEVEL_DETAIL, "jlink_usb_read, result = %d, actual = %d\n",
             result, actual);
    jlink_debug_buffer (data->usb_in_buffer, actual);

    return actual;
}

/* ---------------------------------------------------------------------- */

#define BYTES_PER_LINE  16

static void
jlink_debug_buffer (unsigned char *buffer, int length)
{
    char line[81];
    char s[4];
    int i;
    int j;

    for (i = 0; i < length; i += BYTES_PER_LINE)
    {
        snprintf (line, 5, "%04x", i);
        for (j = i; j < i + BYTES_PER_LINE && j < length; j++)
        {
            snprintf (s, 4, " %02x", buffer[j]);
            strcat (line, s);
        }
        urj_log (URJ_LOG_LEVEL_DETAIL, "%s\n", line);
    }
}

/* ---------------------------------------------------------------------- */

static int
jlink_init (urj_cable_t *cable)
{
    int result;
    urj_usbconn_libusb_param_t *params;
    jlink_usbconn_data_t *data;

    params = cable->link.usb->params;
    params->data = malloc (sizeof (jlink_usbconn_data_t));
    if (params->data == NULL)
    {
        urj_error_set (URJ_ERROR_OUT_OF_MEMORY, "malloc(%zd) fails",
                       sizeof (jlink_usbconn_data_t));
        return URJ_STATUS_FAIL;
    }
    data = params->data;

    if (urj_tap_usbconn_open (cable->link.usb) != URJ_STATUS_OK)
        return URJ_STATUS_FAIL;

    jlink_tap_init (data);

    result = jlink_usb_read (params);

    if (result != 2 || data->usb_in_buffer[0] != 0x07
        || data->usb_in_buffer[1] != 0x00)
    {
        urj_log (URJ_LOG_LEVEL_NORMAL,
                 "J-Link initial read failed, don't worry (result=%d)\n",
                 result);
    }

    result = jlink_get_status (params);
    if (result < 0)
    {
        // retain error state
        urj_log (URJ_LOG_LEVEL_ERROR,
                 "Resetting J-Link. Please retry the cable command.\n");
        libusb_reset_device (params->handle);
        return URJ_STATUS_FAIL;
    }

    urj_log (URJ_LOG_LEVEL_NORMAL, "J-Link JTAG Interface ready\n");

    urj_tap_cable_jlink_set_frequency (cable, 4E6);

    urj_tap_cable_jlink_reset (params, 0, 0);

    return URJ_STATUS_OK;
}

/* ---------------------------------------------------------------------- */

static void
jlink_free (urj_cable_t *cable)
{
    jlink_usbconn_data_t *data;
    data = ((urj_usbconn_libusb_param_t *) (cable->link.usb->params))->data;
    free (data);

    urj_tap_cable_generic_usbconn_free (cable);
}

/* ---------------------------------------------------------------------- */

void
urj_tap_cable_jlink_set_frequency (urj_cable_t *cable, uint32_t frequency)
{
    int result;
    int speed = frequency / 1E3;
    urj_usbconn_libusb_param_t *params = cable->link.usb->params;
    jlink_usbconn_data_t *data = params->data;

    if (1 <= speed && speed <= JLINK_MAX_SPEED)
    {
        data->usb_out_buffer[0] = JLINK_SPEED_COMMAND;
        /* speed = 0xFFFF for automatic (probably needs RTCK) */
        data->usb_out_buffer[1] = (speed >> 0) & 0xff;
        data->usb_out_buffer[2] = (speed >> 8) & 0xff;

        result = jlink_usb_write (params, 3);

        if (result != 3)
        {
            urj_log (URJ_LOG_LEVEL_ERROR,
                     "J-Link setting speed failed (%d)\n", result);
        }
    }
    else
    {
        urj_log (URJ_LOG_LEVEL_NORMAL,
                 "Requested speed %dkHz exceeds maximum of %dkHz, ignored\n",
                 speed, JLINK_MAX_SPEED);
    }
}

/* ---------------------------------------------------------------------- */

static void
jlink_clock (urj_cable_t *cable, int tms, int tdi, int n)
{
    int i;
    urj_usbconn_libusb_param_t *params = cable->link.usb->params;
    jlink_usbconn_data_t *data = params->data;

    for (i = 0; i < n; i++)
    {
        jlink_tap_append_step (data, tms, tdi);
    }
    jlink_tap_execute (params);
}

/* ---------------------------------------------------------------------- */

static int
jlink_get_tdo (urj_cable_t *cable)
{
    urj_usbconn_libusb_param_t *params = cable->link.usb->params;
    jlink_usbconn_data_t *data = params->data;

    // TODO: This is the TDO _before_ last clock occured
    // ...   Anyone knows how to get the current TDO state?

    return data->last_tdo;
}

/* ---------------------------------------------------------------------- */

static void
jlink_copy_out_data (jlink_usbconn_data_t *data, int len, int offset,
                     char *buf)
{
    int i;
    for (i = 0; i < len; i++)
    {
        int bit = (1 << (i & 7));
        int byte = i >> 3;
        buf[offset + i] = (data->usb_in_buffer[byte] & bit) ? 1 : 0;
    }
}

static int
jlink_transfer (urj_cable_t *cable, int len, const char *in, char *out)
{
    int i, j;
    urj_usbconn_libusb_param_t *params = cable->link.usb->params;
    jlink_usbconn_data_t *data = params->data;

    for (j = 0, i = 0; i < len; i++)
    {
        jlink_tap_append_step (data, 0, in[i]);

        if (data->tap_length >= 8 * JLINK_TAP_BUFFER_SIZE)
        {
            jlink_tap_execute (params);
            if (out)
                jlink_copy_out_data (data, i - j, j, out);
            j = i;
        }
    }
    if (data->tap_length > 0)
    {
        jlink_tap_execute (params);
        if (out)
            jlink_copy_out_data (data, i - j, j, out);
    }

    return i;
}

/* ---------------------------------------------------------------------- */

static int
jlink_set_signal (urj_cable_t *cable, int mask, int val)
{
    return 1;
}

const urj_cable_driver_t urj_tap_cable_jlink_driver = {
    "jlink",
    N_("Segger/IAR J-Link, Atmel SAM-ICE and others."),
    URJ_CABLE_DEVICE_USB,
    { .usb = urj_tap_cable_generic_usbconn_connect, },
    urj_tap_cable_generic_disconnect,
    jlink_free,
    jlink_init,
    urj_tap_cable_generic_usbconn_done,
    urj_tap_cable_jlink_set_frequency,
    jlink_clock,
    jlink_get_tdo,
    jlink_transfer,
    jlink_set_signal,
    urj_tap_cable_generic_get_signal,
    urj_tap_cable_generic_flush_using_transfer,
    urj_tap_cable_generic_usbconn_help
};
URJ_DECLARE_USBCONN_CABLE(0x1366, 0x0101, "libusb", "jlink", jlink)
