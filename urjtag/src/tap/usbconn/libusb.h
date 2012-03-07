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

# define LIBUSB_REQUEST_TYPE_STANDARD USB_TYPE_STANDARD
# define LIBUSB_REQUEST_TYPE_CLASS    USB_TYPE_CLASS
# define LIBUSB_REQUEST_TYPE_VENDOR   USB_TYPE_VENDOR
# define LIBUSB_REQUEST_TYPE_RESERVED USB_TYPE_RESERVED

# define LIBUSB_RECIPIENT_DEVICE    USB_RECIP_DEVICE
# define LIBUSB_RECIPIENT_INTERFACE USB_RECIP_INTERFACE
# define LIBUSB_RECIPIENT_ENDPOINT  USB_RECIP_ENDPOINT
# define LIBUSB_RECIPIENT_OTHER     USB_RECIP_OTHER

# define libusb_context void
typedef struct usb_device libusb_device;
# define libusb_device_handle usb_dev_handle
# define libusb_device_descriptor usb_device_descriptor

# define libusb_ref_device(def) def
# define libusb_reset_device(dev) usb_reset (dev)
# define libusb_close(dev) usb_close (dev)
# define libusb_set_configuration(dev, cfg) usb_set_configuration (dev, cfg)
# define libusb_claim_interface(dev, iface) usb_claim_interface (dev, iface)
# define libusb_set_interface_alt_setting(dev, prev, alt) usb_set_altinterface (dev, alt)
# define libusb_release_interface(dev, iface) usb_release_interface (dev, iface)

static inline int
libusb_init (libusb_context **ctx)
{
    int ret;

    *ctx = NULL;

    usb_init ();

    ret = usb_find_busses ();
    if (ret < 0)
        return ret;

    return usb_find_devices ();
}

static inline ssize_t
libusb_get_device_list (libusb_context *ctx, libusb_device ***list)
{
    struct usb_bus *bus;
    ssize_t ret = 0;

    *list = malloc (sizeof (**list));
    for (bus = usb_get_busses (); bus; bus = bus->next)
    {
        libusb_device *dev;

        for (dev = bus->devices; dev; dev = dev->next)
        {
            *list = realloc (*list, sizeof (**list) * (ret + 2));
            (*list)[ret] = malloc (sizeof (***list));
            *(*list)[ret] = *dev;
            ++ret;
        }
    }
    (*list)[ret] = NULL;

    return ret;
}

static inline void
libusb_free_device_list(libusb_device **list, int unref_devices)
{
    size_t i;

    i = 0;
    while (list[i])
        free (list[i++]);

    free (list);
}

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
# define usb_bulk_read(...)  use_libusb_1_api
# define usb_bulk_write(...) use_libusb_1_api

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
# define usb_control_msg(...) use_libusb_1_api

static inline int
libusb_open (libusb_device *dev, libusb_device_handle **handle)
{
    *handle = usb_open (dev);
    return *handle ? 0 : -99;
}
# define usb_open(...) use_libusb_1_api

static inline int
libusb_get_string_descriptor_ascii (libusb_device_handle *dev, uint8_t index,
                                    unsigned char *data, int length)
{
    return usb_get_string_simple (dev, index, (char *) data, length);
}
# define usb_get_string_simple(...) use_libusb_1_api

#endif

/* XXX: libusb-1.x doesn't have a replacement ? :( */
#define usb_strerror() dont_use_usb_strerror

typedef struct
{
    libusb_device *dev;
    struct libusb_device_handle *handle;
    void *data;
} urj_usbconn_libusb_param_t;

#endif
