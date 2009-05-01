/*
 * $Id: urjtag.c 1539 2009-05-01 12:02:08Z rfhh $
 *
 * Copyright (C) 2009 Rutger Hofman, VU Amsterdam
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
 * Written by Rutger Hofman
 */

#include "sysdep.h"

#include <urjtag/error.h>
#include <urjtag/jtag.h>

urj_error_state_t urj_error_state;
int urj_debug_mode = 0;
int urj_big_endian = 0;
