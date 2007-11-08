/*
 * $Id: ftdi.c,v 1.7 2003/08/19 09:05:25 telka Exp $
 *
 * libftdi Driver
 * Copyright (C) 2006 K. Waschk
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
 * Written by Kolja Waschk, 2006.
 * Structure taken from ppdev.c, written by Marcel Telka, 2003.
 *
 */

#include "sysdep.h"

#ifdef HAVE_LIBFTDI

#include <fcntl.h>
#include <stropts.h>
#include <unistd.h>
#include <linux/ioctl.h>

#include <stdlib.h>
#include <string.h>

#include <ftdi.h>

#include "parport.h"
#include "cable.h"

parport_driver_t ftdi_parport_driver;

typedef struct port_node_t port_node_t;

struct port_node_t {
	parport_t *port;
	port_node_t *next;
};

static port_node_t *ports = NULL;		/* devices */

#define OUTBUF_LEN 64 

typedef struct {
	char *serial;
	unsigned int vendor_id;
	unsigned int product_id;
	char autoflush;
	struct ftdi_context *fc;
	unsigned char outcount;
	unsigned char *outbuf;
} ftdi_params_t;

static int ftdi_flush_output ( ftdi_params_t *p );

static parport_t *
ftdi_parport_alloc( const char *vidpid )
{
	ftdi_params_t *params = malloc( sizeof *params );
	parport_t *parport = malloc( sizeof *parport );
	port_node_t *node = malloc( sizeof *node );
	struct ftdi_context *fc = malloc( sizeof(struct ftdi_context) );
	unsigned char *outbuf = malloc( OUTBUF_LEN );

	if (!node || !parport || !params || !fc || !outbuf)  {
		free( node );
		free( parport );
		free( params );
		free( fc );
		return NULL;
	}

	ftdi_init(fc);
	params->outbuf = outbuf;
	params->outcount = 0;
	params->autoflush = 0;
	params->product_id = 0;
	params->vendor_id = 0;
	params->serial = NULL;
	params->fc = fc;

	{
		char *f = strchr(vidpid, ':');
		char *l = strrchr(vidpid, ':');
		if(f)
		{
			params->vendor_id = strtoul(vidpid, NULL, 16);
			params->product_id = strtoul(f+1, NULL, 16);
			if(l!=f) params->serial = strdup(l+1);
		};
	};

	parport->params = params;
	parport->driver = &ftdi_parport_driver;
	parport->cable = NULL;

	node->port = parport;
	node->next = ports;

	ports = node;

	return parport;
}

static void
ftdi_parport_free( parport_t *port )
{
	port_node_t **prev;

	for (prev = &ports; *prev; prev = &((*prev)->next))
		if ((*prev)->port == port)
			break;

	if (*prev) {
		port_node_t *pn = *prev;
		*prev = pn->next;
		free( pn );
	}

	free( ((ftdi_params_t *) port->params)->serial );
	free( ((ftdi_params_t *) port->params)->outbuf );
	free( ((ftdi_params_t *) port->params)->fc );
	free( port->params );
	free( port );
}

static cable_t *
ftdi_connect( const char **par, int parnum )
{
	int i;
	port_node_t *pn;
	parport_t *parport;
	cable_t *cable;

	if (parnum != 2) {
		printf( _("Syntax error!\n") );
		return NULL;
	}

	for (pn = ports; pn; pn = pn->next)
		if (strcmp( pn->port->params, par[0] ) == 0) {
			printf( _("Disconnecting %s from FTDI device %s\n"), _(pn->port->cable->driver->description), par[0] );
			pn->port->cable->driver->disconnect( pn->port->cable );
			break;
		}

	if (strcmp( par[1], "none" ) == 0) {
		printf( _("Changed cable to 'none'\n") );
		return NULL;
	}

	for (i = 0; cable_drivers[i]; i++)
		if (strcmp( par[1], cable_drivers[i]->name ) == 0)
			break;

	if (!cable_drivers[i]) {
		printf( _("Unknown cable: %s\n"), par[1] );
		return NULL;
	}

	printf( _("Initializing %s on FTDI device %s\n"), _(cable_drivers[i]->description), par[0] );

	parport = ftdi_parport_alloc( par[0] );
	if (!parport) {
		printf( _("%s(%d) Out of memory.\n"), __FILE__, __LINE__ );
		return NULL;
	}

	cable = cable_drivers[i]->connect( cable_drivers[i], parport );
	if (!cable)
		ftdi_parport_free( parport );

	return cable;
}

static int
ftdi_open( parport_t *parport )
{
	int r;
	ftdi_params_t *p = parport->params;
	struct ftdi_context *fc = p->fc;

	/* Try to be intelligent about IDs */

	if(p->vendor_id)
		r = ftdi_usb_open_desc(fc, p->vendor_id, p->product_id, NULL, p->serial);	 /* USB-Blaster */
	else
	{
		r = ftdi_usb_open_desc(fc, 0x09FB, 0x6001, NULL, p->serial);	 /* USB-Blaster */
		if(r<0) r = ftdi_usb_open_desc(fc, 0x09FB, 0x6002, NULL, p->serial); /* Cubic Cyclonium */
		if(r<0) r = ftdi_usb_open_desc(fc, 0x09FB, 0x6003, NULL, p->serial); /* NIOS II Evaluation board */
		if(r<0) r = ftdi_usb_open_desc(fc, 0x16C0, 0x06AD, NULL, p->serial); /* http://www.ixo.de/info/usb_jtag/ */
	};

	if(r<0)
	{
		fprintf (stderr, "Can't open ftdi device: %s\n", 
						ftdi_get_error_string (fc));
		ftdi_deinit(fc);
		return -1;
	};

	(void)ftdi_disable_bitbang(fc);

	if(ftdi_set_latency_timer(fc, 2)<0)
	{
		fprintf (stderr, "Can't set minimum latency: %s\n", 
						ftdi_get_error_string (fc));
		ftdi_usb_close(fc);
		ftdi_deinit(fc);
		return -1;
	};

#if 1
	/* libftdi 0.6 doesn't allow high baudrates, so we send the control
	   message outselves */

	if (usb_control_msg(fc->usb_dev, 0x40, 3, 1, 0, NULL, 0, fc->usb_write_timeout) != 0)
	{
		fprintf (stderr, "Can't set max baud rate.\n");
		ftdi_usb_close(fc);
		ftdi_deinit(fc);
		return -1;
	};
#else
	if(ftdi_set_baudrate(fc, 48000000)<0)
	{
	  fprintf (stderr, "Can't set max baud rate: %s\n", 
						ftdi_get_error_string (fc));
		ftdi_usb_close(fc);
		ftdi_deinit(fc);
		return -1;
	};
#endif

	return 0;
}

static int
ftdi_flush_output ( ftdi_params_t *p )
{
	int xferred;

	xferred = ftdi_write_data(p->fc, p->outbuf, p->outcount);

	if(xferred > 0 && xferred < p->outcount)
	{
		int offset = xferred;
		int remaining = p->outcount - xferred;

		while(remaining)
		{
			printf("W\n");
			if(xferred < 0) return xferred;
			xferred = ftdi_write_data(p->fc, p->outbuf + offset, remaining);
            if(xferred < 0)
            {
              memmove(p->outbuf, p->outbuf + offset, remaining);
              p->outcount = remaining;
              return 0;
            }
            offset += xferred;
			remaining  -= xferred;
		}
	};
	p->outcount = 0;

	return 0;
}

static int
ftdi_close( parport_t *parport )
{
	ftdi_params_t *p = parport->params;

	if(p->outcount > 0) ftdi_flush_output( p );
	p->outcount = 0;

	ftdi_usb_close(p->fc);
	ftdi_deinit(p->fc);

	return 0;
}

static int
ftdi_set_data( parport_t *parport, uint8_t data )
{
	ftdi_params_t *p = parport->params;

	if(p->autoflush)
	{
		if(ftdi_write_data(p->fc, &data, 1) != 1) printf("w\n");
	}
	else
	{
		p->outbuf[p->outcount++] = data;

		if(p->outcount >= OUTBUF_LEN)
			return ftdi_flush_output( p );
	};
	   
	return 0;
}

static int
ftdi_get_data( parport_t *parport )
{
	unsigned char d;
	ftdi_params_t *p = parport->params;

	while(ftdi_read_data( p->fc, &d, 1) == 0);
	return d;
}

static int
ftdi_get_status( parport_t *parport )
{
	return 0;
}

static int
ftdi_set_control( parport_t *parport, uint8_t data )
{
	ftdi_params_t *p = parport->params;

	p->autoflush = data;
	if(p->autoflush) ftdi_flush_output( p );
	
	return 0;
}

parport_driver_t ftdi_parport_driver = {
	"ftdi",
	ftdi_connect,
	ftdi_parport_free,
	ftdi_open,
	ftdi_close,
	ftdi_set_data,
	ftdi_get_data,
	ftdi_get_status,
	ftdi_set_control
};

#endif /* HAVE_LIBFTDI */
