/*
 * $Id$
 *
 * Link driver for accessing USB devices via libusb
 *
 * Copyright (C) 2008 K. Waschk
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
 * Written by Kolja Waschk, 2008
 *
 */

#include <sysdep.h>

#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>

#include <urjtag/error.h>
#include <urjtag/log.h>
#include <urjtag/usbconn.h>
#include "libusb.h"
#include "../usbconn.h"

/* ---------------------------------------------------------------------- */

/* @return 1 when found, 0 when not */
static int
libusb_match_desc (libusb_device_handle *handle, unsigned int id, const char *desc)
{
    int r;
    char buf[256];

    if (!id)
        return 0;

    r = libusb_get_string_descriptor_ascii (handle, id, (unsigned char *) buf,
                                            sizeof (buf));
    if (r <= 0)
        return 0;

    return strstr (buf, desc) ? 1 : 0;
}

/* @return 1 when found, 0 when not */
static int
libusb_match (libusb_device *dev, urj_usbconn_cable_t *template)
{
    int r = 0;
    struct libusb_device_descriptor *desc;
    libusb_device_handle *handle;

#ifdef HAVE_LIBUSB1
    struct libusb_device_descriptor _desc;
    desc = &_desc;

    if (libusb_get_device_descriptor (dev, desc))
        return 0;
#else
    desc = &dev->descriptor;
#endif

    /* If VID specified but no match, return fail */
    if (template->vid >= 0 && desc->idVendor != template->vid)
        return 0;

    /* If PID specified but no match, return fail */
    if (template->pid >= 0 && desc->idProduct != template->pid)
        return 0;

    /* If desc not specified, then return pass */
    if (template->desc == NULL)
        return 1;

    /* At this point, try to match the description */
    r = libusb_open (dev, &handle);
    if (r)
    {
        urj_error_set (URJ_ERROR_USB, "usb_open() failed: %i", r);
        errno = 0;
        return 0;
    }
    r = libusb_match_desc (handle, desc->iManufacturer, template->desc);
    if (r <= 0)
        r = libusb_match_desc (handle, desc->iProduct, template->desc);
    if (r <= 0)
        r = libusb_match_desc (handle, desc->iSerialNumber, template->desc);
    libusb_close (handle);

    return r > 0 ? 1 : 0;
}


/* ---------------------------------------------------------------------- */

static urj_usbconn_t *
usbconn_libusb_connect (urj_usbconn_cable_t *template,
                        const urj_param_t *params[])
{
    int ret;
    libusb_device *found_dev = NULL;
    urj_usbconn_t *libusb_conn;
    urj_usbconn_libusb_param_t *libusb_params;
    libusb_context *ctx;
    libusb_device **list;
    ssize_t num_devs, i;

    /* XXX: missing libusb_exit() */
    ret = libusb_init (&ctx);
    if (ret)
    {
        urj_error_set (URJ_ERROR_USB, "libusb_init() failed: %i", ret);
        errno = 0;
        return NULL;
    }

    num_devs = libusb_get_device_list (ctx, &list);
    for (i = 0; i < num_devs; ++i)
    {
        libusb_device *dev = list[i];

        if (libusb_match (dev, template))
            found_dev = libusb_ref_device (dev);
    }
    libusb_free_device_list (list, 0);

    if (!found_dev)
    {
        urj_error_set (URJ_ERROR_NOTFOUND, "no USB connections");
        return NULL;
    }

    libusb_conn = malloc (sizeof (urj_usbconn_t));
    libusb_params = malloc (sizeof (urj_usbconn_libusb_param_t));
    if (libusb_params == NULL || libusb_conn == NULL)
    {
        urj_error_set (URJ_ERROR_OUT_OF_MEMORY,
                       _("malloc(%zd)/malloc(%zd) fails"),
                       sizeof (urj_usbconn_t),
                       sizeof (urj_usbconn_libusb_param_t));
        if (libusb_params)
            free (libusb_params);
        if (libusb_conn)
            free (libusb_conn);
        return NULL;
    }

    libusb_params->dev = found_dev;
    libusb_params->handle = NULL;
    libusb_conn->params = libusb_params;
    libusb_conn->driver = &urj_tap_usbconn_libusb_driver;
    libusb_conn->cable = NULL;

    return libusb_conn;
}


/* ---------------------------------------------------------------------- */

static int
usbconn_libusb_open (urj_usbconn_t *conn)
{
    int ret;
    urj_usbconn_libusb_param_t *p = conn->params;

    ret = libusb_open (p->dev, &p->handle);
    if (ret)
    {
        urj_error_set (URJ_ERROR_USB, "libusb_open() failed: %i", ret);
        errno = 0;
    }
    else
    {
#if 1
        uint8_t configuration;
#ifdef HAVE_LIBUSB1
        struct libusb_config_descriptor *config;
        libusb_get_active_config_descriptor (p->dev, &config);
        configuration = config->bConfigurationValue;
        libusb_free_config_descriptor (config);
#else
        configuration = p->dev->config[0].bConfigurationValue;
#endif
        libusb_set_configuration (p->handle, configuration);
#endif
        ret = libusb_claim_interface (p->handle, 0);
        if (ret)
        {
            libusb_close (p->handle);
            urj_error_set (URJ_ERROR_USB, "libusb_claim_interface failed: %i",
                           ret);
            errno = 0;
            p->handle = NULL;
        }
#if 1
        else
        {
            libusb_set_interface_alt_setting (p->handle, 0, 0);
        }
#endif
    }

    if (p->handle == NULL)
    {
        /* TODO: disconnect? */
        return URJ_STATUS_FAIL;
    }

    return URJ_STATUS_OK;
}

/* ---------------------------------------------------------------------- */

static int
usbconn_libusb_close (urj_usbconn_t *conn)
{
    urj_usbconn_libusb_param_t *p = conn->params;
    if (p->handle != NULL)
    {
        libusb_release_interface (p->handle, 0);
        libusb_close (p->handle);
    }
    p->handle = NULL;
    return URJ_STATUS_OK;
}

/* ---------------------------------------------------------------------- */

static void
usbconn_libusb_free (urj_usbconn_t *conn)
{
    free (conn->params);
    free (conn);
}

/* ---------------------------------------------------------------------- */

const urj_usbconn_driver_t urj_tap_usbconn_libusb_driver = {
    "libusb",
    usbconn_libusb_connect,
    usbconn_libusb_free,
    usbconn_libusb_open,
    usbconn_libusb_close,
    NULL,
    NULL
};
