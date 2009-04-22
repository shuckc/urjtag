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

#ifndef URJ_BSDL_PARSER_H
#define URJ_BSDL_PARSER_H

#include "bsdl_types.h"

/* VHDL lexer declarations */
void *urj_bsdl_flex_init (int);
void urj_bsdl_flex_deinit (void *);
void urj_bsdl_flex_set_bin_x (void *);
void urj_bsdl_flex_set_hex (void *);
void urj_bsdl_flex_set_decimal (void *);
int urj_bsdl_flex_get_compile_errors (void *);
int urj_bsdl_flex_postinc_compile_errors (void *);
void urj_bsdl_flex_switch_buffer (void *, const char *, int);
void urj_bsdl_flex_stop_buffer (void *);

/* BSDL parser declarations */
urj_bsdl_parser_priv_t *urj_bsdl_parser_init (urj_bsdl_jtag_ctrl_t *);
void urj_bsdl_parser_deinit (urj_bsdl_parser_priv_t *);
int urj_bsdl_parse (urj_bsdl_parser_priv_t *);

/* BSDL semantic functions */
int urj_bsdl_process_elements (urj_bsdl_jtag_ctrl_t *, const char *);

#endif /* URJ_BSDL_PARSER_H */
