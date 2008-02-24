/*
 * $Id: xpc.c,v 1.8 2003/08/19 08:42:20 telka Exp $
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

#include "sysdep.h"

#include "cable.h"
#include "chain.h"

#include "generic.h"
#include "generic_usbconn.h"

#include "usbconn.h"
#include "usbconn/libusb.h"

/* ----------------------------------------------------------------- */

int xpcu_select_gpio(struct usb_dev_handle *xpcu, int int_or_ext )
{
    if(usb_control_msg(xpcu, 0x40, 0xB0, 0x0052, int_or_ext, NULL, 0, 1000)<0)
    {
        perror("usb_control_msg(0x52.x) (select gpio)");
        return -1;
    }

    return 0;
}

/* ----------------------------------------------------------------- */

int xpcu_request_28(struct usb_dev_handle *xpcu, int value)
{
    /* Maybe clock speed setting? */

    if(usb_control_msg(xpcu, 0x40, 0xB0, 0x0028, value, NULL, 0, 1000)<0)
    {
        perror("usb_control_msg(0x28.x)");
        return -1;
    }

    return 0;
}

/* ---------------------------------------------------------------------- */

int xpcu_write_gpio(struct usb_dev_handle *xpcu, uint8_t bits)
{
    if(usb_control_msg(xpcu, 0x40, 0xB0, 0x0030, bits, NULL, 0, 1000)<0)
    {
        perror("usb_control_msg(0x30.0x00) (write port E)");
        return -1;
    }

    return 0;
}

/* ---------------------------------------------------------------------- */

int xpcu_read_gpio(struct usb_dev_handle *xpcu, uint8_t *bits)
{
    if(usb_control_msg(xpcu, 0xC0, 0xB0, 0x0038, 0, (char*)bits, 1, 1000)<0)
    {
        perror("usb_control_msg(0x38.0x00) (read port E)");
        return -1;
    }

    return 0;
}

/* ---------------------------------------------------------------------- */


int xpcu_read_cpld_version(struct usb_dev_handle *xpcu, uint16_t *buf)
{
    if(usb_control_msg(xpcu, 0xC0, 0xB0, 0x0050, 0x0001, (char*)buf, 2, 1000)<0)
    {
        perror("usb_control_msg(0x50.1) (read_cpld_version)");
        return -1;
    }
    return 0;
}

/* ---------------------------------------------------------------------- */

int xpcu_read_firmware_version(struct usb_dev_handle *xpcu, uint16_t *buf)
{
    if(usb_control_msg(xpcu, 0xC0, 0xB0, 0x0050, 0x0000, (char*)buf, 2, 1000)<0)
    {
        perror("usb_control_msg(0x50.0) (read_firmware_version)");
        return -1;
    }

    return 0;
}

/* ---------------------------------------------------------------------- */

static int 
xpcu_common_init( cable_t *cable )
{
	uint16_t buf;
	struct usb_dev_handle *xpcu;

	if (usbconn_open( cable->link.usb )) return -1;

	xpcu = ((libusb_param_t*)(cable->link.usb->params))->handle;

    if(xpcu_request_28(xpcu, 0x11)<0)
    {
        usb_close(xpcu);
        return -1;
    };

    if(xpcu_write_gpio(xpcu, 8)<0)
    {
        usb_close(xpcu);
        return -1;
    };

    /* Read firmware version (constant embedded in firmware) */

    if(xpcu_read_firmware_version(xpcu, &buf) < 0)
    {
        usb_close(xpcu);
        return -1;
    }
    else
    {
        printf("firmware version = 0x%04X (%u)\n", buf, buf);
    };

    /* Read CPLD version (via GPIF) */

    if(xpcu_read_cpld_version(xpcu, &buf) < 0)
    {
        usb_close(xpcu);
        return -1;
    }
    else
    {
        printf("cable CPLD version = 0x%04X (%u)\n", buf, buf);
        if(buf == 0)
        {
            printf("Warning: version '0' can't be correct. Please try resetting the cable\n");
        };
    };

	PARAM_TRST(cable) = 1;

	return 0;
}

static int
xpc_int_init( cable_t *cable )
{
	struct usb_dev_handle *xpcu;

	if (xpcu_common_init( cable )<0) return -1;

	xpcu = ((libusb_param_t*)(cable->link.usb->params))->handle;
	if (xpcu_select_gpio(xpcu, 0)<0) return -1;

	return 0;
}

static int
xpc_ext_init( cable_t *cable )
{
	struct usb_dev_handle *xpcu;

	if (xpcu_common_init( cable )<0) return -1;

	xpcu = ((libusb_param_t*)(cable->link.usb->params))->handle;
	if (xpcu_select_gpio(xpcu, 1)<0) return -1;

	return 0;
}

/* ---------------------------------------------------------------------- */

#define	PROG 3
#define	TCK	2
#define TMS 1
#define	TDI	0
#define	TDO	0

static void
xpc_clock( cable_t *cable, int tms, int tdi, int n )
{
	int i;
	struct usb_dev_handle *xpcu;
	xpcu = ((libusb_param_t*)(cable->link.usb->params))->handle;

	tms = tms ? (1<<TMS) : 0;
	tdi = tdi ? (1<<TDI) : 0;

	if( xpcu_write_gpio(xpcu, (1<<PROG) | (0<<TCK) | tms | tdi)>=0)
	{
		cable_wait( cable );
		for (i = 0; i < n; i++)
		{
			xpcu_write_gpio(xpcu, (1<<PROG) | (1<<TCK) | tms | tdi);
			cable_wait( cable );
			xpcu_write_gpio(xpcu, (1<<PROG) | (0<<TCK) | tms | tdi);
			cable_wait( cable );
		}
	}
}

/* ---------------------------------------------------------------------- */

static int
xpc_get_tdo( cable_t *cable )
{
	unsigned char d;
	struct usb_dev_handle *xpcu;
	xpcu = ((libusb_param_t*)(cable->link.usb->params))->handle;

	xpcu_read_gpio(xpcu, &d);
	return (d&(1<<TDO))?1:0;
}

/* ---------------------------------------------------------------------- */

static int
xpc_set_trst( cable_t *cable, int trst )
{
	return 1;
}

cable_driver_t xpc_int_cable_driver = {
	"xpc_int",
	N_("Xilinx Platform Cable USB internal chain"),
	generic_usbconn_connect,
	generic_disconnect,
	generic_usbconn_free,
	xpc_int_init,
	generic_usbconn_done,
	generic_set_frequency,
	xpc_clock,
	xpc_get_tdo,
	generic_transfer,
	xpc_set_trst,
	generic_get_trst,
	generic_flush_using_transfer,
	generic_usbconn_help
};

usbconn_cable_t usbconn_cable_xpc_int = {
	"xpc_int",			/* cable name */
	NULL,				/* string pattern, not used */
	"libusb",			/* usbconn driver */ 
	0x03FD,				/* VID (Xilinx) */
	0x0008				/* PID (8) */
};

cable_driver_t xpc_ext_cable_driver = {
	"xpc_ext",
	N_("Xilinx Platform Cable USB external chain"),
	generic_usbconn_connect,
	generic_disconnect,
	generic_usbconn_free,
	xpc_ext_init,
	generic_usbconn_done,
	generic_set_frequency,
	xpc_clock,
	xpc_get_tdo,
	generic_transfer,
	xpc_set_trst,
	generic_get_trst,
	generic_flush_using_transfer,
	generic_usbconn_help
};

usbconn_cable_t usbconn_cable_xpc_ext = {
	"xpc_ext",			/* cable name */
	NULL,				/* string pattern, not used */
	"libusb",			/* usbconn driver */ 
	0x03FD,				/* VID (Xilinx) */
	0x0008				/* PID (8) */
};


