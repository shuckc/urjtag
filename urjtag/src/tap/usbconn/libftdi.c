/*
 * $Id$
 *
 * Link driver for accessing FTDI devices via libftdi
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
 * Written by Arnim Laeuger, 2008
 *
 */

#include <urjtag/sysdep.h>

#include <fcntl.h>
#if __CYGWIN__ || __MINGW32__
#include <windows.h>
#endif
#include <stdio.h>
#include <string.h>
#ifdef HAVE_STROPTS_H
#include <stropts.h>
#endif
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

#include <ftdi.h>

#include <urjtag/error.h>
#include <urjtag/log.h>
#include <urjtag/usbconn.h>
#include <urjtag/usbconn/libftdx.h>

typedef struct
{
    /* USB device information */
    unsigned int vid;
    unsigned int pid;
    struct ftdi_context *fc;
    char *serial;
    /* send and receive buffer handling */
    uint32_t send_buf_len;
    uint32_t send_buffered;
    uint8_t *send_buf;
    uint32_t recv_buf_len;
    uint32_t to_recv;
    uint32_t recv_write_idx;
    uint32_t recv_read_idx;
    uint8_t *recv_buf;
} ftdi_param_t;

urj_usbconn_driver_t urj_tap_usbconn_ftdi_driver;
urj_usbconn_driver_t urj_tap_usbconn_ftdi_mpsse_driver;

static int usbconn_ftdi_common_open (urj_usbconn_t *conn, urj_log_level_t ll);
static void usbconn_ftdi_free (urj_usbconn_t *conn);

/* ---------------------------------------------------------------------- */

/** @return number of bytes flushed; -1 on error */
static int
usbconn_ftdi_flush (ftdi_param_t *p)
{
    int xferred;
    int recvd = 0;

    if (!p->fc)
        return -1;

    if (p->send_buffered == 0)
        return 0;

    if ((xferred = ftdi_write_data (p->fc, p->send_buf, p->send_buffered)) < 0)
        urj_error_set (URJ_ERROR_IO, _("ftdi_write_data() failed: %s"),
                       ftdi_get_error_string (p->fc));

    if (xferred < p->send_buffered)
    {
        urj_error_set (URJ_ERROR_ILLEGAL_STATE,
                       _("Written fewer bytes than requested"));
        return -1;
    }

    p->send_buffered = 0;

    /* now read all scheduled receive bytes */
    if (p->to_recv)
    {
        if (p->recv_write_idx + p->to_recv > p->recv_buf_len)
        {
            /* extend receive buffer */
            p->recv_buf_len = p->recv_write_idx + p->to_recv;
            if (p->recv_buf)
                p->recv_buf = realloc (p->recv_buf, p->recv_buf_len);
        }

        if (!p->recv_buf)
        {
            urj_error_set (URJ_ERROR_ILLEGAL_STATE,
                           _("Receive buffer does not exist"));
            return -1;
        }

        while (recvd == 0)
            if ((recvd = ftdi_read_data (p->fc,
                                         &(p->recv_buf[p->recv_write_idx]),
                                         p->to_recv)) < 0)
                urj_error_set (URJ_ERROR_IO,
                               _("Error from ftdi_read_data(): %s"),
                               ftdi_get_error_string (p->fc));

        if (recvd < p->to_recv)
            urj_log (URJ_LOG_LEVEL_NORMAL,
                     _("%s(): Received fewer bytes than requested.\n"),
                     __func__);

        p->to_recv -= recvd;
        p->recv_write_idx += recvd;
    }

    return xferred < 0 ? -1 : xferred;
}

/* ---------------------------------------------------------------------- */

/** @return number of bytes read; -1 on error */
static int
usbconn_ftdi_read (urj_usbconn_t *conn, uint8_t *buf, int len)
{
    ftdi_param_t *p = conn->params;
    int cpy_len;
    int recvd = 0;

    if (!p->fc)
        return -1;

    /* flush send buffer to get all scheduled receive bytes */
    if (usbconn_ftdi_flush (p) < 0)
        return -1;

    if (len == 0)
        return 0;

    /* check for number of remaining bytes in receive buffer */
    cpy_len = p->recv_write_idx - p->recv_read_idx;
    if (cpy_len > len)
        cpy_len = len;
    len -= cpy_len;

    if (cpy_len > 0)
    {
        /* get data from the receive buffer */
        memcpy (buf, &(p->recv_buf[p->recv_read_idx]), cpy_len);
        p->recv_read_idx += cpy_len;
        if (p->recv_read_idx == p->recv_write_idx)
            p->recv_read_idx = p->recv_write_idx = 0;
    }

    if (len > 0)
    {
        /* need to get more data directly from the device */
        while (recvd == 0)
            if ((recvd = ftdi_read_data (p->fc, &(buf[cpy_len]), len)) < 0)
                urj_error_set (URJ_ERROR_IO,
                               _("Error from ftdi_read_data(): %s"),
                               ftdi_get_error_string (p->fc));
    }

    return recvd < 0 ? -1 : cpy_len + len;
}

/* ---------------------------------------------------------------------- */

/** @return number of bytes written; -1 on error */
static int
usbconn_ftdi_write (urj_usbconn_t *conn, uint8_t *buf, int len, int recv)
{
    ftdi_param_t *p = conn->params;
    int xferred = 0;

    if (!p->fc)
        return -1;

    /* this write function will try to buffer write data
       buffering will be ceased and a flush triggered in two cases. */

    /* Case A: max number of scheduled receive bytes will be exceeded
       with this write
       Case B: max number of scheduled send bytes has been reached */
    if ((p->to_recv + recv > URJ_USBCONN_FTDI_MAXRECV)
        || ((p->send_buffered > URJ_USBCONN_FTDX_MAXSEND)
            && (p->to_recv == 0)))
        xferred = usbconn_ftdi_flush (p);

    if (xferred < 0)
        return -1;

    /* now buffer this write */
    if (p->send_buffered + len > p->send_buf_len)
    {
        p->send_buf_len = p->send_buffered + len;
        if (p->send_buf)
            p->send_buf = realloc (p->send_buf, p->send_buf_len);
    }

    if (p->send_buf)
    {
        memcpy (&(p->send_buf[p->send_buffered]), buf, len);
        p->send_buffered += len;
        if (recv > 0)
            p->to_recv += recv;

        if (recv < 0)
        {
            /* immediate write requested, so flush the buffered data */
            xferred = usbconn_ftdi_flush (p);
        }

        return xferred < 0 ? -1 : len;
    }
    else
    {
        urj_error_set (URJ_ERROR_ILLEGAL_STATE,
                       _("Send buffer does not exist"));
        return -1;
    }
}

/* ---------------------------------------------------------------------- */

static urj_usbconn_t *
usbconn_ftdi_connect (const char **param, int paramc,
                      urj_usbconn_cable_t *template)
{
    urj_usbconn_t *c = malloc (sizeof (urj_usbconn_t));
    ftdi_param_t *p = malloc (sizeof (ftdi_param_t));
    struct ftdi_context *fc = malloc (sizeof (struct ftdi_context));

    if (p)
    {
        p->send_buf_len = URJ_USBCONN_FTDX_MAXSEND;
        p->send_buffered = 0;
        p->send_buf = malloc (p->send_buf_len);
        p->recv_buf_len = URJ_USBCONN_FTDI_MAXRECV;
        p->to_recv = 0;
        p->recv_write_idx = 0;
        p->recv_read_idx = 0;
        p->recv_buf = malloc (p->recv_buf_len);
    }

    if (!p || !c || !fc || !p->send_buf || !p->recv_buf)
    {
        if (p->send_buf)
            free (p->send_buf);
        if (p->recv_buf)
            free (p->recv_buf);
        if (p)
            free (p);
        if (c)
            free (c);
        if (fc)
            free (fc);
        urj_error_set (URJ_ERROR_OUT_OF_MEMORY,
                       "malloc(%zd)/malloc(%zd)/malloc(%zd)/malloc(%s)/malloc(%s) failed",
                       sizeof (urj_usbconn_t), sizeof (ftdi_param_t),
                       sizeof (struct ftdi_context),
                       "p->send_buf_len", "p->recv_buf_len");
        return NULL;
    }

    ftdi_init (fc);
    p->fc = fc;
    p->pid = template->pid;
    p->vid = template->vid;
    p->serial = template->desc ? strdup (template->desc) : NULL;

    c->params = p;
    c->driver = &urj_tap_usbconn_ftdi_driver;
    c->cable = NULL;

    /* do a test open with the specified cable paramters,
       alternatively we could use libusb to detect the presence of the
       specified USB device */
    if (usbconn_ftdi_common_open (c, URJ_LOG_LEVEL_DETAIL) != 0)
    {
        usbconn_ftdi_free (c);
        return NULL;
    }
    ftdi_usb_close (fc);

    urj_log (URJ_LOG_LEVEL_NORMAL, _("Connected to libftdi driver.\n"));

    return c;
}


static urj_usbconn_t *
usbconn_ftdi_mpsse_connect (const char **param, int paramc,
                            urj_usbconn_cable_t *template)
{
    urj_usbconn_t *conn = usbconn_ftdi_connect (param, paramc, template);

    if (conn)
        conn->driver = &urj_tap_usbconn_ftdi_mpsse_driver;

    return conn;
}


/* ---------------------------------------------------------------------- */

static int
usbconn_ftdi_common_open (urj_usbconn_t *conn, urj_log_level_t ll)
{
    ftdi_param_t *p = conn->params;
    struct ftdi_context *fc = p->fc;
    int status;

    /* use command line string for desc= as serial number and try to
       open a matching device */
    status = ftdi_usb_open_desc (fc, p->vid, p->pid, NULL, p->serial);
    if (status < 0)
        /* try again with matching the string against the description */
        status = ftdi_usb_open_desc (fc, p->vid, p->pid, p->serial, NULL);

    if (status < 0)
    {
        /* device not found == -3 */
        if (status != -3)
            urj_log (ll, _("%s(): ftdi_usb_open_desc() failed: %s"),
                     __func__, ftdi_get_error_string (fc));
        urj_error_set (URJ_ERROR_IO, _("ftdi_usb_open_desc() failed: %s"),
                       ftdi_get_error_string (fc));
        ftdi_deinit (fc);
        /* mark ftdi layer as not initialized */
        p->fc = NULL;

        /* TODO: disconnect? */
        return URJ_STATUS_FAIL;
    }

    return URJ_STATUS_OK;
}

/* ---------------------------------------------------------------------- */

#undef LIBFTDI_UNIMPLEMENTED

static int
seq_purge (struct ftdi_context *fc, int purge_rx, int purge_tx)
{
    int r;
    unsigned char buf;

#ifndef LIBFTDI_UNIMPLEMENTED
    if ((r = ftdi_usb_purge_buffers (fc)) < 0)
        urj_error_set (URJ_ERROR_IO, _("ftdi_usb_purge_buffers() failed: %s"),
                       ftdi_get_error_string (fc));
    if (r >= 0)
        if ((r = ftdi_read_data (fc, &buf, 1)) < 0)
            urj_error_set (URJ_ERROR_IO, _("ftdi_read_data() failed: %s"),
                           ftdi_get_error_string (fc));
#else /* not yet available */
    {
        int rx_loop;

        if (purge_rx)
            for (rx_loop = 0; (rx_loop < 6) && (r >= 0); rx_loop++)
                if ((r = ftdi_usb_purge_rx_buffer (fc)) < 0)
                    urj_error_set (URJ_ERROR_IO,
                                   _("ftdi_usb_purge_rx_buffer() failed: %s"),
                                   ftdi_get_error_string (fc));

        if (purge_tx)
            if (r >= 0)
                if ((r = ftdi_usb_purge_tx_buffer (fc)) < 0)
                    urj_error_set (URJ_ERROR_IO,
                                   _("ftdi_usb_purge_tx_buffer() failed: %s"),
                                   ftdi_get_error_string (fc));
        if (r >= 0)
            if ((r = ftdi_read_data (fc, &buf, 1)) < 0)
                urj_error_set (URJ_ERROR_IO, _("ftdi_read_data() failed: %s"),
                               ftdi_get_error_string (fc));
    }
#endif

    return r < 0 ? URJ_STATUS_FAIL : URJ_STATUS_OK;
}

static int
seq_reset (struct ftdi_context *fc)
{
    int r;

#ifdef LIBFTDI_UNIMPLEMENTED    /* not yet available */
    {
        unsigned short status;
        if ((r = ftdi_poll_status (fc, &status)) < 0)
            urj_error_set (URJ_ERROR_IO, _("ftdi_poll_status() failed: %s"),
                           ftdi_get_error_string (fc));
    }
#endif
    if ((r = ftdi_usb_reset (fc)) < 0)
        urj_error_set (URJ_ERROR_IO, _("ftdi_usb_reset() failed: %s"),
                       ftdi_get_error_string (fc));

    if (r >= 0)
        r = seq_purge (fc, 1, 1);
    return r < 0 ? URJ_STATUS_FAIL : URJ_STATUS_OK;
}

/* ---------------------------------------------------------------------- */

static int
usbconn_ftdi_open (urj_usbconn_t *conn)
{
    ftdi_param_t *p = conn->params;
    struct ftdi_context *fc = p->fc;
    int r;

    if (usbconn_ftdi_common_open (conn, URJ_LOG_LEVEL_NORMAL) < 0)
        return URJ_STATUS_FAIL;

    r = seq_reset (fc);
    if (r >= 0)
        r = seq_purge (fc, 1, 0);

    if (r >= 0)
        if ((r = ftdi_set_latency_timer (fc, 2)) < 0)
            urj_error_set (URJ_ERROR_IO,
                           _("ftdi_set_latency_timer() failed: %s"),
                           ftdi_get_error_string (fc));

#if 0
    /* libftdi 0.6 doesn't allow high baudrates, so we send the control
       message outselves */
    if (r >= 0)
        if (usb_control_msg
            (fc->usb_dev, 0x40, 3, 1, 0, NULL, 0, fc->usb_write_timeout) != 0)
        {
            urj_error_set (URJ_ERROR_IO, "Can't set max baud rate");
            r = -1;
        }
#else
    if (r >= 0)
        if ((r = ftdi_set_baudrate (fc, 3E6)) < 0)
            urj_error_set (URJ_ERROR_IO, _("ftdi_set_baudrate() failed: %s"),
                           ftdi_get_error_string (fc));
#endif

    if (r < 0)
    {
        ftdi_usb_close (fc);
        ftdi_deinit (fc);
        /* mark ftdi layer as not initialized */
        p->fc = NULL;
    }

    return r < 0 ? URJ_STATUS_FAIL : URJ_STATUS_OK;
}

/* ---------------------------------------------------------------------- */

static int
usbconn_ftdi_mpsse_open (urj_usbconn_t *conn)
{
    ftdi_param_t *p = conn->params;
    struct ftdi_context *fc = p->fc;
    int r;

    if (usbconn_ftdi_common_open (conn, URJ_LOG_LEVEL_NORMAL) < 0)
        return URJ_STATUS_FAIL;

    /* This sequence might seem weird and containing superfluous stuff.
       However, it's built after the description of JTAG_InitDevice
       Ref. FTCJTAGPG10.pdf
       Intermittent problems will occur when certain steps are skipped. */
    r = seq_reset (fc);
    if (r >= 0)
        r = seq_purge (fc, 1, 0);

    if (r >= 0)
        if ((r =
             ftdi_write_data_set_chunksize (fc,
                                            URJ_USBCONN_FTDX_MAXSEND_MPSSE)) <
            0)
            puts (ftdi_get_error_string (fc));
    if (r >= 0)
        if ((r =
             ftdi_read_data_set_chunksize (fc,
                                           URJ_USBCONN_FTDX_MAXSEND_MPSSE)) <
            0)
            puts (ftdi_get_error_string (fc));

#ifdef LIBFTDI_UNIMPLEMENTED
    if (r >= 0)
        if ((r = ftdi_set_event_char (fc, 0, 0)) < 0)
            puts (ftdi_get_error_string (fc));
    if (r >= 0)
        if ((r = ftdi_set_error_char (fc, 0, 0)) < 0)
            puts (ftdi_get_error_string (fc));
#endif

    /* set a reasonable latency timer value
       if this value is too low then the chip will send intermediate result data
       in short packets (suboptimal performance) */
    if (r >= 0)
        if ((r = ftdi_set_latency_timer (fc, 16)) < 0)
            urj_error_set (URJ_ERROR_IO,
                           _("ftdi_set_latency_timer() failed: %s"),
                           ftdi_get_error_string (fc));

    if (r >= 0)
        if ((r = ftdi_set_bitmode (fc, 0x0b, BITMODE_MPSSE)) < 0)
            urj_error_set (URJ_ERROR_IO, _("ftdi_set_bitmode() failed: %s"),
                           ftdi_get_error_string (fc));

    if (r >= 0)
        if ((r = ftdi_usb_reset (fc)) < 0)
            urj_error_set (URJ_ERROR_IO, _("ftdi_usb_reset() failed: %s"),
                           ftdi_get_error_string (fc));
    if (r >= 0)
        r = seq_purge (fc, 1, 0);

    /* set TCK Divisor */
    if (r >= 0)
    {
        uint8_t buf[3] = { TCK_DIVISOR, 0x00, 0x00 };
        r = usbconn_ftdi_write (conn, buf, 3, 0);
    }
    /* switch off loopback */
    if (r >= 0)
    {
        uint8_t buf[1] = { LOOPBACK_END };
        r = usbconn_ftdi_write (conn, buf, 1, 0);
    }
    if (r >= 0)
        r = usbconn_ftdi_read (conn, NULL, 0);

    if (r >= 0)
        if ((r = ftdi_usb_reset (fc)) < 0)
            urj_error_set (URJ_ERROR_IO, _("ftdi_usb_reset() failed: %s"),
                           ftdi_get_error_string (fc));
    if (r >= 0)
        r = seq_purge (fc, 1, 0);

    if (r < 0)
    {
        ftdi_usb_close (fc);
        ftdi_deinit (fc);
        /* mark ftdi layer as not initialized */
        p->fc = NULL;
    }

    return r < 0 ? URJ_STATUS_FAIL : URJ_STATUS_OK;
}

/* ---------------------------------------------------------------------- */

static int
usbconn_ftdi_close (urj_usbconn_t *conn)
{
    ftdi_param_t *p = conn->params;

    if (p->fc)
    {
        ftdi_usb_close (p->fc);
        ftdi_deinit (p->fc);
        p->fc = NULL;
    }

    return URJ_STATUS_OK;
}

/* ---------------------------------------------------------------------- */

static void
usbconn_ftdi_free (urj_usbconn_t *conn)
{
    ftdi_param_t *p = conn->params;

    if (p->send_buf)
        free (p->send_buf);
    if (p->recv_buf)
        free (p->recv_buf);
    if (p->fc)
        free (p->fc);
    if (p->serial)
        free (p->serial);

    free (conn->params);
    free (conn);
}

/* ---------------------------------------------------------------------- */

urj_usbconn_driver_t urj_tap_usbconn_ftdi_driver = {
    "ftdi",
    usbconn_ftdi_connect,
    usbconn_ftdi_free,
    usbconn_ftdi_open,
    usbconn_ftdi_close,
    usbconn_ftdi_read,
    usbconn_ftdi_write
};

urj_usbconn_driver_t urj_tap_usbconn_ftdi_mpsse_driver = {
    "ftdi-mpsse",
    usbconn_ftdi_mpsse_connect,
    usbconn_ftdi_free,
    usbconn_ftdi_mpsse_open,
    usbconn_ftdi_close,
    usbconn_ftdi_read,
    usbconn_ftdi_write
};


/*
 Local Variables:
 mode:C
 c-default-style:gnu
 indent-tabs-mode:nil
 End:
*/
