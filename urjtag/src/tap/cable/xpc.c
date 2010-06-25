/*
 * $Id$
 *
 * Xilinx Platform Cable USB Driver (slow GPIO only)
 * Copyright (C) 2008 Kolja Waschk
 *
 * Loosely based on Xilinx DLC5 JTAG Parallel Cable III Driver
 * Copyright (C) 2002, 2003 ETC s.r.o.
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

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#include <urjtag/cable.h>
#include <urjtag/chain.h>

#include "generic.h"
#include "generic_usbconn.h"

#include <urjtag/usbconn.h>
#include "usbconn/libusb.h"

// #define VERBOSE 1
#undef VERBOSE
typedef struct
{
    int last_tdo;
}
xpc_cable_params_t;

static int last_tdo;

/* Connectivity on Spartan-3E starter kit:
 *
 * = FX2 Port A =
 *
 *   IOA.0 => green LED (0=off)
 *   IOA.1 => red LED   (0=off)
 *   IOA.2 is tied to VCC via R25 on my board
 *   IOA.3 isn't connected
 *   IOA.4 => CPLD pin 85 (reset?)
 *   IOA.5 => CPLD pin 86, eventually OE?
 *   IOA.6 => CPLD pin 83 (reset?)
 *   IOA.7 => CPLD pin 49 (reset?)
 *
 * = FX2 Port C =
 *
 *   probably used as GPIFADR 0..7, to CPLD
 *
 * = FX2 Port E =
 *
 *   IOE.3 => CPLD TCK
 *   IOE.4 => CPLD TMS
 *   IOE.5 => CPLD TDO
 *   IOE.6 => CPLD TDI
 */

/* ---------------------------------------------------------------------- */

static int
xpcu_output_enable (struct libusb_device_handle *xpcu, int enable)
{
    if (libusb_control_transfer
        (xpcu, 0x40, 0xB0, enable ? 0x18 : 0x10, 0, NULL, 0, 1000) < 0)
    {
        urj_error_IO_set ("libusb_control_transfer(0x10/0x18)");
        return URJ_STATUS_FAIL;
    }

    return URJ_STATUS_OK;
}

/* ---------------------------------------------------------------------- */

#ifdef UNUSED                   /* RFHH */
static int
xpcu_bit_reverse (struct libusb_device_handle *xpcu, uint8_t bits_in,
                  uint8_t *bits_out)
{
    if (libusb_control_transfer
        (xpcu, 0xC0, 0xB0, 0x0020, bits_in, bits_out, 1, 1000) < 0)
    {
        urj_error_IO_set ("libusb_control_transfer(0x20.x) (bit reverse)");
        return URJ_STATUS_FAIL;
    }

    return URJ_STATUS_OK;
}
#endif

/* ----------------------------------------------------------------- */

static int
xpcu_request_28 (struct libusb_device_handle *xpcu, int value)
{
    /* Typical values seen during autodetection of chain configuration: 0x11, 0x12 */

    if (libusb_control_transfer (xpcu, 0x40, 0xB0, 0x0028, value, NULL, 0, 1000) < 0)
    {
        urj_error_IO_set ("libusb_control_transfer(0x28.x)");
        return URJ_STATUS_FAIL;
    }

    return URJ_STATUS_OK;
}

/* ---------------------------------------------------------------------- */

static int
xpcu_write_gpio (struct libusb_device_handle *xpcu, uint8_t bits)
{
    if (libusb_control_transfer (xpcu, 0x40, 0xB0, 0x0030, bits, NULL, 0, 1000) < 0)
    {
        urj_error_IO_set ("libusb_control_transfer(0x30.0x00) (write port E)");
        return URJ_STATUS_FAIL;
    }

    return URJ_STATUS_OK;
}

/* ---------------------------------------------------------------------- */

static int
xpcu_read_gpio (struct libusb_device_handle *xpcu, uint8_t *bits)
{
    if (libusb_control_transfer (xpcu, 0xC0, 0xB0, 0x0038, 0, bits, 1, 1000)
        < 0)
    {
        urj_error_IO_set ("libusb_control_transfer(0x38.0x00) (read port E)");
        return URJ_STATUS_FAIL;
    }

    return URJ_STATUS_OK;
}

/* ---------------------------------------------------------------------- */


static int
xpcu_read_cpld_version (struct libusb_device_handle *xpcu, uint16_t *buf)
{
    if (libusb_control_transfer
        (xpcu, 0xC0, 0xB0, 0x0050, 0x0001, (unsigned char *) buf, 2, 1000) < 0)
    {
        urj_error_IO_set ("libusb_control_transfer(0x50.1) (read_cpld_version)");
        return URJ_STATUS_FAIL;
    }
    return URJ_STATUS_OK;
}

/* ---------------------------------------------------------------------- */

static int
xpcu_read_firmware_version (struct libusb_device_handle *xpcu, uint16_t *buf)
{
    if (libusb_control_transfer
        (xpcu, 0xC0, 0xB0, 0x0050, 0x0000, (unsigned char *) buf, 2, 1000) < 0)
    {
        urj_error_IO_set ("libusb_control_transfer(0x50.0) (read_firmware_version)");
        return URJ_STATUS_FAIL;
    }

    return URJ_STATUS_OK;
}

/* ----------------------------------------------------------------- */

static int
xpcu_select_gpio (struct libusb_device_handle *xpcu, int int_or_ext)
{
    if (libusb_control_transfer (xpcu, 0x40, 0xB0, 0x0052, int_or_ext, NULL, 0, 1000)
        < 0)
    {
        urj_error_IO_set ("libusb_control_transfer(0x52.x) (select gpio)");
        return URJ_STATUS_FAIL;
    }

    return URJ_STATUS_OK;
}

/* ---------------------------------------------------------------------- */

/* === A6 transfer (TDI/TMS/TCK/RDO) ===
 *
 *   Vendor request 0xA6 initiates a quite universal shift operation. The data
 *   is passed directly to the CPLD as 16-bit words.
 *
 *   The argument N in the request specifies the number of state changes/bits.
 *
 *   State changes are described by the following bulk write. It consists
 *   of ceil(N/4) little-endian 16-bit words, each describing up to 4 changes:
 *
 *   Care has to be taken that N is NOT a multiple of 4.
 *   The CPLD doesn't seem to handle that well.
 *
 *   Bit 0: Value for first TDI to shift out.
 *   Bit 1: Second TDI.
 *   Bit 2: Third TDI.
 *   Bit 3: Fourth TDI.
 *
 *   Bit 4: Value for first TMS to shift out.
 *   Bit 5: Second TMS.
 *   Bit 6: Third TMS.
 *   Bit 7: Fourth TMS.
 *
 *   Bit 8: Whether to raise/lower TCK for first bit.
 *   Bit 9: Same for second bit.
 *   Bit 10: Third bit.
 *   Bit 11: Fourth bit.
 *
 *   Bit 12: Whether to read TDO for first bit
 *   Bit 13: Same for second bit.
 *   Bit 14: Third bit.
 *   Bit 15: Fourth bit.
 *
 *   After the bulk write, if any of the bits 12..15 was set in any word, a
 *   bulk_read shall follow to collect the TDO data.
 *
 *   TDO data is shifted in from MSB. In a "full" word with 16 TDO bits, the
 *   earliest one reached bit 0. The earliest of 15 bits however would be bit 0,
 *   and if there's only one TDO bit, it arrives as the MSB of the word.
 */

/** @return 0 on success; -1 on error */
static int
xpcu_shift (struct libusb_device_handle *xpcu, int reqno, int bits, int in_len,
            uint8_t *in, int out_len, uint8_t *out)
{
    int ret, actual;

    if (libusb_control_transfer (xpcu, 0x40, 0xB0, reqno, bits, NULL, 0, 1000) < 0)
    {
        urj_error_IO_set ("libusb_control_transfer(x.x) (shift)");
        return -1;
    }

#if VERBOSE
    {
        int i;
        urj_log (URJ_LOG_LEVEL_DETAIL, "###\n");
        urj_log (URJ_LOG_LEVEL_DETAIL, "reqno = %02X\n", reqno);
        urj_log (URJ_LOG_LEVEL_DETAIL, "bits    = %d\n", bits);
        urj_log (URJ_LOG_LEVEL_DETAIL, "in_len  = %d, in_len*2  = %d\n", in_len, in_len * 2);
        urj_log (URJ_LOG_LEVEL_DETAIL, "out_len = %d, out_len*8 = %d\n", out_len, out_len * 8);

        urj_log (URJ_LOG_LEVEL_DETAIL, "a6_display(\"%02X\", \"", bits);
        for (i = 0; i < in_len; i++)
            urj_log (URJ_LOG_LEVEL_DETAIL, "%02X%s", in[i], (i + 1 < in_len) ? "," : "");
        urj_log (URJ_LOG_LEVEL_DETAIL, "\", ");
    }
#endif

    ret = libusb_bulk_transfer (xpcu, 0x02, in, in_len, &actual, 1000);
    if (ret)
    {
        urj_error_IO_set ("usb_bulk_write error(shift): %i (transferred %i)",
                          ret, actual);
        return -1;
    }

    if (out_len > 0 && out != NULL)
    {
        ret = libusb_bulk_transfer (xpcu, 0x06 | LIBUSB_ENDPOINT_IN, out, out_len, &actual, 1000);
        if (ret)
        {
            urj_error_IO_set ("usb_bulk_read error(shift): %i (transferred %i)",
                              ret, actual);
            return -1;
        }
    }

#if VERBOSE
    {
        int i;
        urj_log (URJ_LOG_LEVEL_DETAIL, "\"");
        for (i = 0; i < out_len; i++)
            urj_log (URJ_LOG_LEVEL_DETAIL, "%02X%s", out[i], (i + 1 < out_len) ? "," : "");
        urj_log (URJ_LOG_LEVEL_DETAIL, "\")\n");
    }
#endif

    return 0;
}

/* ---------------------------------------------------------------------- */

static int
xpcu_common_init (urj_cable_t *cable)
{
    int r;
    uint16_t buf;
    struct libusb_device_handle *xpcu;

    if (urj_tap_usbconn_open (cable->link.usb) != URJ_STATUS_OK)
        return URJ_STATUS_FAIL;

    xpcu = ((urj_usbconn_libusb_param_t *) (cable->link.usb->params))->handle;

    r = xpcu_request_28 (xpcu, 0x11);
    if (r != URJ_STATUS_FAIL)
        r = xpcu_write_gpio (xpcu, 8);

    /* Read firmware version (constant embedded in firmware) */

    if (r != URJ_STATUS_FAIL)
        r = xpcu_read_firmware_version (xpcu, &buf);
    if (r != URJ_STATUS_FAIL)
    {
        urj_log (URJ_LOG_LEVEL_NORMAL,
                 "firmware version = 0x%04X (%u)\n", buf, buf);
    }

    /* Read CPLD version (via GPIF) */

    if (r != URJ_STATUS_FAIL)
        // @@@@ RFHH added assignment of result to r:
        r = xpcu_read_cpld_version (xpcu, &buf);
    if (r != URJ_STATUS_FAIL)
    {
        urj_log (URJ_LOG_LEVEL_NORMAL, "cable CPLD version = 0x%04X (%u)\n",
                 buf, buf);
        if (buf == 0)
        {
            urj_warning ("version '0' can't be correct. Please try resetting the cable\n");
            r = URJ_STATUS_FAIL;
        }
    }

    if (r != URJ_STATUS_OK)
    {
        libusb_close (xpcu);
    }

    return r;
}

static int
xpc_int_init (urj_cable_t *cable)
{
    struct libusb_device_handle *xpcu;

    if (xpcu_common_init (cable) == URJ_STATUS_FAIL)
        return URJ_STATUS_FAIL;

    xpcu = ((urj_usbconn_libusb_param_t *) (cable->link.usb->params))->handle;
    if (xpcu_select_gpio (xpcu, 0) == URJ_STATUS_FAIL)
        return URJ_STATUS_FAIL;

    return URJ_STATUS_OK;
}

static int
xpc_ext_init (urj_cable_t *cable)
{
    struct libusb_device_handle *xpcu;
    uint8_t zero[2] = { 0, 0 };
    int r;

    free (cable->params);
    cable->params = NULL;

    r = xpcu_common_init (cable);

    if (r == URJ_STATUS_FAIL)
        return r;

    cable->params = malloc (sizeof (xpc_cable_params_t));
    if (cable->params == NULL)
    {
        urj_error_set (URJ_ERROR_OUT_OF_MEMORY, "malloc(%zd) fails",
                       sizeof (xpc_cable_params_t));
        r = URJ_STATUS_FAIL;
    }

    xpcu = ((urj_usbconn_libusb_param_t *) (cable->link.usb->params))->handle;

    if (r == URJ_STATUS_OK)
        r = xpcu_output_enable (xpcu, 0);
    if (r == URJ_STATUS_OK)
        r = xpcu_request_28 (xpcu, 0x11);
    if (r == URJ_STATUS_OK)
        r = xpcu_output_enable (xpcu, 1);
    if (r == URJ_STATUS_OK)
        r = xpcu_shift (xpcu, 0xA6, 2, 2, zero, 0, NULL) == -1
            ? URJ_STATUS_FAIL : URJ_STATUS_OK;
    if (r == URJ_STATUS_OK)
        r = xpcu_request_28 (xpcu, 0x12);

    if (r != URJ_STATUS_OK)
    {
        libusb_close (xpcu);

        free (cable->params);
        cable->params = NULL;
    }

    return r;
}

/* ---------------------------------------------------------------------- */

static void
xpc_ext_free (urj_cable_t *cable)
{
    if (cable->params)
    {
        free (cable->params);
        cable->params = NULL;
    }
    urj_tap_cable_generic_usbconn_free (cable);
}

/* ---------------------------------------------------------------------- */

#define PROG    3
#define TCK     2
#define TMS     1
#define TDI     0
#define TDO     0

static void
xpc_clock (urj_cable_t *cable, int tms, int tdi, int n)
{
    int i;
    struct libusb_device_handle *xpcu;
    xpcu = ((urj_usbconn_libusb_param_t *) (cable->link.usb->params))->handle;

    tms = tms ? (1 << TMS) : 0;
    tdi = tdi ? (1 << TDI) : 0;

    if (xpcu_write_gpio (xpcu, (1 << PROG) | (0 << TCK) | tms | tdi)
        != URJ_STATUS_FAIL)
    {
        urj_tap_cable_wait (cable);
        for (i = 0; i < n; i++)
        {
            xpcu_write_gpio (xpcu, (1 << PROG) | (1 << TCK) | tms | tdi);
            urj_tap_cable_wait (cable);
            xpcu_write_gpio (xpcu, (1 << PROG) | (0 << TCK) | tms | tdi);
            urj_tap_cable_wait (cable);
        }
    }
}

/* ---------------------------------------------------------------------- */

static int
xpc_get_tdo (urj_cable_t *cable)
{
    unsigned char d;
    struct libusb_device_handle *xpcu;
    xpcu = ((urj_usbconn_libusb_param_t *) (cable->link.usb->params))->handle;

    xpcu_read_gpio (xpcu, &d);
    return (d & (1 << TDO)) ? 1 : 0;
}

/* ---------------------------------------------------------------------- */

static int
xpc_set_signal (urj_cable_t *cable, int mask, int val)
{
    return 1;
}

/* ---------------------------------------------------------------------- */

static void
xpc_ext_clock (urj_cable_t *cable, int tms, int tdi, int n)
{
    int i;
    uint8_t tdo[2];
    uint8_t clock[2];
    struct libusb_device_handle *xpcu;

    clock[0] = (tms ? 0x10 : 0) | (tdi ? 0x01 : 0);
    clock[1] = 0x11;            /* clock'n read */

    xpcu = ((urj_usbconn_libusb_param_t *) (cable->link.usb->params))->handle;

    for (i = 0; i < n; i++)
        xpcu_shift (xpcu, 0xA6, 1, 2, clock, 2, tdo);

    last_tdo = tdo[1] ? 1 : 0;
    // ((xpc_cable_params_t*)(cable->params))->last_tdo = tdo[1] ? 1:0;
}

/* ---------------------------------------------------------------------- */

static int
xpc_ext_get_tdo (urj_cable_t *cable)
{
    return last_tdo;
    // return ((xpc_cable_params_t*)(cable->params))->last_tdo;
}

/* ---------------------------------------------------------------------- */

/* 16-bit words. More than 4 currently leads to bit errors; 13 to serious problems */
#define XPC_A6_CHUNKSIZE 4

typedef struct
{
    urj_cable_t *cable;
    struct libusb_device_handle *xpcu;
    int in_bits;
    int out_bits;
    int out_done;
    uint8_t *out;
    uint8_t buf[XPC_A6_CHUNKSIZE * 2];
}
xpc_ext_transfer_state_t;

/* ---------------------------------------------------------------------- */

/** @return 0 on success; -1 on error */
static int
xpcu_do_ext_transfer (xpc_ext_transfer_state_t *xts)
{
    int r;
    int in_len, out_len;
    // int last_tdo;

    in_len = 2 * (xts->in_bits >> 2);
    if ((xts->in_bits & 3) != 0)
        in_len += 2;

    out_len = 2 * (xts->out_bits >> 4);
    if ((xts->out_bits & 15) != 0)
        out_len += 2;

    if (xts->out != NULL)
    {
        r = xpcu_shift (xts->xpcu, 0xA6, xts->in_bits, in_len, xts->buf,
                        out_len, xts->buf);
    }
    else
    {
        r = xpcu_shift (xts->xpcu, 0xA6, xts->in_bits, in_len, xts->buf, 0,
                        NULL);
    }

    if (r != -1 && xts->out_bits > 0)
    {
        int out_idx = 0;
        int out_rem = xts->out_bits;

        while (out_rem > 0)
        {
            uint32_t mask, rxw;

            rxw = (xts->buf[out_idx + 1] << 8) | xts->buf[out_idx];

            /* In the last (incomplete) word, the data isn't shifted completely to LSB */

            mask = (out_rem >= 16) ? 1 : (1 << (16 - out_rem));

            while (mask <= 32768 && out_rem > 0)
            {
                last_tdo = (rxw & mask) ? 1 : 0;
                xts->out[xts->out_done] = last_tdo;
                xts->out_done++;
                mask <<= 1;
                out_rem--;
            }

            out_idx += 2;
        }
    }

    xts->in_bits = 0;
    xts->out_bits = 0;

    // ((xpc_cable_params_t*)(xts->cable->params))->last_tdo = last_tdo;

    return r;
}

/* ---------------------------------------------------------------------- */

static void
xpcu_add_bit_for_ext_transfer (xpc_ext_transfer_state_t *xts, char in,
                               char is_real)
{
    int bit_idx = (xts->in_bits & 3);
    int buf_idx = (xts->in_bits - bit_idx) >> 1;

    if (bit_idx == 0)
    {
        xts->buf[buf_idx] = 0;
        xts->buf[buf_idx + 1] = 0;
    }

    xts->in_bits++;

    if (is_real)
    {
        if (in)
            xts->buf[buf_idx] |= (0x01 << bit_idx);

        if (xts->out)
        {
            xts->buf[buf_idx + 1] |= (0x11 << bit_idx);
            xts->out_bits++;
        }
        else
        {
            xts->buf[buf_idx + 1] |= (0x01 << bit_idx);
        }
    }
}

/* ---------------------------------------------------------------------- */

// @@@@ RFHH the specx say that it should be
//      @return: num clocks on success, -1 on error.
//              Might have to be: return i;

/** @return 0 on success; -1 on error */
static int
xpc_ext_transfer (urj_cable_t *cable, int len, const char *in, char *out)
{
    int i, j;
    xpc_ext_transfer_state_t xts;

#if VERBOSE
    urj_log (URJ_LOG_LEVEL_DETAIL, "---\n");
    urj_log (URJ_LOG_LEVEL_DETAIL, "transfer size %d, %s output\n", len,
            (out != NULL) ? "with" : "without");
    urj_log (URJ_LOG_LEVEL_DETAIL, "tdi: ");
    for (i = 0; i < len; i++)
        urj_log (URJ_LOG_LEVEL_DETAIL, "%c", in[i] ? '1' : '0');
    urj_log (URJ_LOG_LEVEL_DETAIL, "\n");
#endif

    xts.xpcu =
        ((urj_usbconn_libusb_param_t *) (cable->link.usb->params))->handle;
    xts.out = (uint8_t *) out;
    xts.in_bits = 0;
    xts.out_bits = 0;
    xts.out_done = 0;
    xts.cable = cable;

    for (i = 0, j = 0; i < len && j >= 0; i++)
    {
        xpcu_add_bit_for_ext_transfer (&xts, in[i], 1);
        if (xts.in_bits == (4 * XPC_A6_CHUNKSIZE - 1))
        {
            j = xpcu_do_ext_transfer (&xts);
        }
    }

    if (xts.in_bits > 0 && j >= 0)
    {
        /* CPLD doesn't like multiples of 4; add one dummy bit */
        if ((xts.in_bits & 3) == 0)
        {
            xpcu_add_bit_for_ext_transfer (&xts, 0, 0);
        }
        j = xpcu_do_ext_transfer (&xts);
    }

    return j;
}

/* ---------------------------------------------------------------------- */


const urj_cable_driver_t urj_tap_cable_xpc_int_driver = {
    "xpc_int",
    N_("Xilinx Platform Cable USB internal chain"),
    URJ_CABLE_DEVICE_USB,
    { .usb = urj_tap_cable_generic_usbconn_connect, },
    urj_tap_cable_generic_disconnect,
    urj_tap_cable_generic_usbconn_free,
    xpc_int_init,
    urj_tap_cable_generic_usbconn_done,
    urj_tap_cable_generic_set_frequency,
    xpc_clock,
    xpc_get_tdo,
    urj_tap_cable_generic_transfer,
    xpc_set_signal,
    urj_tap_cable_generic_get_signal,
    urj_tap_cable_generic_flush_using_transfer,
    urj_tap_cable_generic_usbconn_help
};
URJ_DECLARE_USBCONN_CABLE(0x03FD, 0x0008, "libusb", "xpc_int", xpc_int)

const urj_cable_driver_t urj_tap_cable_xpc_ext_driver = {
    "xpc_ext",
    N_("Xilinx Platform Cable USB external chain"),
    URJ_CABLE_DEVICE_USB,
    { .usb = urj_tap_cable_generic_usbconn_connect, },
    urj_tap_cable_generic_disconnect,
    xpc_ext_free,
    xpc_ext_init,
    urj_tap_cable_generic_usbconn_done,
    urj_tap_cable_generic_set_frequency,
    xpc_ext_clock,
    xpc_ext_get_tdo,
    xpc_ext_transfer,
    xpc_set_signal,
    urj_tap_cable_generic_get_signal,
    urj_tap_cable_generic_flush_using_transfer,
    urj_tap_cable_generic_usbconn_help
};
URJ_DECLARE_USBCONN_CABLE(0x03FD, 0x0008, "libusb", "xpc_ext", xpc_ext)
