/*
 * $Id$
 *
 * Copyright (C) 2002 ETC s.r.o.
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
 * Written by Marcel Telka <marcel@telka.sk>, 2002.
 *
 */

#ifndef INSTRUCTION_H
#define	INSTRUCTION_H

#include "register.h"
#include "data_register.h"

#define	MAXLEN_INSTRUCTION	20

typedef struct instruction instruction;

struct instruction {
	char name[MAXLEN_INSTRUCTION + 1];
	tap_register *value;
	tap_register *out;
	data_register *data_register;
	instruction *next;
};

instruction *instruction_alloc( const char *name, int len, const char *val );
void instruction_free( instruction *i );

#endif /* INSTRUCTION_H */
