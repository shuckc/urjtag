/*
 * $Id: jlink.c,v 1.8 2003/08/19 08:42:20 telka Exp $
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


#include "generic.h"
#include "generic_usbconn.h"

#include "usbconn.h"
#include "usbconn/libusb.h"

/* ---------------------------------------------------------------------- */


#include "sysdep.h"

#include "cable.h"
#include "chain.h"

#include "jtag.h"

#include <usb.h>
#include <string.h>

#define INFO     printf
#define ERROR    printf
#define DEBUG    printf

#define JLINK_WRITE_ENDPOINT 0x02
#define JLINK_READ_ENDPOINT  0x81

#define JLINK_USB_TIMEOUT 100

#define JLINK_IN_BUFFER_SIZE  2064
#define JLINK_OUT_BUFFER_SIZE 2064

/* Global USB buffers */
static char usb_in_buffer[JLINK_IN_BUFFER_SIZE];
static char usb_out_buffer[JLINK_OUT_BUFFER_SIZE];

/* Constants for JLink command */
#define JLINK_SPEED_COMMAND                0x05
#define JLINK_TAP_SEQUENCE_COMMAND         0xcd
#define JLINK_SET_SRST_LOW_COMMAND         0xdc
#define JLINK_SET_SRST_HIGH_COMMAND        0xdd
#define JLINK_SET_TRST_LOW_COMMAND         0xde
#define JLINK_SET_TRST_HIGH_COMMAND        0xdf

#define JLINK_MAX_SPEED 12000


/* Queue command functions */
static void jlink_reset( cable_t *cable, int trst, int srst);
static void jlink_simple_command( cable_t *cable, uint8_t command );

/* J-Link tap buffer functions */
static void jlink_tap_init();
static int jlink_tap_execute();
static void jlink_tap_append_step(int tms, int tdi);

/* Jlink lowlevel functions */
static int jlink_usb_message( cable_t *cable, int out_length, int in_length);
static int jlink_usb_write( cable_t *cable, unsigned int out_length);
static int jlink_usb_read( cable_t *cable );
static void jlink_debug_buffer(uint8_t *buffer, int length);



void jlink_set_frequency( cable_t *cable, uint32_t frequency )
{
    int result;
	int speed = frequency / 1E3;

    if (1 <= speed && speed <= JLINK_MAX_SPEED)
    {
        usb_out_buffer[0] = JLINK_SPEED_COMMAND;
        usb_out_buffer[1] = (speed >> 0) & 0xff;
        usb_out_buffer[2] = (speed >> 8) & 0xff;

        result = jlink_usb_write( cable, 3 );

        if (result != 3)
        {
            ERROR("J-Link setting speed failed (%d)\n", result);
        }
    }
    else
    {
        INFO("Requested speed %dkHz exceeds maximum of %dkHz, ignored\n", 
            speed, JLINK_MAX_SPEED);
    }
}

void jlink_reset( cable_t *cable, int trst, int srst)
{
    DEBUG("trst: %i, srst: %i\n", trst, srst);

    /* Signals are active low */
    if (trst == 0)
    {
        jlink_simple_command( cable, JLINK_SET_TRST_HIGH_COMMAND);
    }
    else if (trst == 1)
    {
        jlink_simple_command( cable, JLINK_SET_TRST_LOW_COMMAND);
    }

    if (srst == 0)
    {
        jlink_simple_command( cable, JLINK_SET_SRST_HIGH_COMMAND);
    }
    else if (srst == 1)
    {
        jlink_simple_command( cable, JLINK_SET_SRST_LOW_COMMAND);
    }
}


static void jlink_simple_command( cable_t *cable, uint8_t command)
{
    int result;

    DEBUG("simple_command: 0x%02x\n", command);

    usb_out_buffer[0] = command;
    result = jlink_usb_write( cable, 1 );

    if (result != 1)
    {
        ERROR("J-Link command 0x%02x failed (%d)\n", command, result);
    }
}

/***************************************************************************/
/* J-Link tap functions */

/* We use the maximal value observed */
#define JLINK_TAP_BUFFER_SIZE 390

static int tap_length;
static uint8_t tms_buffer[JLINK_TAP_BUFFER_SIZE];
static uint8_t tdi_buffer[JLINK_TAP_BUFFER_SIZE];

static int last_tdo;

static void jlink_tap_init()
{
    tap_length = 0;
}

static void jlink_tap_append_step(int tms, int tdi)
{
    int index = tap_length >> 3;

    if (index < JLINK_TAP_BUFFER_SIZE)
    {
        int bit_index = tap_length & 7;
        uint8_t bit = 1 << bit_index;

		if(bit_index == 0)
		{
			tms_buffer[index] = 0;
			tdi_buffer[index] = 0;
		};

		if(tms) tms_buffer[index] |= bit;
		if(tdi) tdi_buffer[index] |= bit;

        tap_length++;
    }
    else
    {
        ERROR("jlink_tap_append_step, overflow\n");
    }
}

/* Pad and send a tap sequence to the device, and receive the answer.
 * For the purpose of padding we assume that we are in idle or pause state. */
static int jlink_tap_execute( cable_t *cable )
{
    int byte_length;
    int tms_offset;
    int tdi_offset;
    int i;
    int result;

    if (tap_length > 0)
    {
        byte_length = (tap_length + 7) >> 3;
        usb_out_buffer[0] = JLINK_TAP_SEQUENCE_COMMAND;
        usb_out_buffer[1] = (tap_length >> 0) & 0xff;
        usb_out_buffer[2] = (tap_length >> 8) & 0xff;

        tms_offset = 3;
        for (i = 0; i < byte_length; i++)
        {
            usb_out_buffer[tms_offset + i] = tms_buffer[i];
        }

        tdi_offset = tms_offset + byte_length;
        for (i = 0; i < byte_length; i++)
        {
            usb_out_buffer[tdi_offset + i] = tdi_buffer[i];
        }

        result = jlink_usb_message(
            cable,
            3 + 2 * byte_length,
            byte_length);

        if (result == byte_length)
        {
            int bit_index = (tap_length - 1) & 7;
            uint8_t bit = 1 << bit_index;

            last_tdo = ((usb_in_buffer[byte_length-1]) & bit ) ? 1 : 0;
        }
        else
        {
            ERROR(
                "jlink_tap_execute, wrong result %d, expected %d",
                result,
                byte_length);

            return -2;
        }

        jlink_tap_init();
    }
    return 0;
}

/* ---------------------------------------------------------------------- */

/* Send a message and receive the reply. */
static int jlink_usb_message(
    cable_t *cable,
    int out_length,
    int in_length)
{
    int result;

    result = jlink_usb_write( cable, out_length );
    if (result == out_length)
    {
        result = jlink_usb_read( cable );
        if (result == in_length)
        {
            return result;
        }
        else
        {
            ERROR(
                "usb_bulk_read failed (requested=%d, result=%d)\n",
                in_length,
                result);

            return -1;
        }
    }
    else
    {
        ERROR(
            "usb_bulk_write failed (requested=%d, result=%d)\n",
            out_length,
            result);

        return -1;
    }
}

/* ---------------------------------------------------------------------- */

/* Write data from out_buffer to USB. */
static int jlink_usb_write( cable_t *cable, unsigned int out_length )
{
    int result;
    struct usb_dev_handle *jlink;
    jlink = ((libusb_param_t*)(cable->link.usb->params))->handle;

    if (out_length > JLINK_OUT_BUFFER_SIZE)
    {
        ERROR("jlink_jtag_write illegal out_length=%d (max=%d)\n", out_length, 
        JLINK_OUT_BUFFER_SIZE);

        return -1;
    }

    result = usb_bulk_write(
        jlink, 
        JLINK_WRITE_ENDPOINT,
        usb_out_buffer,
        out_length,
        JLINK_USB_TIMEOUT);

    DEBUG("jlink_usb_write, out_length = %d, result = %d\n", out_length, result);
    jlink_debug_buffer(usb_out_buffer, out_length);
    return result;
}

/* ---------------------------------------------------------------------- */

/* Read data from USB into in_buffer. */
static int jlink_usb_read( cable_t *cable )
{
    struct usb_dev_handle *jlink;
    jlink = ((libusb_param_t*)(cable->link.usb->params))->handle;

    int result = usb_bulk_read(
        jlink,
        JLINK_READ_ENDPOINT,
        usb_in_buffer, 
        JLINK_IN_BUFFER_SIZE, 
        JLINK_USB_TIMEOUT);

    DEBUG("jlink_usb_read, result = %d\n", result);
    jlink_debug_buffer(usb_in_buffer, result);
    return result;
}

/* ---------------------------------------------------------------------- */

#define BYTES_PER_LINE  16

static void jlink_debug_buffer(uint8_t *buffer, int length)
{
    char line[81];
    char s[4];
    int i;
    int j;

    for (i = 0; i < length; i += BYTES_PER_LINE)
    {
        snprintf(line, 5, "%04x", i);
        for (j = i; j < i + BYTES_PER_LINE && j < length; j++)
        {
            snprintf(s, 4, " %02x", buffer[j]);
            strcat(line, s);
        }
        DEBUG(line);
        DEBUG("\n");
    }
}

/* ---------------------------------------------------------------------- */

static int 
jlink_init( cable_t *cable )
{
    int result;
    struct usb_dev_handle *jlink;

    if (usbconn_open( cable->link.usb )) return -1;

    jlink = ((libusb_param_t*)(cable->link.usb->params))->handle;

    jlink_tap_init();

    result = jlink_usb_read( cable );
    if (result != 2 || usb_in_buffer[0] != 0x07 || usb_in_buffer[1] != 0x00)
    {
        INFO("J-Link initial read failed, don't worry (result=%d)\n", result);
    }

    INFO("J-Link JTAG Interface ready\n");

    jlink_set_frequency( cable, 4E6 );
    jlink_reset( cable, 0, 0);
    jlink_tap_init();


    PARAM_TRST(cable) = 1;

    return 0;
}

/* ---------------------------------------------------------------------- */

int tdo_cache;

static void
jlink_clock( cable_t *cable, int tms, int tdi, int n )
{
    int i;

    for (i = 0; i < n; i++)
    {
        jlink_tap_append_step(tms, tdi);
    }
    jlink_tap_execute( cable );
}

/* ---------------------------------------------------------------------- */

static int
jlink_get_tdo( cable_t *cable )
{
	// TODO: This is the TDO _before_ last clock occured
	// ...   Anyone knows how to get the current TDO state?

    return last_tdo;
}

/* ---------------------------------------------------------------------- */

void
jlink_copy_out_data( cable_t *cable, int len, int offset, char *buf )
{
    int i;
    for(i=0;i<len;i++)
    {
        int bit = (1<<(i&7));
        int byte = i>>3;
        buf[offset+i] = (usb_in_buffer[byte] & bit) ? 1 : 0;
    }
}

int
jlink_transfer( cable_t *cable, int len, char *in, char *out )
{
	int i,j;

    for(j=0, i=0; i<len; i++)
    {
        jlink_tap_append_step(0, in[i]);

        if(tap_length >= 8*JLINK_TAP_BUFFER_SIZE)
        {
            jlink_tap_execute( cable );
            if(out) jlink_copy_out_data( cable, i-j, j, out);
            j = i;
        }
    };
    if(tap_length > 0)
    {
        jlink_tap_execute( cable );
        if(out) jlink_copy_out_data( cable, i-j, j, out);
    }
    
	return i;
}

/* ---------------------------------------------------------------------- */

static int
jlink_set_trst( cable_t *cable, int trst )
{
    return 1;
}

cable_driver_t jlink_cable_driver = {
    "jlink",
    N_("Segger/IAR J-Link, Atmel SAM-ICE and others."),
    generic_usbconn_connect,
    generic_disconnect,
    generic_usbconn_free,
    jlink_init,
    generic_usbconn_done,
    jlink_set_frequency,
    jlink_clock,
    jlink_get_tdo,
    jlink_transfer,
    jlink_set_trst,
    generic_get_trst,
    generic_flush_using_transfer,
    generic_usbconn_help
};

usbconn_cable_t usbconn_cable_jlink = {
    "jlink",            /* cable name */
    NULL,                /* string pattern, not used */
    "libusb",            /* usbconn driver */ 
    0x1366,                /* VID */
    0x0101                /* PID */
};


