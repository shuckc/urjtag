/*
 * $Id: ftdi.c,v 1.7 2003/08/19 09:05:25 telka Exp $
 *
 * Driver for Xilinx Platform Cable USB
 * Copyright (C) 2007 K. Waschk
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
 * Written by Kolja Waschk, 2007.
 * Structure taken from ppdev.c, written by Marcel Telka, 2003.
 *
 */

#include "sysdep.h"

#ifdef HAVE_LIBUSB

#include <fcntl.h>
#if __CYGWIN__
#include <windows.h>
#else
#include <linux/ioctl.h>
#endif
#include <stdio.h>
#include <string.h>
#ifdef HAVE_STROPTS_H
#include <stropts.h>
#endif
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <usb.h>

/* ---------------------------------------------------------------------- */

struct usb_device *find_xpcu(void)
{
	struct usb_device *xpcu_dev = NULL;

	if(usb_find_devices()<0)
	{
		perror("find_xpcu: usb_find_devices failed");
	}
	else
	{
		struct usb_bus *bus;

		for (bus = usb_busses; bus && !xpcu_dev; bus = bus->next)
		{
			struct usb_device *dev;
		
			for (dev = bus->devices; dev && !xpcu_dev; dev = dev->next)
			{
				if(dev->descriptor.idVendor == 0x3fd)
				{
					if(dev->descriptor.idProduct == 0x8)
					{
						xpcu_dev = dev;
					}
					else
					{
						fprintf(stderr, 
							"Found Xilinx device with unknown PID %04X. No firmware loaded?\n",
							dev->descriptor.idProduct);
					};
				};
			};
		};
	};

	return xpcu_dev;
}

/* ---------------------------------------------------------------------- */

int xpcu_init()
{
	struct usb_device *xpcu_dev = find_xpcu();
	struct usb_dev_handle *xpcu;

	if(xpcu_dev == NULL)
	{
		fprintf(stderr, "xpcu_reset: no device found\n");
		return -1;
	};

	xpcu = usb_open(xpcu_dev);
	if(xpcu == NULL)
	{
		perror("xpcu_reset: usb_open() failed");
		return -1;
	};

	if(usb_reset(xpcu) < 0)
	{
		perror("xpcu_reset: usb_reset() failed");
		return -1;
	};

	return 0;
}

/* ---------------------------------------------------------------------- */

int xpcu_close(struct usb_dev_handle *xpcu)
{
	usb_release_interface(xpcu, 0);
	usb_close(xpcu);

	return 0;
}

/* ---------------------------------------------------------------------- */

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


int xpcu_raise_ioa5(struct usb_dev_handle *xpcu)
{
	if(usb_control_msg(xpcu, 0x40, 0xB0, 0x0018, 0x0000, NULL, 0, 1000)<0)
	{
		perror("usb_control_msg(0x18.0x00) (raise IOA.5");
		return -1;
	}

	return 0;
}

/* ---------------------------------------------------------------------- */


int xpcu_select_gpio(struct usb_dev_handle *xpcu, int chain)
{
	if(usb_control_msg(xpcu, 0x40, 0xB0, 0x0052, chain, NULL, 0, 1000)<0)
	{
		perror("usb_control_msg(0x52.x) (select gpio)");
		return -1;
	}

	return 0;
}

/* ---------------------------------------------------------------------- */

int xpcu_open(struct usb_dev_handle **xpcu)
{
	uint16_t buf;
	struct usb_device *xpcu_dev = find_xpcu();

	if(xpcu_dev == NULL)
	{
		fprintf(stderr, "xpcu_open: no device found\n");
		return -1;
	};

	*xpcu = usb_open(xpcu_dev);
	if(*xpcu == NULL)
	{
		perror("xpcu_open: usb_open() failed");
		return -1;
	};

	if(usb_claim_interface(*xpcu, 0) != 0)
	{
		perror("xpcu_open: usb_claim_interface failed");
		usb_close(*xpcu);
		return -1;
	};


	if(xpcu_request_28(*xpcu, 0x11)<0)
	{
		usb_close(*xpcu);
		return -1;
	};

	if(xpcu_write_gpio(*xpcu, 8)<0)
	{
		usb_close(*xpcu);
		return -1;
	};

	/* Read firmware version (constant embedded in firmware) */

	if(xpcu_read_firmware_version(*xpcu, &buf) < 0)
	{
		usb_close(*xpcu);
		return -1;
	}
	else
	{
		printf("firmware version = 0x%04X (%u)\n", buf, buf);
	};

	/* Read CPLD version (via GPIF) */

	if(xpcu_read_cpld_version(*xpcu, &buf) < 0)
	{
		usb_close(*xpcu);
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

	return 0;
}

#endif /* HAVE_LIBUSB */
