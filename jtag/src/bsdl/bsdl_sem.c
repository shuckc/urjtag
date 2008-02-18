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
 * This file contains semantic actions that are called by the bison
 * parser. They interface between the parser and the jtag application.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <jtag.h>
#include <cmd.h>

#include "bsdl_sysdep.h"

#include "bsdl_bison.h"
#include "bsdl.h"


/*****************************************************************************
 * void print_cmd(char **cmd)
 *
 * Prints the strings in array cmd until NULL is encountered.
 *
 * Parameters
 *   cmd : array of strings to print, terminated by NULL
 *
 * Returns
 *   void
 ****************************************************************************/
static void print_cmd(char **cmd)
{
  int   idx = 0;
  char *elem;

  while ((elem = cmd[idx])) {
    printf(" %s", elem);
    idx++;
  }
  printf("\n");
}


/*****************************************************************************
 * void bsdl_sem_init(parser_priv_t *priv)
 *
 * Performs initialization of data structures for the semantic action
 * routines.
 *
 * Parameters
 *   priv : private data container for parser related tasks
 *
 * Returns
 *   void
 ****************************************************************************/
void bsdl_sem_init(parser_priv_t *priv)
{
  struct access_data *ad = &(priv->jtag_ctrl.access_data);
  struct port_desc   *pd = &(priv->jtag_ctrl.port_desc);

  ad->ainfo_list = NULL;
  ad->instr_list = NULL;

  pd->names_list = NULL;

  priv->jtag_ctrl.instr_list = NULL;
}


/*****************************************************************************
 * void free_instr_list(struct instr_elem *il)
 *
 * Deallocates the given list of instr_elem.
 *
 * Parameters
 *   il : first instr_elem to deallocate
 *
 * Returns
 *   void
 ****************************************************************************/
static void free_instr_list(struct instr_elem *il)
{
  if (il) {
    if (il->instr)
      free(il->instr);
    if (il->opcode)
      free(il->opcode);
    free_instr_list(il->next);
    free(il);
  }
}


/*****************************************************************************
 * void free_ainfo_list(struct ainfo_elem *ai)
 *
 * Deallocates the given list of ainfo_elem.
 *
 * Parameters
 *  ai : first ainfo_elem to deallocate
 *
 * Returns
 *  void
 ****************************************************************************/
static void free_ainfo_list(struct ainfo_elem *ai)
{
  if (ai) {
    if (ai->reg)
      free(ai->reg);
    free_instr_list(ai->instr_list);
    free_ainfo_list(ai->next);
    free(ai);
  }
}


/*****************************************************************************
 * void free_string_list(struct string_elem *sl)
 *
 * Deallocates the given list of string_elem.
 *
 * Parameters
 *  sl : first string_elem to deallocate
 *
 * Returns
 *  void
 ****************************************************************************/
static void free_string_list(struct string_elem *sl)
{
  if (sl) {
    if (sl->string)
      free(sl->string);
    free_string_list(sl->next);
    free(sl);
  }
}


/*****************************************************************************
 * void bsdl_sem_deinit(parser_priv_t *priv)
 *
 * Performs deinitialization of data structures for the semantic action
 * routines.
 *
 * Parameters
 *   priv : private data container for parser related tasks
 *
 * Returns
 *   void
 ****************************************************************************/
void bsdl_sem_deinit(parser_priv_t *priv)
{
  struct access_data *ad = &(priv->jtag_ctrl.access_data);
  struct port_desc *pd = &(priv->jtag_ctrl.port_desc);

  if (ad->ainfo_list) {
    free_ainfo_list(ad->ainfo_list);
    ad->ainfo_list = NULL;
  }

  if (priv->jtag_ctrl.instr_list) {
    free_instr_list(priv->jtag_ctrl.instr_list);
    priv->jtag_ctrl.instr_list = NULL;
  }

  if (pd->names_list) {
    free_string_list(pd->names_list);
    pd->names_list = NULL;
  }
}


/*****************************************************************************
 * void bsdl_set_entity(parser_priv_t *priv, char *entityname)
 *
 * Applies the entity name from BSDL as the part name.
 *
 * Parameters
 *   priv       : private data container for parser related tasks
 *   entityname : entity name string, memory gets free'd
 *
 * Returns
 *   void
 ****************************************************************************/
void bsdl_set_entity(parser_priv_t *priv, char *entityname)
{
  if (priv->jtag_ctrl.mode >= 1) {
    strncpy(priv->jtag_ctrl.part->part, entityname, MAXLEN_PART);
    priv->jtag_ctrl.part->part[MAXLEN_PART] = '\0';
  }

  free(entityname);
}


/*****************************************************************************
 * void bsdl_set_instruction_length(parser_priv_t *priv, int len)
 *
 * Sets the specified length of the instruction register via shell command
 *   instruction length <len>
 *
 * Parameters
 *   priv : private data container for parser related tasks
 *   len  : number of bits (= length) of instruction register
 ****************************************************************************/
void bsdl_set_instruction_length(parser_priv_t *priv, int len)
{
  if (priv->jtag_ctrl.mode >= 0) {
    char lenstring[6];
    char *cmd[] = {"instruction",
                   "length",
                   lenstring,
                   NULL};

    snprintf(lenstring, 6, "%i", len);
    lenstring[5] = '\0';

    if (priv->jtag_ctrl.mode >= 1)
      cmd_run(cmd);
    else
      print_cmd(cmd);
  }
}


/*****************************************************************************
 * void bsdl_prt_add_name(parser_priv_t *priv, char *name)
 * Port name management function
 *
 * Sets the name field of the temporary storage area for port description
 * (port_desc) to the parameter name.
 *
 * Parameters
 *   priv : private data container for parser related tasks
 *   name : base name of the port, memory get's free'd lateron
 *
 * Returns
 *   void
 ****************************************************************************/
void bsdl_prt_add_name(parser_priv_t *priv, char *name)
{
  struct port_desc *pd = &(priv->jtag_ctrl.port_desc);
  struct string_elem *new_string;

  new_string = (struct string_elem *)malloc(sizeof(struct string_elem));
  if (new_string) {
    new_string->next   = pd->names_list;
    new_string->string = name;

    pd->names_list = new_string;
  } else
    bsdl_msg(BSDL_MSG_FATAL, _("Out of memory, %s line %i\n"), __FILE__, __LINE__);
}


/*****************************************************************************
 * void bsdl_prt_add_bit(parser_priv_t *priv)
 * Port name management function
 *
 * Sets the vector and index fields of the temporary storage area for port
 * description (port_desc) to non-vector information. The low and high indice
 * are set to equal numbers (exact value is irrelevant).
 *
 * Parameters
 *   priv : private data container for parser related tasks
 *
 * Returns
 *   void
 ****************************************************************************/
void bsdl_prt_add_bit(parser_priv_t *priv)
{
  struct port_desc *pd = &(priv->jtag_ctrl.port_desc);

  pd->is_vector = 0;
  pd->low_idx   = 0;
  pd->high_idx  = 0;
}


/*****************************************************************************
 * void bsdl_prt_add_range(parser_priv_t *priv, int low, int high)
 * Port name management function
 *
 * Sets the vector and index fields of the temporary storage area for port
 * description (port_desc) to the specified vector information.
 *
 * Parameters
 *   priv : private data container for parser related tasks
 *   low  : low index of vector
 *   high : high index of vector
 *
 * Returns
 *   void
 ****************************************************************************/
void bsdl_prt_add_range(parser_priv_t *priv, int low, int high)
{
  struct port_desc *pd = &(priv->jtag_ctrl.port_desc);

  pd->is_vector = 1;
  pd->low_idx   = low;
  pd->high_idx  = high;
}


/*****************************************************************************
 * void bsdl_prt_apply_port(parser_priv_t *priv)
 * Port name management function
 *
 * Adds the specified port name as a signal via shell command
 *   signal <pin>
 * The port name is taken from the port_desc structure that was filled in
 * previously by rule Scalar_or_Vector. This way, the function can build
 * vectored ports as well.
 * Keep in mind that multiple names can be defined by one port specification
 * (there's a names_list in port_desc).
 *
 * Parameters
 *   priv : private data container for parser related tasks
 *
 * Returns
 *   void
 ****************************************************************************/
void bsdl_prt_apply_port(parser_priv_t *priv)
{
  struct port_desc *pd = &(priv->jtag_ctrl.port_desc);

  if (priv->jtag_ctrl.mode >= 0) {
    char *cmd[] = {"signal",
                   NULL,
                   NULL};
    struct string_elem *name;
    size_t str_len, name_len;
    char *port_string;
    int idx;

    name = pd->names_list;
    while (name) {
      /* handle indexed port name:
         - names of scalar ports are simply copied from the port_desc structure
           to the final string that goes into ci
         - names of vectored ports are expanded with their decimal index as
           collected earlier in rule Scalar_or_Vector
      */
      name_len = strlen(name->string);
      str_len = name_len + 1 + 10 + 1 + 1;
      if ((port_string = (char *)malloc(str_len)) != NULL) {
        cmd[1] = port_string;

        for (idx = pd->low_idx; idx <= pd->high_idx; idx++) {
          if (pd->is_vector)
            snprintf(port_string, str_len-1, "%s(%d)", name->string, idx);
          else
            strncpy(port_string, name->string, str_len-1);
          port_string[str_len-1] = '\0';

          if (priv->jtag_ctrl.mode >= 1)
            cmd_run(cmd);
          else
            print_cmd(cmd);
        }

        free(port_string);
      } else
        bsdl_msg(BSDL_MSG_FATAL, _("Out of memory, %s line %i\n"), __FILE__, __LINE__);

      name = name->next;
    }
  }

  if (pd->names_list) {
    free_string_list(pd->names_list);
    pd->names_list = NULL;
  }
}


/*****************************************************************************
 * void create_register(parser_priv_t *priv, char *reg_name, size_t len)
 *
 * Generic function to create a jtag register via shell command
 *   register <reg_name> <len>
 *
 * Parameters
 *   priv     : private data container for parser related tasks
 *   reg_name : name of the new register
 *   len      : number of bits (= length) of new register
 *
 * Returns
 *   void
 ****************************************************************************/
static void create_register(parser_priv_t *priv, char *reg_name, size_t len)
{
  if (priv->jtag_ctrl.mode >= 0) {
    const size_t str_len = 10;
    char len_str[str_len+1];
    char *cmd[] = {"register",
                   reg_name,
                   len_str,
                   NULL};

    if (part_find_data_register(priv->jtag_ctrl.part, reg_name))
      return;

    /* convert length information to string */
    snprintf(len_str, str_len, "%i", len);

    if (priv->jtag_ctrl.mode >= 1)
      cmd_run(cmd);
    else
      print_cmd(cmd);
  }
}


/*****************************************************************************
 * void bsdl_set_idcode(parser_priv_t *priv, char *idcode)
 *
 * Stores the specified IDCODE of the device for later use and creates
 * the DIR register.
 *
 * Parameters
 *   priv   : private data container for parser related tasks
 *   idcode : string containing the device's IDCODE
 *
 * Returns
 *   void
 ****************************************************************************/
void bsdl_set_idcode(parser_priv_t *priv, char *idcode)
{
  priv->jtag_ctrl.idcode = idcode;

  create_register(priv, "DIR", strlen(idcode));
}


/*****************************************************************************
 * void bsdl_set_usercode(parser_priv_t *priv, char *usercode)
 *
 * Creates the USERCODE register, the contents of the usercode string is
 * ignored.
 *
 * Parameters
 *   priv     : private data container for parser related tasks
 *   usercode : string containing the device's USERCODE, memory gets free'd
 *
 * Returns
 *   void
 ****************************************************************************/
void bsdl_set_usercode(parser_priv_t *priv, char *usercode)
{
  create_register(priv, "USERCODE", strlen(usercode));

  /* we're not interested in the usercode value at all */
  free(usercode);
}


/*****************************************************************************
 * void bsdl_add_instruction(parser_priv_t *priv, char *instr, char *opcode)
 *
 * Converts the instruction specification into a member of the main
 * list of instructions at priv->jtag_ctrl.instr_list.
 *
 * Parameters
 *   priv   : private data container for parser related tasks
 *   instr  : instruction name
 *   opcode : instruction opcode
 *
 * Returns
 *   void
 ****************************************************************************/
void bsdl_add_instruction(parser_priv_t *priv, char *instr, char *opcode)
{
  struct instr_elem *new_instr;

  new_instr = (struct instr_elem *)malloc(sizeof(struct instr_elem));
  if (new_instr) {
    new_instr->next   = priv->jtag_ctrl.instr_list;
    new_instr->instr  = instr;
    new_instr->opcode = opcode;

    priv->jtag_ctrl.instr_list = new_instr;
  } else
    bsdl_msg(BSDL_MSG_FATAL, _("Out of memory, %s line %i\n"), __FILE__, __LINE__);
}


/*****************************************************************************
 * void bsdl_set_bsr_length(parser_priv_t *priv, int len)
 *
 * Creates the BSR register based on the specified length.
 *
 * Parameters
 *   priv : private data container for parser related tasks
 *   len  : number of bits (= length) of BSR register
 *
 * Returns
 *   void
 ****************************************************************************/
void bsdl_set_bsr_length(parser_priv_t *priv, int len)
{
  create_register(priv, "BSR", len);
}


/*****************************************************************************
 * Cell Info management function
 * void bsdl_ci_no_disable(parser_priv_t *priv)
 *
 * Tracks that there is no disable term for the current cell info.
 *
 * Parameters
 *   priv : private data container for parser related tasks
 *
 * Returns
 *   void
 ****************************************************************************/
void bsdl_ci_no_disable(parser_priv_t *priv)
{
  priv->jtag_ctrl.cell_info.ctrl_bit_num = -1;
}


/*****************************************************************************
 * Cell Info management function
 * void bsdl_ci_set_cell_spec(parser_priv_t *priv,
 *                            int function, char *safe_value)
 *
 * Sets the specified values of the current cell_spec (without disable term)
 * to the variables for temporary storage of these information elements.
 * The name of the related port is taken from the port_desc structure that
 * was filled in previously by the rule Port_Name.
 *
 * Parameters
 *   priv       : private data container for parser related tasks
 *   function   : cell function indentificator
 *   safe_value : safe value for initialization of this cell
 *
 * Returns
 *   void
 ****************************************************************************/
void bsdl_ci_set_cell_spec(parser_priv_t *priv,
                           int function, char *safe_value)
{
  struct cell_info *ci     = &(priv->jtag_ctrl.cell_info);
  struct port_desc *pd     = &(priv->jtag_ctrl.port_desc);
  struct string_elem *name = priv->jtag_ctrl.port_desc.names_list;
  char   *port_string;
  size_t  str_len, name_len;

  ci->cell_function    = function;
  ci->basic_safe_value = safe_value;

  /* handle indexed port name:
   - names of scalar ports are simply copied from the port_desc structure
     to the final string that goes into ci
   - names of vectored ports are expanded with their decimal index as
     collected earlier earlier in rule Port_Name
  */
  name_len = strlen(name->string);
  str_len = name_len + 1 + 10 + 1 + 1;
  if ((port_string = (char *)malloc(str_len)) != NULL) {
    if (pd->is_vector)
      snprintf(port_string, str_len-1, "%s(%d)", name->string, pd->low_idx);
    else
      strncpy(port_string, name->string, str_len-1);
    port_string[str_len-1] = '\0';

    ci->port_name = port_string;
  } else {
    bsdl_msg(BSDL_MSG_FATAL, _("Out of memory, %s line %i\n"), __FILE__, __LINE__);
    ci->port_name = NULL;
  }

  free(name->string);
  free(name);
  priv->jtag_ctrl.port_desc.names_list = NULL;
}


/*****************************************************************************
 * Cell Info management function
 * void bsdl_ci_set_cell_spec_disable(parser_priv_t *priv,
 *                                    int ctrl_bit_num, int safe_value, int disable_value)
 *
 * Applies the disable specification of the current cell spec to the variables
 * for temporary storage of these information elements.
 *
 * Parameters
 *   priv          : private data container for parser related tasks
 *   ctrl_bit_num  : bit number of related control cell
 *   safe_value    : safe value for initialization of this cell
 *   disable_value : currently ignored
 *
 * Returns
 *   void
 ****************************************************************************/
void bsdl_ci_set_cell_spec_disable(parser_priv_t *priv,
                                   int ctrl_bit_num, int safe_value, int disable_value)
{
  struct cell_info *ci = &(priv->jtag_ctrl.cell_info);

  ci->ctrl_bit_num       = ctrl_bit_num;
  ci->disable_safe_value = safe_value;
  /* disable value is ignored at the moment */
}


/*****************************************************************************
 * Cell Info management function
 * void bsdl_ci_apply_cell_info(parser_priv_t *priv, int bit_num)
 *
 * Creates a BSR cell from the temporary storage variables via shell command
 *   bit <bit_num> <type> <default> <signal> [<cbit> <cval> Z]
 *
 * Parameters
 *   priv    : private data container for parser related tasks
 *   bit_num : bit number of current cell
 *
 * Returns
 *   void
 ****************************************************************************/
void bsdl_ci_apply_cell_info(parser_priv_t *priv, int bit_num)
{
  struct cell_info *ci = &(priv->jtag_ctrl.cell_info);

  ci->bit_num = bit_num;

  if (priv->jtag_ctrl.mode >= 0) {
    const size_t str_len = 10;
    char bit_num_str[str_len+1];
    char ctrl_bit_num_str[str_len+1];
    char disable_safe_value_str[str_len+1];
    char *cmd[] = {"bit",
                   bit_num_str,
                   NULL,
                   NULL,
                   NULL,
                   ctrl_bit_num_str,
                   disable_safe_value_str,
                   "Z",
                   NULL};

    /* convert bit number to string */
    snprintf(bit_num_str, str_len, "%i", ci->bit_num);
    bit_num_str[str_len] = '\0';
    /* convert cell function from BSDL token to jtag syntax */
    switch (ci->cell_function) {
      case INTERNAL:
        /* fall through */
      case OUTPUT2:
        /* fall through */
      case OUTPUT3:
        cmd[2] = "O";
        break;
      case OBSERVE_ONLY:
        /* fall through */
      case INPUT:
        /* fall through */
      case CLOCK:
        cmd[2] = "I";
        break;
      case CONTROL:
        /* fall through */
      case CONTROLR:
        cmd[2] = "C";
        break;
      case BIDIR:
        cmd[2] = "B";
        break;
      default:
        /* spoil command */
        cmd[2] = "?";
        break;
    }
    /* convert basic safe value */
    cmd[3] = strcasecmp(ci->basic_safe_value, "x") == 0 ? "?" : ci->basic_safe_value;
    /* apply port name */
    cmd[4] = ci->port_name;

    /* add disable spec if present */
    if (ci->ctrl_bit_num >= 0) {
      /* convert bit number to string */
      snprintf(ctrl_bit_num_str, str_len, "%i", ci->ctrl_bit_num);
      ctrl_bit_num_str[str_len] = '\0';
      /* convert disable safe value to string */
      snprintf(disable_safe_value_str, str_len, "%i", ci->disable_safe_value);
      disable_safe_value_str[str_len] = '\0';
    } else
      /* stop command procssing here */
      cmd[5] = NULL;

    if (priv->jtag_ctrl.mode >= 1)
      cmd_run(cmd);
    else
      print_cmd(cmd);
  }

  /* free malloc'ed memory */
  if (ci->port_name) {
    free(ci->port_name);
    ci->port_name = NULL;
  }
  if (ci->basic_safe_value) {
    free(ci->basic_safe_value);
    ci->basic_safe_value = NULL;
  }
}


/*****************************************************************************
 * Register Access management function
 * void bsdl_ac_set_register(parser_priv_t *priv, char *reg, int reg_len)
 *
 * Stores the register specification values for the current register access
 * specification in the temporary storage region for later usage.
 *
 * Parameters
 *   priv    : private data container for parser related tasks
 *   reg     : register name
 *   reg_len : optional register length
 *
 * Returns
 *   void
 ****************************************************************************/
void bsdl_ac_set_register(parser_priv_t *priv, char *reg, int reg_len)
{
  struct access_data *ad = &(priv->jtag_ctrl.access_data);

  ad->reg     = reg;
  ad->reg_len = reg_len;
}


/*****************************************************************************
 * Register Access management function
 * void bsdl_ac_add_instruction(parser_priv_t *priv, char *instr)
 *
 * Appends the specified instruction to the list of instructions for the
 * current register access specification in the temporary storage region
 * for later usage.
 *
 * Parameters
 *   priv  : private data container for parser related tasks
 *   instr : instruction name
 *
 * Returns
 *   void
 ****************************************************************************/
void bsdl_ac_add_instruction(parser_priv_t *priv, char *instr)
{
  struct access_data *ad = &(priv->jtag_ctrl.access_data);
  struct instr_elem  *new_instr;

  new_instr = (struct instr_elem *)malloc(sizeof(struct instr_elem));
  if (new_instr) {
    new_instr->next   = ad->instr_list;
    new_instr->instr  = instr;
    new_instr->opcode = NULL;

    ad->instr_list = new_instr;
  } else
    bsdl_msg(BSDL_MSG_FATAL, _("Out of memory, %s line %i\n"), __FILE__, __LINE__);
}


/*****************************************************************************
 * Register Access management function
 * void bsdl_ac_apply_assoc(parser_priv_t *priv)
 *
 * Appends the collected register access specification from the temporary
 * storage region to the main ainfo list.
 *
 * Parameters
 *   priv : private data container for parser related tasks
 *
 * Returns
 *   void
 ****************************************************************************/
void bsdl_ac_apply_assoc(parser_priv_t *priv)
{
  struct access_data *ad = &(priv->jtag_ctrl.access_data);
  struct ainfo_elem *new_ainfo;

  new_ainfo = (struct ainfo_elem *)malloc(sizeof(struct ainfo_elem));
  if (new_ainfo) {
    new_ainfo->next       = ad->ainfo_list;
    new_ainfo->reg        = ad->reg;
    new_ainfo->reg_len    = ad->reg_len;
    new_ainfo->instr_list = ad->instr_list;

    ad->ainfo_list = new_ainfo;
  } else
    bsdl_msg(BSDL_MSG_FATAL, _("Out of memory, %s line %i\n"), __FILE__, __LINE__);

  /* clean up obsolete temporary entries */
  ad->reg        = NULL;
  ad->reg_len    = 0;
  ad->instr_list = NULL;
}


/*****************************************************************************
 * Register Access management function
 * void bsdl_ac_finalize(parser_priv_t *priv)
 *
 * Runs through the main instruction list and builds the instruction/register
 * association for each instruction from the register access specifications
 * via shell command
 *   instruction <instruction> <code> <register>
 *
 * Additional register are created on the fly:
 *   - standard registers that haven't been created so far
 *   - non-standard registers encountered in register access specs
 *
 * Mandatory instruction/register associations are generated also in
 * absence of a related register access specification (such specs are
 * optional in the BSDL standard).
 *
 * Parameters
 *   priv : private data container for parser related tasks
 *
 * Returns
 *   void
 ****************************************************************************/
void bsdl_ac_finalize(parser_priv_t *priv)
{
  /* ensure that all mandatory registers are created prior to
     handling the instruction/register associations
     + BOUNDARY/BSR has been generated during the parsing process
     + DEVICE_ID/DIR has been generated during the parsing process
  */
  /* we need a BYPASS register */
  create_register(priv, "BYPASS", 1);

  if (priv->jtag_ctrl.mode >= 0) {
    struct ainfo_elem *ai;
    struct instr_elem *cinst;

    /* next scan through all register_access definitions and create
       the non-standard registers */
    ai = priv->jtag_ctrl.access_data.ainfo_list;
    while (ai) {
      int is_std = 0;

      if (strcasecmp(ai->reg, "BOUNDARY" ) == 0) is_std = 1;
      if (strcasecmp(ai->reg, "BYPASS"   ) == 0) is_std = 1;
      if (strcasecmp(ai->reg, "DEVICE_ID") == 0) is_std = 1;
      if (strcasecmp(ai->reg, "USERCODE" ) == 0) is_std = 1;

      if (!is_std)
        create_register(priv, ai->reg, ai->reg_len);

      ai = ai->next;
    }


    /* next scan through all instruction/opcode definitions and resolve
       the instruction/register associations for these */
    cinst = priv->jtag_ctrl.instr_list;
    while (cinst) {
      char *reg_name = NULL;
      char *instr_name = NULL;

      /* now see which of the register_access elements matches this instruction */
      ai = priv->jtag_ctrl.access_data.ainfo_list;
      while (ai && (reg_name == NULL)) {
        struct instr_elem *tinst = ai->instr_list;

        while (tinst && (reg_name == NULL)) {
          if (strcasecmp(tinst->instr, cinst->instr) == 0) {
            /* found the instruction inside the current access info,
               now set the register name
               map some standard register names to different internal names*/
            if (strcasecmp(ai->reg, "BOUNDARY") == 0) reg_name = "BSR";
            else if (strcasecmp(ai->reg, "DEVICE_ID") == 0) reg_name = "DIR";
            else reg_name = ai->reg;
          }

          tinst = tinst->next;
        }

        ai = ai->next;
      }

      if (reg_name == NULL) {
        /* BSDL file didn't specify an explicit register_access definition
           if we're looking at a standard mandatory instruction, we should
           build the association ourselves */
        if      (strcasecmp(cinst->instr, "BYPASS"  ) == 0) reg_name = "BYPASS";
        else if (strcasecmp(cinst->instr, "CLAMP"   ) == 0) reg_name = "BYPASS";
        else if (strcasecmp(cinst->instr, "EXTEST"  ) == 0) reg_name = "BSR";
        else if (strcasecmp(cinst->instr, "HIGHZ"   ) == 0) reg_name = "BYPASS";
        else if (strcasecmp(cinst->instr, "IDCODE"  ) == 0) reg_name = "DIR";
        else if (strcasecmp(cinst->instr, "INTEST"  ) == 0) reg_name = "BSR";
        else if (strcasecmp(cinst->instr, "PRELOAD" ) == 0) reg_name = "BSR";
        else if (strcasecmp(cinst->instr, "SAMPLE"  ) == 0) reg_name = "BSR";
        else if (strcasecmp(cinst->instr, "USERCODE") == 0) reg_name = "USERCODE";
      }

      if (strcasecmp(cinst->instr, "SAMPLE" ) == 0)
        instr_name = "SAMPLE/PRELOAD";
      else
        instr_name = cinst->instr;

      if (reg_name) {
        char *cmd[] = {"instruction",
                       instr_name,
                       cinst->opcode,
                       reg_name,
                       NULL};

        if (priv->jtag_ctrl.mode >= 1)
          cmd_run(cmd);
        else
          print_cmd(cmd);
      }

      cinst = cinst->next;
    }
  }
}
