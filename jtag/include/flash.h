/*
 * $Id$
 *
 * Flash Memory interface
 * Copyright (C) 2003 AH
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
 * Written by August Hörandl <august.hoerandl@gmx.at>
 *
 */

#ifndef FLASH_H
#define	FLASH_H

#include <brux/flash.h>

extern flash_driver_t *flash_driver;
extern flash_driver_t *flash_drivers[];

#define	CFI_INTEL_ERROR_UNKNOWN				1
#define	CFI_INTEL_ERROR_UNSUPPORTED			2
#define	CFI_INTEL_ERROR_LOW_VPEN			3
#define	CFI_INTEL_ERROR_BLOCK_LOCKED			4
#define	CFI_INTEL_ERROR_INVALID_COMMAND_SEQUENCE	5

#endif /* FLASH_H */
