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

#ifndef URJ_USBCONN_LIBUSB_H
#define URJ_USBCONN_LIBUSB_H 1

#ifdef HAVE_LIBUSB1
# include <libusb.h>
#else

/* Glue the libusb-0.x API to the libusb-1.x API */
# include <usb.h>

# define LIBUSB_ENDPOINT_IN USB_ENDPOINT_IN

# define libusb_device usb_device
# define libusb_device_handle usb_dev_handle
# define libusb_device_descriptor usb_device_descriptor

# define libusb_reset_device(dev) usb_reset (dev)
# define libusb_close(dev) usb_close (dev)
# define libusb_set_configuration(dev, cfg) usb_set_configuration (dev, cfg)
# define libusb_claim_interface(dev, iface) usb_claim_interface (dev, iface)
# define libusb_set_interface_alt_setting(dev, prev, alt) usb_set_altinterface (dev, alt)
# define libusb_release_interface(dev, iface) usb_release_interface (dev, iface)

static inline int
libusb_bulk_transfer (libusb_device_handle *dev_handle, unsigned char endpoint,
                      unsigned char *data, int length, int *actual_length,
                      unsigned int timeout)
{
    char *cdata = (char *)data;
    int ret;

    if (endpoint & LIBUSB_ENDPOINT_IN)
        ret = usb_bulk_read (dev_handle, endpoint, cdata, length, timeout);
    else
        ret = usb_bulk_write (dev_handle, endpoint, cdata, length, timeout);

    if (ret < 0)
    {
        *actual_length = 0;
        return -ret;
    }
    else
    {
        *actual_length = ret;
        return 0;
    }
}

static inline int
libusb_control_transfer (libusb_device_handle *dev_handle, uint8_t request_type,
                         uint8_t request, uint16_t value, uint16_t index,
                         unsigned char *data, uint16_t length, unsigned int timeout)
{
    char *cdata = (char *)data;
    int ret;

    ret = usb_control_msg (dev_handle, request_type, request, value, index,
                           cdata, length, timeout);

    if (ret < 0)
        return -ret;
    else
        return ret;
}

static inline int
libusb_open (struct libusb_device *dev, libusb_device_handle **handle)
{
    *handle = usb_open (dev);
    return *handle ? 0 : -99;
}

static inline int
libusb_get_string_descriptor_ascii (libusb_device_handle *dev, uint8_t index,
                                    unsigned char *data, int length)
{
    return usb_get_string_simple (dev, index, (char *) data, length);
}

#endif

/* XXX: libusb-1.x doesn't have a replacement ? :( */
#define usb_strerror() dont_use_usb_strerror

typedef struct
{
    struct libusb_device *dev;
    struct libusb_device_handle *handle;
    void *data;
} urj_usbconn_libusb_param_t;

#endif
