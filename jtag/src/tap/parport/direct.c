/*
 * $Id$
 *
 * Direct Parallel Port Connection Driver
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
 * Ported to NetBSD/i386 by Jachym Holecek <freza@psi.cz>, 2003.
 *
 */

#include "sysdep.h"

#include <stdlib.h>
#include <string.h>

#include "parport.h"
#include "cable.h"

#if defined(WIN32) && defined(HAVE_INPOUTXX)
void    _stdcall Out32(short PortAddress, short data);
short   _stdcall Inp32(short PortAddress);

#define inb(p) Inp32(p)
#define outb(d,p) Out32(p,d)

#elif defined(HAVE_IOPERM) || defined(HAVE_I386_SET_IOPERM)

#if defined(HAVE_IOPERM)
#include <sys/io.h>
#elif defined(HAVE_I386_SET_IOPERM)
#include <sys/types.h>
#include <machine/sysarch.h>
#include <err.h>
#endif

#ifdef HAVE_I386_SET_IOPERM
static __inline int
ioperm( unsigned long from, unsigned long num, int permit )
{
#ifdef __FreeBSD__
	if (i386_set_ioperm( from, num, permit ) == -1)
		return -1;
#else
	u_long ports[32];
	u_long i;

	if (i386_get_ioperm( ports ) == -1)
		return -1;

	for (i = from; i < (from + num); i++)
		if (permit)
			ports[i / 32] &= ~(1 << (i % 32));
		else
			ports[i / 32] |= (1 << (i % 32));

	if (i386_set_ioperm( ports ) == -1)
		return -1;
#endif
	return 0;
}

static __inline int
iopl( int level )
{
#ifndef __FreeBSD__
	return i386_iopl( level );
#endif
	return 0;
}

static __inline unsigned char
inb( unsigned short int port )
{
	unsigned char _v;

	__asm__ __volatile__ ("inb %w1,%0":"=a" (_v):"Nd" (port));
	return _v;
}

static __inline void
outb( unsigned char value, unsigned short int port )
{
	__asm__ __volatile__ ("outb %b0,%w1": :"a" (value), "Nd" (port));
}
#endif /* HAVE_I386_SET_IOPERM */

parport_driver_t direct_parport_driver;

typedef struct port_node_t port_node_t;

struct port_node_t {
	parport_t *port;
	port_node_t *next;
};

static port_node_t *ports = NULL;		/* direct parallel ports */

typedef struct {
	unsigned int port;
} direct_params_t;

static parport_t *
direct_parport_alloc( unsigned int port )
{
	direct_params_t *params = malloc( sizeof *params );
	parport_t *parport = malloc( sizeof *parport );
	port_node_t *node = malloc( sizeof *node );

	if (!node || !parport || !params) {
		free( node );
		free( parport );
		free( params );
		return NULL;
	}

	params->port = port;

	parport->params = params;
	parport->driver = &direct_parport_driver;
	parport->cable = NULL;

	node->port = parport;
	node->next = ports;

	ports = node;

	return parport;
}

static void
direct_parport_free( parport_t *port )
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

	free( port->params );
	free( port );
}

parport_t *
direct_connect( const char **par, int parnum )
{
	long int port_scan_val;
	unsigned int port;
	port_node_t *pn = ports;
	parport_t *parport;

	if (parnum != 1) {
		printf( _("Syntax error!\n") );
		return NULL;
	}

	port_scan_val = strtol(par[0], NULL, 0);
	
	if (port_scan_val < 0 || (port_scan_val + 3) > 0xffff) {
		printf( _("Invalid port address!\n") );
		return NULL;
	}

	port = (unsigned int )port_scan_val;

	while (pn)
		for (pn = ports; pn; pn = pn->next) {
			unsigned int aport;

			aport = ((direct_params_t*) pn->port->params)->port;
			if (abs( aport - port ) < 3) {
				printf( _("Disconnecting %s from parallel port at 0x%x\n"), _(pn->port->cable->driver->description), aport );
				pn->port->cable->driver->disconnect( pn->port->cable );
				break;
			}
		}

	printf( _("Initializing parallel port at 0x%x\n"), port );

	parport = direct_parport_alloc( port );
	if (!parport) {
		printf( _("%s(%d) Out of memory.\n"), __FILE__, __LINE__ );
		return NULL;
	}

	return parport;
}

static int
direct_open( parport_t *parport )
{
	unsigned int port = ((direct_params_t *) parport->params)->port;
	return ((port + 3 <= 0x400) && ioperm( port, 3, 1 )) || ((port + 3 > 0x400) && iopl( 3 ));
}

static int
direct_close( parport_t *parport )
{
	unsigned int port = ((direct_params_t *) parport->params)->port;
	return (port + 3 <= 0x400) ? ioperm( port, 3, 0 ) : iopl( 0 );
}

static int
direct_set_data( parport_t *parport, uint8_t data )
{
	unsigned int port = ((direct_params_t *) parport->params)->port;
	outb( data, port );
	return 0;
}

static int
direct_get_data( parport_t *parport )
{
	unsigned int port = ((direct_params_t *) parport->params)->port;
	return inb( port );
}

static int
direct_get_status( parport_t *parport )
{
	unsigned int port = ((direct_params_t *) parport->params)->port;
	return inb( port + 1 ) ^ 0x80;		/* BUSY is inverted */
}

static int
direct_set_control( parport_t *parport, uint8_t data )
{
	unsigned int port = ((direct_params_t *) parport->params)->port;
	outb( data ^ 0x0B, port + 2 );		/* SELECT, AUTOFD, and STROBE are inverted */
	return 0;
}

parport_driver_t direct_parport_driver = {
	"parallel",
	direct_connect,
	direct_parport_free,
	direct_open,
	direct_close,
	direct_set_data,
	direct_get_data,
	direct_get_status,
	direct_set_control
};

#endif /* defined(HAVE_IOPERM) || defined(HAVE_I386_SET_IOPERM) */
