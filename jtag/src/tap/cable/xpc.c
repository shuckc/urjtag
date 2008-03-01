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

#include <errno.h>
#include <string.h>

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

int xpcu_output_enable(struct usb_dev_handle *xpcu, int enable)
{
    if(usb_control_msg(xpcu, 0x40, 0xB0, enable ? 0x18 : 0x10, 0, NULL, 0, 1000)<0)
    {
        perror("usb_control_msg(0x10/0x18)");
        return -1;
    }

    return 0;
}

/* ---------------------------------------------------------------------- */

int xpcu_bit_reverse(struct usb_dev_handle *xpcu, uint8_t bits_in, uint8_t *bits_out)
{
    if(usb_control_msg(xpcu, 0xC0, 0xB0, 0x0020, bits_in, (char*)bits_out, 1, 1000)<0)
    {
        perror("usb_control_msg(0x20.x) (bit reverse)");
        return -1;
    }

    return 0;
}

/* ----------------------------------------------------------------- */

int xpcu_request_28(struct usb_dev_handle *xpcu, int value)
{
    /* Typical values seen during autodetection of chain configuration: 0x11, 0x12 */

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

static int
xpcu_shift(struct usb_dev_handle *xpcu, int reqno, int bits, int in_len, uint8_t *in, int out_len, uint8_t *out )
{
    if(usb_control_msg(xpcu, 0x40, 0xB0, reqno, bits, NULL, 0, 1000)<0)
    {
        perror("usb_control_msg(x.x) (shift)");
        return -1;
    }

#if VERBOSE
	{
	int i;
    printf("###\n");
    printf("reqno = %02X\n", reqno);
    printf("bits    = %d\n", bits);
    printf("in_len  = %d, in_len*2  = %d\n", in_len, in_len * 2);
    printf("out_len = %d, out_len*8 = %d\n", out_len, out_len * 8);

    printf("a6_display(\"%02X\", \"", bits);
    for(i=0;i<in_len;i++) printf("%02X%s", in[i], (i+1<in_len)?",":"");
    printf("\", ");
	}
#endif

    if(usb_bulk_write(xpcu, 0x02, (char*)in, in_len, 1000)<0)
    {
        printf("\nusb_bulk_write error(shift): %s\n", strerror(errno));
        return -1;
    }

    if(out_len > 0 && out != NULL)
    {
      if(usb_bulk_read(xpcu, 0x86, (char*)out, out_len, 1000)<0)
      {
        printf("\nusb_bulk_read error(shift): %s\n", strerror(errno));
        return -1;
      }
    }

#if VERBOSE
	{
	int i;
    printf("\"");
    for(i=0;i<out_len;i++) printf("%02X%s", out[i], (i+1<out_len)?",":"");
    printf("\")\n");
	}
#endif
 
    return 0;
}

/* ---------------------------------------------------------------------- */

static int 
xpcu_common_init( cable_t *cable )
{
    int r;
    uint16_t buf;
    struct usb_dev_handle *xpcu;

    if (usbconn_open( cable->link.usb )) return -1;

    xpcu = ((libusb_param_t*)(cable->link.usb->params))->handle;

    r = xpcu_request_28(xpcu, 0x11);
    if (r>=0) r = xpcu_write_gpio(xpcu, 8);

    /* Read firmware version (constant embedded in firmware) */

    if (r>=0) r = xpcu_read_firmware_version(xpcu, &buf);
    if (r>=0)
    {
        printf("firmware version = 0x%04X (%u)\n", buf, buf);
    };

    /* Read CPLD version (via GPIF) */

    if (r>=0) xpcu_read_cpld_version(xpcu, &buf);
    if (r>=0)
    {
        printf("cable CPLD version = 0x%04X (%u)\n", buf, buf);
        if(buf == 0)
        {
            printf("Warning: version '0' can't be correct. Please try resetting the cable\n");
            r = -1;
        };
    };

    if (r<0)
    {
        usb_close(xpcu);
    };

    return r;
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
	uint8_t zero[2] = {0,0};
	int r;

	free(cable->params);
	cable->params = NULL;

	r = xpcu_common_init( cable );

	if (r<0) return r;

	cable->params = malloc(sizeof(xpc_cable_params_t));
	if(cable->params == NULL) r = -1;

	xpcu = ((libusb_param_t*)(cable->link.usb->params))->handle;

	if (r>=0) r = xpcu_request_28(xpcu, 0x11);
	if (r>=0) r = xpcu_output_enable(xpcu, 1);
	if (r>=0) r = xpcu_shift(xpcu, 0xA6, 2, 2, zero, 0, NULL);
	if (r>=0) r = xpcu_request_28(xpcu, 0x12);

	if (r<0)
	{
		usb_close(xpcu);

		free(cable->params);
		cable->params = NULL;
	}

	return r;
}

/* ---------------------------------------------------------------------- */

static void
xpc_ext_done( cable_t *cable )
{
	struct usb_dev_handle *xpcu;
	xpcu = ((libusb_param_t*)(cable->link.usb->params))->handle;
	xpcu_output_enable(xpcu, 0);
	generic_usbconn_done( cable );
}

/* ---------------------------------------------------------------------- */

static void
xpc_ext_free( cable_t *cable )
{
	if(cable->params) 
	{
		free(cable->params);
		cable->params = NULL;
	}
    generic_usbconn_free( cable );
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

/* ---------------------------------------------------------------------- */

static void
xpc_ext_clock( cable_t *cable, int tms, int tdi, int n )
{
	int i;
	uint8_t tdo[2];
	uint8_t clock[2];
	struct usb_dev_handle *xpcu;

	clock[0] = (tms?0x10:0) | (tdi?0x01:0);
	clock[1] = 0x11; /* clock'n read */

	xpcu = ((libusb_param_t*)(cable->link.usb->params))->handle;

	for(i=0;i<n;i++) xpcu_shift(xpcu, 0xA6, 1, 2, clock, 2, tdo);

	last_tdo = tdo[1] ? 1:0;
	// ((xpc_cable_params_t*)(cable->params))->last_tdo = tdo[1] ? 1:0;
}

/* ---------------------------------------------------------------------- */

static int
xpc_ext_get_tdo( cable_t *cable )
{
	return last_tdo;
	// return ((xpc_cable_params_t*)(cable->params))->last_tdo;
}

/* ---------------------------------------------------------------------- */

/* 16-bit words. More than 4 currently leads to bit errors; 13 to serious problems */
#define XPC_A6_CHUNKSIZE 4

typedef struct
{
	cable_t *cable;
	struct usb_dev_handle *xpcu;
	int in_bits;
	int out_bits;
	int out_done;
	uint8_t *out;
	uint8_t buf[XPC_A6_CHUNKSIZE*2];
}
xpc_ext_transfer_state_t;

/* ---------------------------------------------------------------------- */

static int
xpcu_do_ext_transfer( xpc_ext_transfer_state_t *xts )
{
	int r;
	int in_len, out_len;
	// int last_tdo;

	in_len = 2 * (xts->in_bits >> 2);
	if ((xts->in_bits & 3) != 0) in_len += 2;

	out_len = 2 * (xts->out_bits >> 4);
	if ((xts->out_bits & 15) != 0) out_len += 2;

	if(xts->out != NULL)
	{
		r = xpcu_shift (xts->xpcu, 0xA6, xts->in_bits, in_len, xts->buf, out_len, xts->buf);
	}
	else
	{
		r = xpcu_shift (xts->xpcu, 0xA6, xts->in_bits, in_len, xts->buf, 0, NULL);
	}

	if(r >= 0 && xts->out_bits > 0)
	{
		int out_idx = 0;
		int out_rem = xts->out_bits;

		while (out_rem > 0)
		{
			uint32_t mask, rxw;

			rxw = (xts->buf[out_idx+1]<<8) | xts->buf[out_idx];

			/* In the last (incomplete) word, the data isn't shifted completely to LSB */

			mask = (out_rem >= 16) ? 1 : (1<<(16 - out_rem));

			while(mask <= 32768 && out_rem > 0)
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
xpcu_add_bit_for_ext_transfer( xpc_ext_transfer_state_t *xts, char in, char is_real )
{
	int bit_idx = (xts->in_bits & 3);
	int buf_idx = (xts->in_bits - bit_idx) >> 1;

	if(bit_idx == 0)
	{
		xts->buf[buf_idx] = 0;
		xts->buf[buf_idx+1] = 0;
	};

	xts->in_bits++;

	if(is_real)
	{
		if(in) xts->buf[buf_idx] |= (0x01<<bit_idx);

		if(xts->out)
		{
			xts->buf[buf_idx+1] |= (0x11<<bit_idx);
			xts->out_bits++;
		}
		else
		{
			xts->buf[buf_idx+1] |= (0x01<<bit_idx);
		}
	}
}
	
/* ---------------------------------------------------------------------- */

static int
xpc_ext_transfer( cable_t *cable, int len, char *in, char *out )
{
	int i,j;
	xpc_ext_transfer_state_t xts;

#if VERBOSE
	printf("---\n");
	printf("transfer size %d, %s output\n", len, (out!=NULL) ? "with" : "without");
	printf("tdi: ");
	for(i=0;i<len;i++) printf("%c", in[i]?'1':'0');
	printf("\n");
#endif

	xts.xpcu = ((libusb_param_t*)(cable->link.usb->params))->handle;
	xts.out = (uint8_t*)out;
	xts.in_bits = 0;
	xts.out_bits = 0;
	xts.out_done = 0;
	xts.cable = cable;

	for(i=0,j=0; i<len && j>=0; i++)
	{
		xpcu_add_bit_for_ext_transfer( &xts, in[i], 1 );
		if(xts.in_bits == (4*XPC_A6_CHUNKSIZE - 1))
		{
			j = xpcu_do_ext_transfer( &xts );
		}
	};

	if(xts.in_bits > 0 && j>=0)
	{
		/* CPLD doesn't like multiples of 4; add one dummy bit */
		if((xts.in_bits & 3) == 0)
		{
			xpcu_add_bit_for_ext_transfer( &xts, 0, 0 );
		}
		j = xpcu_do_ext_transfer( &xts );
	}

	return j;
}

/* ---------------------------------------------------------------------- */


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
	xpc_ext_free,
	xpc_ext_init,
	xpc_ext_done,
	generic_set_frequency,
	xpc_ext_clock,
	xpc_ext_get_tdo,
	xpc_ext_transfer,
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


