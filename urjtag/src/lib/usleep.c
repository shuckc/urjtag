/*
 * $Id: usleep.c 1519 2009-04-22 23:12:44Z rfhh $
 *
 * Copyright (C) 2009, Rutger Hofman, VU Amsterdam
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

#ifndef HAVE_USLEEP

#ifndef HAVE_NANOSLEEP
#  error Need usleep or nanosleep
#endif

#include <time.h>

#define MICRO   1000000L
#define NANO    1000000000L

int usleep (long unsigned usec)
{
    struct timespec req;

    req.tv_sec = usec / MICRO;
    req.tv_nsec = (usec % MICRO) * (NANO / MICRO);

    return nanosleep (&req, NULL);
}

#endif
