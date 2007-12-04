/*
 * $Id$
 *
 * Linux ppdev Driver
 * Copyright (C) 2003 ETC s.r.o.
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
 * Written by Marcel Telka <marcel@telka.sk>, 2003.
 *
 */

#include "sysdep.h"

#ifdef HAVE_LINUX_PPDEV_H

#include <fcntl.h>
#include <stropts.h>
#include <unistd.h>
#include <linux/ppdev.h>
#include <linux/ioctl.h>

#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "parport.h"
#include "cable.h"

parport_driver_t ppdev_parport_driver;

typedef struct port_node_t port_node_t;

struct port_node_t {
	parport_t *port;
	port_node_t *next;
};

static port_node_t *ports = NULL;		/* ppdev parallel ports */

typedef struct {
	char *portname;
	int fd;
} ppdev_params_t;

static parport_t *
ppdev_parport_alloc( const char *port )
{
	ppdev_params_t *params = malloc( sizeof *params );
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
	parport->driver = &ppdev_parport_driver;
	parport->cable = NULL;

	node->port = parport;
	node->next = ports;

	ports = node;

	return parport;
}

static void
ppdev_parport_free( parport_t *port )
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

	free( ((ppdev_params_t *) port->params)->portname );
	free( port->params );
	free( port );
}

parport_t *
ppdev_connect( const char **par, int parnum )
{
	port_node_t *pn;
	parport_t *parport;

	if (parnum != 1) {
		printf( _("Syntax error!\n") );
		return NULL;
	}

	for (pn = ports; pn; pn = pn->next)
		if (strcmp( pn->port->params, par[0] ) == 0) {
			printf( _("Disconnecting %s from ppdev port %s\n"), _(pn->port->cable->driver->description), par[0] );
			pn->port->cable->driver->disconnect( pn->port->cable );
			break;
		}

	printf( _("Initializing ppdev port %s\n"), par[0] );

	parport = ppdev_parport_alloc( par[0] );
	if (!parport) {
		printf( _("%s(%d) Out of memory.\n"), __FILE__, __LINE__ );
		return NULL;
	}

	return parport;
}

static int
ppdev_open( parport_t *parport )
{
	ppdev_params_t *p = parport->params;

	p->fd = open( p->portname, O_RDWR );
	if (p->fd < 0) {
		printf( _("Could not open port %s: %s\n"), p->portname, strerror(errno) );
		return -1;
        }

	if (/*(ioctl( p->fd, PPEXCL ) == -1) ||*/ (ioctl( p->fd, PPCLAIM ) == -1))  {
		printf( _("Could not claim ppdev device: %s\n"), strerror(errno) );
		close( p->fd );
		p->fd = -1;
		return -1;
	}

	return 0;
}

static int
ppdev_close( parport_t *parport )
{
	int r = 0;
	ppdev_params_t *p = parport->params;

	if (ioctl( p->fd, PPRELEASE ) == -1)
		r = -1;

	if (close( p->fd ) != 0)
		return -1;

	p->fd = -1;
	return r;
}

static int
ppdev_set_data( parport_t *parport, uint8_t data )
{
	ppdev_params_t *p = parport->params;

	if (ioctl( p->fd, PPWDATA, &data ) == -1)
		return -1;

	return 0;
}

static int
ppdev_get_data( parport_t *parport )
{
	unsigned char d;
	ppdev_params_t *p = parport->params;

	if (ioctl( p->fd, PPRDATA, &d ) == -1)
		return -1;

	return d;
}

static int
ppdev_get_status( parport_t *parport )
{
	unsigned char d;
	ppdev_params_t *p = parport->params;

	if (ioctl( p->fd, PPRSTATUS, &d ) == -1)
		return -1;

	return d ^ 0x80;			/* BUSY is inverted */
}

static int
ppdev_set_control( parport_t *parport, uint8_t data )
{
	ppdev_params_t *p = parport->params;

	data ^= 0x0B;				/* SELECT, AUTOFD, and STROBE are inverted */

	if (ioctl( p->fd, PPWCONTROL, &data ) == -1)
		return -1;

	return 0;
}

parport_driver_t ppdev_parport_driver = {
	"ppdev",
	ppdev_connect,
	ppdev_parport_free,
	ppdev_open,
	ppdev_close,
	ppdev_set_data,
	ppdev_get_data,
	ppdev_get_status,
	ppdev_set_control
};

#endif /* HAVE_LINUX_PPDEV_H */
