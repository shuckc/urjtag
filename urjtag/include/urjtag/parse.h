/*
 * $Id: parse.h 1544 2009-05-01 12:55:19Z rfhh $
 *
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

#ifndef URJ_PARSE_H
#define URJ_PARSE_H

#include <stdio.h>

#include "types.h"

int urj_parse_file (urj_chain_t *chain, const char *filename);
int urj_parse_line (urj_chain_t *chain, char *line);
int urj_parse_stream (urj_chain_t *chain, FILE * f);

#endif /* URJ_PARSE_H */

