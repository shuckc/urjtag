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

#include "parport.h"
#include "cable.h"

#include "xpcu.h"

typedef struct {
	char *serial;
	unsigned int vendor_id;
	unsigned int product_id;
    usb_dev_handle *dev;
} xpcu_params_t;

parport_driver_t xpcu_pp_driver;

typedef struct port_node_t port_node_t;

struct port_node_t {
	parport_t *port;
	port_node_t *next;
};

static port_node_t *ports = NULL;		/* devices */

/* ---------------------------------------------------------------------- */

static parport_t *
xpcu_pp_alloc( const char *vidpid )
{
	xpcu_params_t *params = malloc( sizeof *params );
	parport_t *parport = malloc( sizeof *parport );
	port_node_t *node = malloc( sizeof *node );

	if (!node || !parport || !params)  {
		free( node );
		free( parport );
		free( params );
		return NULL;
	}

	params->product_id = 0;
	params->vendor_id = 0;
	params->serial = NULL;
	params->dev = NULL;

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
	parport->driver = &xpcu_pp_driver;
	parport->cable = NULL;

	node->port = parport;
	node->next = ports;

	ports = node;

	return parport;
}

/* ---------------------------------------------------------------------- */

static void
xpcu_pp_free( parport_t *port )
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

	free( ((xpcu_params_t *) port->params)->serial );
	free( port->params );
	free( port );
}

/* ---------------------------------------------------------------------- */

parport_t *
xpcu_pp_connect( const char **par, int parnum )
{
	port_node_t *pn;
	parport_t *parport;


	if (parnum != 1) {
		printf( _("Syntax error!\n") );
		return NULL;
	}

	for (pn = ports; pn; pn = pn->next)
		if (strcmp( pn->port->params, par[0] ) == 0) {
			printf( _("Disconnecting %s, device %s\n"), _(pn->port->cable->driver->description), par[0] );
			pn->port->cable->driver->disconnect( pn->port->cable );
			break;
		}

#ifdef TODO
	printf( _("Initializing %s, device %s\n"), _(cable_drivers[i]->description), par[0] );
#else
	printf( _("Initializing device %s\n"), par[0] );
#endif

	parport = xpcu_pp_alloc( par[0] );
	if (!parport) {
		printf( _("%s(%d) Out of memory.\n"), __FILE__, __LINE__ );
		return NULL;
	}

	return parport;
}

/* ---------------------------------------------------------------------- */

static int
xpcu_pp_open( parport_t *parport )
{
	xpcu_params_t *p = parport->params;

	usb_init();
  
	if(usb_find_busses()<0)
	{
		perror("usb_find_busses failed");
		return -1;
	};

	if(xpcu_init() < 0)
	{
		fprintf (stderr, "can't initialize XPCU\n");
		return -1;
		};

	if(xpcu_open(&(p->dev)) < 0)
	{
		fprintf (stderr, "can't open XPCU\n");
		return -1;
	};

	if(xpcu_raise_ioa5(p->dev)<0) return -1;

	/* access external chain by default */
	if(xpcu_select_gpio(p->dev, 0)<0) return -1;

	return 0;
}

/* ---------------------------------------------------------------------- */

static int
xpcu_pp_close( parport_t *parport )
{
	xpcu_params_t *p = parport->params;
	xpcu_close(p->dev);
	return 0;
}

/* ---------------------------------------------------------------------- */

static int
xpcu_pp_set_data( parport_t *parport, uint8_t data )
{
	xpcu_params_t *p = parport->params;

	if(xpcu_write_gpio(p->dev, data) < 0) return -1;
	return 0;
}

/* ---------------------------------------------------------------------- */

static int
xpcu_pp_get_data( parport_t *parport )
{
	unsigned char d;
	xpcu_params_t *p = parport->params;

	if(xpcu_read_gpio(p->dev, &d) < 0) return 0;
	return d;
}

/* ---------------------------------------------------------------------- */

static int
xpcu_pp_get_status( parport_t *parport )
{
	return 0;
}

/* ---------------------------------------------------------------------- */

static int
xpcu_pp_set_control( parport_t *parport, uint8_t data )
{
	xpcu_params_t *p = parport->params;

    if(xpcu_select_gpio(p->dev, data)<0) return -1;
	return 0;
}

/* ---------------------------------------------------------------------- */

parport_driver_t xpcu_pp_driver = {
	"xpcu",
	xpcu_pp_connect,
	xpcu_pp_free,
	xpcu_pp_open,
	xpcu_pp_close,
	xpcu_pp_set_data,
	xpcu_pp_get_data,
	xpcu_pp_get_status,
	xpcu_pp_set_control
};

#endif /* HAVE_LIBUSB */
