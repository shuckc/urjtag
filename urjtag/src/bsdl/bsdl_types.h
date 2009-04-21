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

#ifndef BSDL_TYPES_H
#define BSDL_TYPES_H

#include <jtag.h>
#include <bsdl_mode.h>

/* private data of the flex scanner
   handled internally in bsdl_flex.l as yyextra */
struct scan_extra
{
    int proc_mode;
    int Compile_Errors;
    int Base;
};
typedef struct scan_extra scan_extra_t;

/* list of instructions
   the instruction name and its opcode (optional) is stored here */
struct instr_elem
{
    struct instr_elem *next;
    char *instr;
    char *opcode;
};
typedef struct instr_elem instr_elem_t;

/* register access information
 * derived from the entries of the REGISTER_ACCESS attribute
 * ainfo_elem describes a register and its accosiated instructions
 * - register name
 * - register length (optional)
 * - list of associated instructions
 */
struct ainfo_elem
{
    struct ainfo_elem *next;
    char *reg;
    int reg_len;
    instr_elem_t *instr_list;
};
typedef struct ainfo_elem ainfo_elem_t;

/* structure cell_info collects bit/cell information from the
   BOUNDARY_REGISTER attribute
   each Cell_Entry fills in the structure and the contents is used for the
   respective 'bit' command */
struct cell_info
{
    struct cell_info *next;
    /* basic cell spec entries */
    int bit_num;
    char *port_name;
    int cell_function;
    char *basic_safe_value;
    /* the disable spec entries */
    int ctrl_bit_num;
    int disable_safe_value;
};
typedef struct cell_info cell_info_t;

/* structure string_elem enables to build lists of strings */
struct string_elem
{
    struct string_elem *next;
    char *string;
};
typedef struct string_elem string_elem_t;

/* structure port_desc contains all descriptive information for a port
   definition:
   - one or more names
   - flag showing whether it's a vector (element) or a scalar
   - low and high indice if it's a vector */
struct port_desc
{
    string_elem_t *names_list;
    struct port_desc *next;
    int is_vector;
    int low_idx;
    int high_idx;
};
typedef struct port_desc port_desc_t;

typedef enum
{
    VET_CONSTANT,
    VET_ATTRIBUTE_STRING,
    VET_ATTRIBUTE_DECIMAL,
    VET_UNKNOWN
} vhdl_elem_type_t;

struct vhdl_elem
{
    struct vhdl_elem *next;
    vhdl_elem_type_t type;
    char *name;
    char *payload;
    int line;
};
typedef struct vhdl_elem vhdl_elem_t;

typedef enum
{
    CONF_1990,
    CONF_1993,
    CONF_2001,
    CONF_UNKNOWN
} bsdl_conformance_t;

/* structure jtag_ctrl collects all elements that are required to interface
   with jtag internals */
struct jtag_ctrl
{
    int proc_mode;
    chain_t *chain;
    part_t *part;
    /* collected by VHDL parser */
    port_desc_t *port_desc;
    vhdl_elem_t *vhdl_elem_first;
    vhdl_elem_t *vhdl_elem_last;
    /* collected by BSDL parser */
    char *idcode;               /* IDCODE string */
    char *usercode;             /* USERCODE string */
    int instr_len;
    int bsr_len;
    bsdl_conformance_t conformance;
    instr_elem_t *instr_list;
    ainfo_elem_t *ainfo_list;
    cell_info_t *cell_info_first;
    cell_info_t *cell_info_last;
};
typedef struct jtag_ctrl jtag_ctrl_t;

/* private data of the VHDL bison parser
   used to store variables the would end up as globals otherwise */
struct vhdl_parser_priv
{
    char Package_File_Name[100];
    int Reading_Package;
    char *buffer;
    size_t len_buffer;
    void *scanner;
    jtag_ctrl_t *jtag_ctrl;
    port_desc_t tmp_port_desc;
};
typedef struct vhdl_parser_priv vhdl_parser_priv_t;

/* private data of the BSDL bison parser
   used to store variables the would end up as globals otherwise */
struct bsdl_parser_priv
{
    void *scanner;
    jtag_ctrl_t *jtag_ctrl;
    int lineno;
    ainfo_elem_t ainfo;
    cell_info_t tmp_cell_info;
    port_desc_t tmp_port_desc;
};
typedef struct bsdl_parser_priv bsdl_parser_priv_t;

#endif /* BSDL_TYPES_H */


/*
 Local Variables:
 mode:C
 c-default-style:gnu
 indent-tabs-mode:nil
 End:
*/
