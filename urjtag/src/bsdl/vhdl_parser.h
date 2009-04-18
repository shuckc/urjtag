/*
 * $Id$
 *
 * Copyright (C) 2008, Arnim Laeuger
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
 * Written by Arnim Laeuger <arniml@users.sourceforge.net>, 2008.
 *
 */

#ifndef VHDL_PARSER_H
#define VHDL_PARSER_H

#include "bsdl_types.h"

/* VHDL lexer declarations */
void *vhdl_flex_init( FILE *, int );
void  vhdl_flex_deinit( void * );
void  vhdl_flex_switch_file( void *, char * );
int   vhdl_flex_get_compile_errors( void * );
int   vhdl_flex_postinc_compile_errors( void * );
int   vhdl_flex_get_lineno( void * );

/* VHDL parser declarations */
vhdl_parser_priv_t *vhdl_parser_init( FILE *, jtag_ctrl_t * );
void vhdl_parser_deinit( vhdl_parser_priv_t * );
int vhdlparse( vhdl_parser_priv_t * );

#endif /* VHDL_PARSER_H */
