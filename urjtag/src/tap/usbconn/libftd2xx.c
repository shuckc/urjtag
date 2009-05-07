/*
 * $Id$
 *
 * Link driver for accessing FTDI devices via libftd2xx
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

#include <ftd2xx.h>

#include <urjtag/error.h>
#include <urjtag/log.h>
#include <urjtag/usbconn.h>
#include <urjtag/usbconn/libftdx.h>


/* enables debug output */
static const char *module = "usbconn_ftd2xx_";


typedef struct
{
    /* USB device information */
    unsigned int vid;
    unsigned int pid;
    FT_HANDLE fc;
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
} ftd2xx_param_t;

urj_usbconn_driver_t urj_tap_usbconn_ftd2xx_driver;
urj_usbconn_driver_t urj_tap_usbconn_ftd2xx_mpsse_driver;

static int usbconn_ftd2xx_common_open (urj_usbconn_t *conn, urj_log_level_t ll);
static void usbconn_ftd2xx_free (urj_usbconn_t *conn);

/* ---------------------------------------------------------------------- */

/** @return number of flushed bytes on success; -1 on error */
static int
usbconn_ftd2xx_flush (ftd2xx_param_t *p)
{
    FT_STATUS status;
    DWORD xferred;
    DWORD recvd = 0;

    if (!p->fc)
        return -1;

    urj_log (URJ_LOG_LEVEL_DETAIL, "%sflush begin:\n", module);
    urj_log (URJ_LOG_LEVEL_DETAIL, "  send_buf_len %d, send_buffered %d\n",
             p->send_buf_len, p->send_buffered);
    urj_log (URJ_LOG_LEVEL_DETAIL, "  recv_buf_len %d, to_recv %d\n",
             p->recv_buf_len, p->to_recv);
    urj_log (URJ_LOG_LEVEL_DETAIL, "  recv_write_idx %d, recv_read_idx %d\n",
             p->recv_write_idx, p->recv_read_idx);

    if (p->send_buffered == 0)
        return 0;

    if ((status = FT_Write (p->fc, p->send_buf, p->send_buffered,
                            &xferred)) != FT_OK)
        urj_error_set (URJ_ERROR_IO, _("FT_Write() failed"));

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
            if ((status = FT_Read (p->fc, &(p->recv_buf[p->recv_write_idx]),
                                   p->to_recv, &recvd)) != FT_OK)
                urj_error_set (URJ_ERROR_IO, _("Error from FT_Read(): %d"),
                               (int) status);

        if (recvd < p->to_recv)
            urj_log (URJ_LOG_LEVEL_NORMAL,
                     _("%s(): Received fewer bytes than requested.\n"),
                    __func__);

        p->to_recv -= recvd;
        p->recv_write_idx += recvd;
    }

    urj_log (URJ_LOG_LEVEL_DETAIL,
             "%sflush end: status %ld, xferred %ld, recvd %ld\n", module,
            status, xferred, recvd);

    return status != FT_OK ? -1 : xferred;
}

/* ---------------------------------------------------------------------- */

static int
usbconn_ftd2xx_read (urj_usbconn_t *conn, uint8_t *buf, int len)
{
    ftd2xx_param_t *p = conn->params;
    int cpy_len;
    FT_STATUS status = FT_OK;
    DWORD recvd = 0;

    urj_log (URJ_LOG_LEVEL_DETAIL, "%sread begin: len %d\n", module, len);

    if (!p->fc)
        return -1;

    /* flush send buffer to get all scheduled receive bytes */
    if (usbconn_ftd2xx_flush (p) < 0)
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
            if ((status =
                 FT_Read (p->fc, &(buf[cpy_len]), len, &recvd)) != FT_OK)
                urj_error_set (URJ_ERROR_IO, _("Error from FT_Read(): %d"),
                               (int) status);
    }

    urj_log (URJ_LOG_LEVEL_DETAIL, "%sread end  : status %ld, length %d\n",
             module, status, cpy_len + len);

    return status != FT_OK ? -1 : cpy_len + len;
}

/* ---------------------------------------------------------------------- */

static int
usbconn_ftd2xx_write (urj_usbconn_t *conn, uint8_t *buf, int len, int recv)
{
    ftd2xx_param_t *p = conn->params;
    int xferred = 0;

    if (!p->fc)
        return -1;

    urj_log (URJ_LOG_LEVEL_DETAIL, "%swrite begin: len %d, recv %d\n", module,
             len, recv);

    /* this write function will try to buffer write data
       buffering will be ceased and a flush triggered in two cases. */

    /* Case A: max number of scheduled receive bytes will be exceeded
       with this write
       Case B: max number of scheduled send bytes has been reached */
    if ((p->to_recv + recv > URJ_USBCONN_FTD2XX_MAXRECV)
        || ((p->send_buffered > URJ_USBCONN_FTDX_MAXSEND)
            && (p->to_recv == 0)))
        xferred = usbconn_ftd2xx_flush (p);

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
            xferred = usbconn_ftd2xx_flush (p);
        }

        urj_log (URJ_LOG_LEVEL_DETAIL, "%swrite end: xferred %d\n", module,
                 xferred);

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
usbconn_ftd2xx_connect (const char **param, int paramc,
                        urj_usbconn_cable_t *template)
{
    urj_usbconn_t *c = malloc (sizeof (urj_usbconn_t));
    ftd2xx_param_t *p = malloc (sizeof (ftd2xx_param_t));

    if (p)
    {
        p->send_buf_len = URJ_USBCONN_FTDX_MAXSEND;
        p->send_buffered = 0;
        p->send_buf = malloc (p->send_buf_len);
        p->recv_buf_len = URJ_USBCONN_FTD2XX_MAXRECV;
        p->to_recv = 0;
        p->recv_write_idx = 0;
        p->recv_read_idx = 0;
        p->recv_buf = malloc (p->recv_buf_len);
    }

    if (!p || !c || !p->send_buf || !p->recv_buf)
    {
        if (p->send_buf)
            free (p->send_buf);
        if (p->recv_buf)
            free (p->recv_buf);
        if (p)
            free (p);
        if (c)
            free (c);
        urj_error_set (URJ_ERROR_OUT_OF_MEMORY,
                       "malloc(%zd)/malloc(%zd)/malloc(%s) failed",
                       sizeof (urj_usbconn_t), sizeof (ftd2xx_param_t),
                       "p->send_buf_len");
        return NULL;
    }

    p->fc = NULL;
    p->pid = template->pid;
    p->vid = template->vid;
    p->serial = template->desc ? strdup (template->desc) : NULL;

    c->params = p;
    c->driver = &urj_tap_usbconn_ftd2xx_driver;
    c->cable = NULL;

    /* do a test open with the specified cable paramters,
       there's no other way to detect the presence of the specified
       USB device */
    if (usbconn_ftd2xx_common_open (c, URJ_LOG_LEVEL_DETAIL) != URJ_STATUS_OK)
    {
        usbconn_ftd2xx_free (c);
        return NULL;
    }
    FT_Close (p->fc);

    urj_log (URJ_LOG_LEVEL_NORMAL, _("Connected to libftd2xx driver.\n"));

    return c;
}


static urj_usbconn_t *
usbconn_ftd2xx_mpsse_connect (const char **param, int paramc,
                              urj_usbconn_cable_t *template)
{
    urj_usbconn_t *conn = usbconn_ftd2xx_connect (param, paramc, template);

    if (conn)
        conn->driver = &urj_tap_usbconn_ftd2xx_mpsse_driver;

    return conn;
}


/* ---------------------------------------------------------------------- */

static int
usbconn_ftd2xx_common_open (urj_usbconn_t *conn, urj_log_level_t ll)
{
    ftd2xx_param_t *p = conn->params;
    FT_STATUS status;

#if !__CYGWIN__ && !__MINGW32__
    /* Add non-standard Vid/Pid to the linux driver */
    if ((status = FT_SetVIDPID (p->vid, p->pid)) != FT_OK)
        urj_warning ("couldn't add %4.4x:%4.4x", p->vid, p->pid);
#endif

    /* try various methods to open a FTDI device */
    if (p->serial)
    {
        /* serial number/description is specified */

        /* first try to match against the serial string */
        status = FT_OpenEx (p->serial, FT_OPEN_BY_SERIAL_NUMBER, &(p->fc));

        if (status != FT_OK)
            /* then try to match against the description string */
            status = FT_OpenEx (p->serial, FT_OPEN_BY_DESCRIPTION, &(p->fc));
    }
    else
        /* give it a plain try */
        status = FT_Open (0, &(p->fc));

    if (status != FT_OK)
    {
        urj_error_set (URJ_ERROR_IO, "Unable to open TFDI device");
        urj_log (ll, "Unable to open FTDI device.\n");
        /* mark ftd2xx layer as not initialized */
        p->fc = NULL;
        return URJ_STATUS_FAIL;
    }

    return URJ_STATUS_OK;
}

/* ---------------------------------------------------------------------- */

static int
usbconn_ftd2xx_open (urj_usbconn_t *conn)
{
    ftd2xx_param_t *p = conn->params;
    FT_HANDLE fc;
    FT_STATUS status;

    if (usbconn_ftd2xx_common_open (conn, URJ_LOG_LEVEL_NORMAL)
        != URJ_STATUS_OK)
        return URJ_STATUS_FAIL;

    fc = p->fc;

    if ((status = FT_ResetDevice (fc)) != FT_OK)
        urj_error_set (URJ_ERROR_IO, _("Can't reset device"));
    if (status == FT_OK)
        if ((status = FT_Purge (fc, FT_PURGE_RX)) != FT_OK)
            urj_error_set (URJ_ERROR_IO, _("Can't purge RX buffer"));

    if (status == FT_OK)
        if ((status = FT_SetLatencyTimer (fc, 2)) != FT_OK)
            urj_error_set (URJ_ERROR_IO, _("Can't set latency timer"));

    if (status == FT_OK)
        if ((status = FT_SetBaudRate (fc, 3E6)) != FT_OK)
            urj_error_set (URJ_ERROR_IO, _("Can't set baudrate"));

    if (status != FT_OK)
    {
        FT_Close (fc);
        /* mark ftdi layer as not initialized */
        p->fc = NULL;
    }

    return status != FT_OK ? URJ_STATUS_FAIL : URJ_STATUS_OK;
}

/* ---------------------------------------------------------------------- */

static int
usbconn_ftd2xx_mpsse_open (urj_usbconn_t *conn)
{
    ftd2xx_param_t *p = conn->params;
    FT_HANDLE fc;
    FT_STATUS status;

    if (usbconn_ftd2xx_common_open (conn, URJ_LOG_LEVEL_NORMAL)
        != URJ_STATUS_OK)
        return URJ_STATUS_FAIL;

    fc = p->fc;

    /* This sequence might seem weird and containing superfluous stuff.
       However, it's built after the description of JTAG_InitDevice
       Ref. FTCJTAGPG10.pdf
       Intermittent problems will occur when certain steps are skipped. */
    if ((status = FT_ResetDevice (fc)) != FT_OK)
        urj_error_set (URJ_ERROR_IO, _("Can't reset device"));
    if (status == FT_OK)
        if ((status = FT_Purge (fc, FT_PURGE_RX)) != FT_OK)
            urj_error_set (URJ_ERROR_IO, _("s(): Can't purge RX buffer"));

    if (status == FT_OK)
        if ((status =
             FT_SetUSBParameters (fc, URJ_USBCONN_FTDX_MAXSEND_MPSSE,
                                  URJ_USBCONN_FTDX_MAXSEND_MPSSE)) != FT_OK)
            urj_error_set (URJ_ERROR_IO, _("Can't set USB parameters"));

    if (status == FT_OK)
        if ((status = FT_SetChars (fc, 0, 0, 0, 0)) != FT_OK)
            urj_error_set (URJ_ERROR_IO, _("Can't set special characters"));

    /* set a reasonable latency timer value
       if this value is too low then the chip will send intermediate result data
       in short packets (suboptimal performance) */
    if (status == FT_OK)
        if ((status = FT_SetLatencyTimer (fc, 16)) != FT_OK)
            urj_error_set (URJ_ERROR_IO, _("Can't set target latency timer"));

    if (status == FT_OK)
        if ((status =
             FT_SetBitMode (fc, 0x0b, 0x02 /* BITMODE_MPSSE */ )) != FT_OK)
            urj_error_set (URJ_ERROR_IO, _("Can't set MPSSE bitmode"));

    if (status == FT_OK)
        if ((status = FT_ResetDevice (fc)) != FT_OK)
            urj_error_set (URJ_ERROR_IO, _("Can't reset device"));
    if (status == FT_OK)
        if ((status = FT_Purge (fc, FT_PURGE_RX)) != FT_OK)
            urj_error_set (URJ_ERROR_IO, _("Can't purge RX buffer"));

    /* set TCK Divisor */
    if (status == FT_OK)
    {
        uint8_t buf[3] = { 0x86, 0x00, 0x00 };
        if (usbconn_ftd2xx_write (conn, buf, 3, 0) < 0)
            status = FT_OTHER_ERROR;
    }
    /* switch off loopback */
    if (status == FT_OK)
    {
        uint8_t buf[1] = { 0x85 };
        if (usbconn_ftd2xx_write (conn, buf, 1, 0) < 0)
            status = FT_OTHER_ERROR;
    }
    if (status == FT_OK)
        if (usbconn_ftd2xx_read (conn, NULL, 0) < 0)
            status = FT_OTHER_ERROR;

    if (status == FT_OK)
        if ((status = FT_ResetDevice (fc)) != FT_OK)
            urj_error_set (URJ_ERROR_IO, _("Can't reset device"));
    if (status == FT_OK)
        if ((status = FT_Purge (fc, FT_PURGE_RX)) != FT_OK)
            urj_error_set (URJ_ERROR_IO, _("Can't purge RX buffer"));

    if (status != FT_OK)
    {
        FT_Close (fc);
        /* mark ftdi layer as not initialized */
        p->fc = NULL;
    }

    return status != FT_OK ? URJ_STATUS_FAIL : URJ_STATUS_OK;
}

/* ---------------------------------------------------------------------- */

static int
usbconn_ftd2xx_close (urj_usbconn_t *conn)
{
    ftd2xx_param_t *p = conn->params;

    if (p->fc)
    {
        FT_Close (p->fc);
        p->fc = NULL;
    }

    return URJ_STATUS_OK;
}

/* ---------------------------------------------------------------------- */

static void
usbconn_ftd2xx_free (urj_usbconn_t *conn)
{
    ftd2xx_param_t *p = conn->params;

    if (p->send_buf)
        free (p->send_buf);
    if (p->recv_buf)
        free (p->recv_buf);
    if (p->serial)
        free (p->serial);

    free (conn->params);
    free (conn);
}

/* ---------------------------------------------------------------------- */

urj_usbconn_driver_t urj_tap_usbconn_ftd2xx_driver = {
    "ftd2xx",
    usbconn_ftd2xx_connect,
    usbconn_ftd2xx_free,
    usbconn_ftd2xx_open,
    usbconn_ftd2xx_close,
    usbconn_ftd2xx_read,
    usbconn_ftd2xx_write
};

urj_usbconn_driver_t urj_tap_usbconn_ftd2xx_mpsse_driver = {
    "ftd2xx-mpsse",
    usbconn_ftd2xx_mpsse_connect,
    usbconn_ftd2xx_free,
    usbconn_ftd2xx_mpsse_open,
    usbconn_ftd2xx_close,
    usbconn_ftd2xx_read,
    usbconn_ftd2xx_write
};


/*
 Local Variables:
 mode:C
 c-default-style:gnu
 indent-tabs-mode:nil
 End:
*/
