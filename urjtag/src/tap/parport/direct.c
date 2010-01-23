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

#include <sysdep.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <urjtag/error.h>
#include <urjtag/log.h>
#include <urjtag/parport.h>
#include <urjtag/cable.h>
#include "../parport.h"

#if defined(HAVE_INPOUTXX)

static HINSTANCE inpout32_dll_handle = NULL;

typedef short _stdcall (*inpfuncPtr) (short p);
typedef void _stdcall (*outfuncPtr) (short p, short d);

static inpfuncPtr Inp32;
static outfuncPtr Out32;

#define inb(p) (Inp32)(p)
#define outb(d,p) (Out32)(p,d)

#elif defined(HAVE_IOPERM)

#include <sys/io.h>

#elif defined(HAVE_I386_SET_IOPERM)

#include <sys/types.h>
#include <machine/sysarch.h>
#include <err.h>

static __inline int
ioperm (unsigned long from, unsigned long num, int permit)
{
#ifdef __FreeBSD__
    if (i386_set_ioperm (from, num, permit) == -1)
        return -1;
#else
    u_long ports[32];
    u_long i;

    if (i386_get_ioperm (ports) == -1)
        return -1;

    for (i = from; i < (from + num); i++)
        if (permit)
            ports[i / 32] &= ~(1 << (i % 32));
        else
            ports[i / 32] |= (1 << (i % 32));

    if (i386_set_ioperm (ports) == -1)
        return -1;
#endif
    return 0;
}

static __inline int
iopl (int level)
{
#ifndef __FreeBSD__
    return i386_iopl (level);
#endif
    return 0;
}

static __inline unsigned char
inb (unsigned short int port)
{
    unsigned char _v;

    __asm__ __volatile__ ("inb %w1,%0":"=a" (_v):"Nd" (port));
    return _v;
}

static __inline void
outb (unsigned char value, unsigned short int port)
{
    __asm__ __volatile__ ("outb %b0,%w1"::"a" (value), "Nd" (port));
}
#endif /* HAVE_I386_SET_IOPERM */

static port_node_t *ports = NULL;       /* direct parallel ports */

typedef struct
{
    unsigned int port;
} direct_params_t;

static urj_parport_t *
direct_parport_alloc (unsigned int port)
{
    direct_params_t *params = malloc (sizeof *params);
    urj_parport_t *parport = malloc (sizeof *parport);
    port_node_t *node = malloc (sizeof *node);

    if (!node || !parport || !params)
    {
        free (node);
        free (parport);
        free (params);
        urj_error_set (URJ_ERROR_OUT_OF_MEMORY,
                       "malloc(%zd)/malloc(%zd)/malloc(%zd) fails",
                       sizeof *params, sizeof *parport, sizeof *node);
        return NULL;
    }

#if defined(HAVE_INPOUTXX)
    if (inpout32_dll_handle == NULL)
    {
        inpout32_dll_handle = LoadLibrary ("inpout32.dll");
    }
    if (inpout32_dll_handle == NULL)
    {
        urj_error_set (URJ_ERROR_IO,
                       _("Couldn't load InpOut32.dll; maybe not installed?"));
        urj_error_state.sys_errno = GetLastError();
        return NULL;
    }

    Inp32 = (inpfuncPtr) GetProcAddress (inpout32_dll_handle, "Inp32");
    Out32 = (outfuncPtr) GetProcAddress (inpout32_dll_handle, "Out32");
#endif

    params->port = port;

    parport->params = params;
    parport->driver = &urj_tap_parport_direct_parport_driver;
    parport->cable = NULL;

    node->port = parport;
    node->next = ports;

    ports = node;

    return parport;
}

static void
direct_parport_free (urj_parport_t *port)
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

    free (port->params);
    free (port);

#if defined(HAVE_INPOUTXX)
    if (inpout32_dll_handle != NULL)
        FreeLibrary (inpout32_dll_handle);
#endif
}

static urj_parport_t *
direct_connect (const char *devname)
{
    long int port_scan_val;
    unsigned int port;
    port_node_t *pn = ports;
    urj_parport_t *parport;

    errno = 0;
    port_scan_val = strtol (devname, NULL, 0);
    if (errno != 0)
    {
        urj_error_IO_set ("strtol(%s) fails", devname);
        return NULL;
    }

    if (port_scan_val < 0 || (port_scan_val + 3) > 0xffff)
    {
        urj_error_set (URJ_ERROR_INVALID,  _("Invalid port address"));
        return NULL;
    }

    port = (unsigned int) port_scan_val;

    while (pn)
        for (pn = ports; pn; pn = pn->next)
        {
            unsigned int aport;

            aport = ((direct_params_t *) pn->port->params)->port;
            if (abs (aport - port) < 3)
            {
                urj_log (URJ_LOG_LEVEL_NORMAL,
                         _("Disconnecting %s from parallel port at 0x%x\n"),
                         _(pn->port->cable->driver->description), aport);
                pn->port->cable->driver->disconnect (pn->port->cable);
                break;
            }
        }

    urj_log (URJ_LOG_LEVEL_NORMAL, _("Initializing parallel port at 0x%x\n"),
             port);

    parport = direct_parport_alloc (port);
    if (!parport)
        return NULL;

    return parport;
}

static int
direct_open (urj_parport_t *parport)
{
#ifdef HAVE_INPOUTXX
    return URJ_STATUS_OK;
#else
    unsigned int port = ((direct_params_t *) parport->params)->port;
    if (((port + 3 <= 0x400) ? ioperm (port, 3, 1) : iopl (3)) == -1)
    {
        urj_error_IO_set ("ioperm(3,1) or iopl(3) fails");
        return URJ_STATUS_FAIL;
    }
    return URJ_STATUS_OK;
#endif
}

static int
direct_close (urj_parport_t *parport)
{
#if defined(HAVE_INPOUTXX)
    return URJ_STATUS_OK;
#else
    unsigned int port = ((direct_params_t *) parport->params)->port;
    if (((port + 3 <= 0x400) ? ioperm (port, 3, 0) : iopl (0)) == -1)
    {
        urj_error_IO_set ("ioperm(3,0) or iopl(0) fails");
        return URJ_STATUS_FAIL;
    }
    return URJ_STATUS_OK;
#endif
}

static int
direct_set_data (urj_parport_t *parport, unsigned char data)
{
    unsigned short int port = ((direct_params_t *) parport->params)->port;
    outb (data, port);
    return URJ_STATUS_OK;
}

static int
direct_get_data (urj_parport_t *parport)
{
    unsigned int port = ((direct_params_t *) parport->params)->port;
    return inb (port);
}

static int
direct_get_status (urj_parport_t *parport)
{
    unsigned int port = ((direct_params_t *) parport->params)->port;
    return inb (port + 1) ^ 0x80;       /* BUSY is inverted */
}

static int
direct_set_control (urj_parport_t *parport, unsigned char data)
{
    unsigned short int port = ((direct_params_t *) parport->params)->port;
    outb (data ^ 0x0B, port + 2);       /* SELECT, AUTOFD, and STROBE are inverted */
    return URJ_STATUS_OK;
}

const urj_parport_driver_t urj_tap_parport_direct_parport_driver = {
    URJ_CABLE_PARPORT_DEV_PARALLEL,
    direct_connect,
    direct_parport_free,
    direct_open,
    direct_close,
    direct_set_data,
    direct_get_data,
    direct_get_status,
    direct_set_control
};
