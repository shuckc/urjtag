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

#ifndef BSBIT_H
#define	BSBIT_H

typedef struct bsbit bsbit;

#include <signal.h>

#define	BSBIT_INPUT	1
#define	BSBIT_OUTPUT	2
#define	BSBIT_CONTROL	3
#define	BSBIT_INTERNAL	4

#define	BSBIT_STATE_Z	(-1)

struct bsbit {
	int bit;
	char *name;
	int type;
	signal *signal;
	int safe;		/* safe value */
	int control;		/* -1 for none */
	int control_value;
	int control_state;
};

bsbit *bsbit_alloc( int bit, const char *name, int type, signal* signals, int safe );
void bsbit_free( bsbit *b );

#endif /* BSBIT_H */
