/*
 * $Id$
 *
 * Copyright (C) 2007, Arnim Laeuger
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
 * Written by Arnim Laeuger <arniml@users.sourceforge.net>, 2007.
 *
 */

#ifndef BSDL_H
#define BSDL_H

#include "chain.h"

extern int bsdl_debug;

int bsdl_read_file(chain_t *, const char *, int, char *);
void bsdl_set_path(const char *);
int bsdl_scan_files(chain_t *, const char *, int);

#endif /* BSDL_H */
