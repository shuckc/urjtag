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

#include <sysdep.h>

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <dev/ppbus/ppi.h>
#include <dev/ppbus/ppbconf.h>

#include <stdlib.h>
#include <string.h>

#include <urjtag/error.h>
#include <urjtag/log.h>
#include <urjtag/parport.h>
#include <urjtag/cable.h>
#include "../parport.h"

static port_node_t *ports = NULL;       /* ppi parallel ports */

typedef struct
{
    char *portname;
    int fd;
} ppi_params_t;

static urj_parport_t *
ppi_parport_alloc (const char *port)
{
    ppi_params_t *params = malloc (sizeof *params);
    char *portname = strdup (port);
    urj_parport_t *parport = malloc (sizeof *parport);
    port_node_t *node = malloc (sizeof *node);

    if (!node || !parport || !params || !portname)
    {
        free (node);
        free (parport);
        free (params);
        free (portname);
        urj_error_set (URJ_ERROR_OUT_OF_MEMORY,
                       "malloc(%zd)/strdup(%s)/malloc(%zd)/malloc(%zd) fails",
                       sizeof *params, port, sizeof *parport, sizeof *node);
        return NULL;
    }

    params->portname = portname;
    params->fd = -1;

    parport->params = params;
    parport->driver = &urj_tap_parport_ppi_parport_driver;
    parport->cable = NULL;

    node->port = parport;
    node->next = ports;

    ports = node;

    return parport;
}

static void
ppi_parport_free (urj_parport_t *port)
{
    port_node_t **prev;

    for (prev = &ports; *prev; prev = &((*prev)->next))
        if ((*prev)->port == port)
            break;

    if (*prev)
    {
        port_node_t *pn = *prev;
        *prev = pn->next;
        free (pn);
    }

    free (((ppi_params_t *) port->params)->portname);
    free (port->params);
    free (port);
}

static urj_parport_t *
ppi_connect (const char *devname)
{
    port_node_t *pn;
    urj_parport_t *parport;

    for (pn = ports; pn; pn = pn->next)
        if (strcmp (pn->port->params, devname) == 0)
        {
            urj_log (URJ_LOG_LEVEL_NORMAL,
                     _("Disconnecting %s from ppi port %s\n"),
                     _(pn->port->cable->driver->description), devname);
            pn->port->cable->driver->disconnect (pn->port->cable);
            break;
        }

    urj_log (URJ_LOG_LEVEL_NORMAL, _("Initializing ppi port %s\n"), devname);

    parport = ppi_parport_alloc (devname);
    if (!parport)
        return NULL;

    return parport;
}

static int
ppi_open (urj_parport_t *parport)
{
    ppi_params_t *p = parport->params;

    p->fd = open (p->portname, O_RDWR);
    if (p->fd < 0)
    {
        urj_error_IO_set ("Cannot open(%s)", p->portname);
        return URJ_STATUS_FAIL;
    }

    return URJ_STATUS_OK;
}

static int
ppi_close (urj_parport_t *parport)
{
    int r = URJ_STATUS_OK;
    ppi_params_t *p = parport->params;

    if (close (p->fd) != 0)
    {
        urj_error_IO_set ("Cannot close(%d)", p->fd);
        return URJ_STATUS_FAIL;
    }

    p->fd = -1;
    return r;
}

static int
ppi_set_data (urj_parport_t *parport, unsigned char data)
{
    ppi_params_t *p = parport->params;

    uint8_t d = data;

    if (ioctl (p->fd, PPISDATA, &d) == -1)
    {
        urj_error_IO_set ("ioctl(PPISDATA) fails");
        return URJ_STATUS_FAIL;
    }

    return URJ_STATUS_OK;
}

static int
ppi_get_data (urj_parport_t *parport)
{
    unsigned char d;
    ppi_params_t *p = parport->params;

    if (ioctl (p->fd, PPIGDATA, &d) == -1)
    {
        urj_error_IO_set ("ioctl(PPIGDATA) fails");
        return -1;
    }

    return d;
}

static int
ppi_get_status (urj_parport_t *parport)
{
    unsigned char d;
    ppi_params_t *p = parport->params;

    if (ioctl (p->fd, PPIGSTATUS, &d) == -1)
    {
        urj_error_IO_set ("ioctl(PPIGSTATUS) fails");
        return -1;
    }

    return d ^ 0x80;            /* BUSY is inverted */
}

static int
ppi_set_control (urj_parport_t *parport, unsigned char data)
{
    ppi_params_t *p = parport->params;

    uint8_t d = data ^ 0x0B;    /* SELECT, AUTOFD, and STROBE are inverted */

    if (ioctl (p->fd, PPIGCTRL, &d) == -1)
    {
        urj_error_IO_set ("ioctl(PPIGCTRL) fails");
        return URJ_STATUS_FAIL;
    }

    return URJ_STATUS_OK;
}

const urj_parport_driver_t urj_tap_parport_ppi_parport_driver = {
    URJ_CABLE_PARPORT_DEV_PPI,
    ppi_connect,
    ppi_parport_free,
    ppi_open,
    ppi_close,
    ppi_set_data,
    ppi_get_data,
    ppi_get_status,
    ppi_set_control
};
