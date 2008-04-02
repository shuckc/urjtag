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

#ifndef BSDL_LOCAL_H
#define BSDL_LOCAL_H

#include <jtag.h>


/* message types for bsdl_msg() */
#define BSDL_MSG_NOTE  0
#define BSDL_MSG_WARN  1
#define BSDL_MSG_ERR   2
#define BSDL_MSG_FATAL 3


/* private data of the flex scanner
   handled internally in bsdl_flex.l as yyextra */
struct scan_extra {
    int mode;
    int debug;
    int Compile_Errors;
    int Base;
};
typedef struct scan_extra scan_extra_t;


/* list of instructions
   the instruction name and its opcode (optional) is stored here */
struct instr_elem {
    struct instr_elem *next;
    char *instr;
    char *opcode;
};

/* register access information
 * derived from the entries of the REGISTER_ACCESS attribute
 * ainfo_elem describes a register and its accosiated instructions
 * - register name
 * - register length (optional)
 * - list of associated instructions
 */
struct ainfo_elem {
    struct ainfo_elem *next;
    char *reg;
    int   reg_len;
    struct instr_elem *instr_list;
};

/* the access_data structure is the entry point into the register access
   management data
   it contains the pointer to the main list of REGISTER_ACCESS entries plus
   components that store temporary data while an entry is parser/built */
struct access_data {
    struct ainfo_elem *ainfo_list;
    /* temporary private data for building the "current" access info entry */
    char  *reg;
    int    reg_len;
    struct instr_elem *instr_list;
};

/* structure cell_info collects bit/cell information from the
   BOUNDARY_REGISTER attribute
   each Cell_Entry fills in the structure and the contents is used for the
   respective 'bit' command */
struct cell_info {
    /* basic cell spec entries */
    int   bit_num;
    char *port_name;
    int   cell_function;
    char *basic_safe_value;
    /* the disable spec entries */
    int   ctrl_bit_num;
    int   disable_safe_value;
};

/* structure string_elem enables to build lists of strings */
struct string_elem {
    struct string_elem *next;
    char *string;
};

/* structure port_desc contains all descriptive information for a port
   definition:
   - one or more names
   - flag showing whether it's a vector (element) or a scalar
   - low and high indice if it's a vector */
struct port_desc {
    struct string_elem *names_list;
    int is_vector;
    int low_idx;
    int high_idx;
};

/* structure jtag_ctrl collects all elements that are required to interface
   with jtag internals */
struct jtag_ctrl {
    int     mode;
    int     debug;
    char   *idcode;       /* IDCODE string */
    chain_t *chain;
    part_t *part;
    struct  port_desc port_desc;
    struct  cell_info cell_info;
    struct  instr_elem *instr_list;
    struct  access_data access_data;
};

/* private data of the bison parser
   used to store variables the would end up as globals otherwise */
struct parser_priv {
    char    Package_File_Name[100];
    int     Reading_Package;
    char   *buffer_for_switch;
    size_t  len_buffer_for_switch;
    void   *scanner;
    struct  jtag_ctrl jtag_ctrl;
};
typedef struct parser_priv parser_priv_t;


void bsdl_msg(int, const char *, ...);

/* BSDL lexer declarations */
void *bsdl_flex_init(FILE *, int, int);
void  bsdl_flex_deinit(void *);
void  bsdl_flex_switch_file(void *, char *);
void  bsdl_flex_switch_buffer(void *, const char *);
int   bsdl_flex_get_compile_errors(void *);
int   bsdl_flex_postinc_compile_errors(void *);
int   bsdl_flex_get_lineno(void *);
void  bsdl_flex_set_bin_x(void *);

/* BSDL parser declarations */
parser_priv_t *bsdl_parser_init(FILE *, int, int);
void bsdl_parser_deinit(parser_priv_t *);
int bsdlparse(parser_priv_t *);

/* BSDL / JTAG semantic action interface */
void bsdl_sem_init(parser_priv_t *);
void bsdl_sem_deinit(parser_priv_t *);
void bsdl_set_entity(parser_priv_t *, char *);
void bsdl_set_instruction_length(parser_priv_t *, int);
void bsdl_prt_add_name(parser_priv_t *, char *);
void bsdl_prt_add_bit(parser_priv_t *);
void bsdl_prt_add_range(parser_priv_t *, int, int);
void bsdl_prt_apply_port(parser_priv_t *);
void bsdl_set_idcode(parser_priv_t *, char *);
void bsdl_set_usercode(parser_priv_t *, char *);
void bsdl_add_instruction(parser_priv_t *, char *, char *);
void bsdl_set_bsr_length(parser_priv_t *, int);
void bsdl_ci_no_disable(parser_priv_t *);
void bsdl_ci_set_cell_spec(parser_priv_t *, int, char *);
void bsdl_ci_set_cell_spec_disable(parser_priv_t *, int, int, int);
void bsdl_ci_apply_cell_info(parser_priv_t *, int);
void bsdl_ac_set_register(parser_priv_t *, char *, int);
void bsdl_ac_add_instruction(parser_priv_t *, char *);
void bsdl_ac_apply_assoc(parser_priv_t *);
void bsdl_ac_finalize(parser_priv_t *);

#endif /* BSDL_LOCAL_H */
