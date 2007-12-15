/*
 * $Id$
 *
 * FreeBSD ppi Driver
 * Copyright (C) 2005 Daniel O'Connor
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
 * Written by Daniel O'Connor <doconnor@gsoft.com.au> July 2005.
 *
 */

#include "sysdep.h"

#ifdef HAVE_DEV_PPBUS_PPI_H

#include <fcntl.h>
#include <unistd.h>
#include <dev/ppbus/ppi.h>
#include <dev/ppbus/ppbconf.h>

#include <stdlib.h>
#include <string.h>

#include "parport.h"
#include "cable.h"

parport_driver_t ppi_parport_driver;

typedef struct port_node_t port_node_t;

struct port_node_t {
	parport_t *port;
	port_node_t *next;
};

static port_node_t *ports = NULL;		/* ppi parallel ports */

typedef struct {
	char *portname;
	int fd;
} ppi_params_t;

static parport_t *
ppi_parport_alloc( const char *port )
{
	ppi_params_t *params = malloc( sizeof *params );
	char *portname = strdup( port );
	parport_t *parport = malloc( sizeof *parport );
	port_node_t *node = malloc( sizeof *node );

	if (!node || !parport || !params || !portname) {
		free( node );
		free( parport );
		free( params );
		free( portname );
		return NULL;
	}

	params->portname = portname;
	params->fd = -1;

	parport->params = params;
	parport->driver = &ppi_parport_driver;
	parport->cable = NULL;

	node->port = parport;
	node->next = ports;

	ports = node;

	return parport;
}

static void
ppi_parport_free( parport_t *port )
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

	free( ((ppi_params_t *) port->params)->portname );
	free( port->params );
	free( port );
}

static cable_t *
ppi_connect( const char **par, int parnum )
{
	port_node_t *pn;
	parport_t *parport;

	if (parnum != 1) {
		printf( _("Syntax error!\n") );
		return NULL;
	}

	for (pn = ports; pn; pn = pn->next)
		if (strcmp( pn->port->params, par[0] ) == 0) {
			printf( _("Disconnecting %s from ppi port %s\n"), _(pn->port->cable->driver->description), par[0] );
			pn->port->cable->driver->disconnect( pn->port->cable );
			break;
		}

	printf( _("Initializing on ppi port %s\n"), par[0] );

	parport = ppi_parport_alloc( par[0] );
	if (!parport) {
		printf( _("%s(%d) Out of memory.\n"), __FILE__, __LINE__ );
		return NULL;
	}

	return parport;
}

static int
ppi_open( parport_t *parport )
{
	ppi_params_t *p = parport->params;

	p->fd = open( p->portname, O_RDWR );
	if (p->fd < 0)
		return -1;

	return 0;
}

static int
ppi_close( parport_t *parport )
{
	int r = 0;
	ppi_params_t *p = parport->params;

	if (close( p->fd ) != 0)
		return -1;

	p->fd = -1;
	return r;
}

static int
ppi_set_data( parport_t *parport, uint8_t data )
{
	ppi_params_t *p = parport->params;

	if (ioctl( p->fd, PPISDATA, &data ) == -1)
		return -1;

	return 0;
}

static int
ppi_get_data( parport_t *parport )
{
	unsigned char d;
	ppi_params_t *p = parport->params;

	if (ioctl( p->fd, PPIGDATA, &d ) == -1)
		return -1;

	return d;
}

static int
ppi_get_status( parport_t *parport )
{
	unsigned char d;
	ppi_params_t *p = parport->params;

	if (ioctl( p->fd, PPIGSTATUS, &d ) == -1)
		return -1;

	return d ^ 0x80;			/* BUSY is inverted */
}

static int
ppi_set_control( parport_t *parport, uint8_t data )
{
	ppi_params_t *p = parport->params;

	data ^= 0x0B;				/* SELECT, AUTOFD, and STROBE are inverted */

	if (ioctl( p->fd, PPIGCTRL, &data ) == -1)
		return -1;

	return 0;
}

parport_driver_t ppi_parport_driver = {
	"ppi",
	ppi_connect,
	ppi_parport_free,
	ppi_open,
	ppi_close,
	ppi_set_data,
	ppi_get_data,
	ppi_get_status,
	ppi_set_control
};

#endif /* HAVE_DEV_PPBUS_PPI_H */
