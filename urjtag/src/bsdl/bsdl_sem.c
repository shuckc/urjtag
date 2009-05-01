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
 * Written by Arnim Laeuger <arniml@users.sourceforge.net>, 2007.
 *
 * This file contains semantic actions that are called by the bison
 * parser. They interface between the parser and the jtag application.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <urjtag/jtag.h>
#include <urjtag/cmd.h>

#include "bsdl_sysdep.h"

#include "bsdl_types.h"
#include "bsdl_msg.h"

#include "bsdl_parser.h"
#include "bsdl_bison.h"

#ifdef DMALLOC
#include "dmalloc.h"
#endif

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
static void
print_cmd (char **cmd)
{
    int idx = 0;
    char *elem;

    while ((elem = cmd[idx]))
    {
        printf ("%s%s", idx > 0 ? " " : "", elem);
        idx++;
    }
    printf ("\n");
}


/*****************************************************************************
 * int urj_bsdl_set_instruction_length( urj_bsdl_jtag_ctrl_t *jc )
 *
 * Sets the specified length of the instruction register via shell command
 *   instruction length <len>
 *
 * Parameters
 *   jc : jtag control structure
 *
 * Returns
 *   1 -> all ok
 *   0 -> error occured
 ****************************************************************************/
static int
urj_bsdl_set_instruction_length (urj_bsdl_jtag_ctrl_t *jc)
{
    char lenstring[6];
    char *cmd[] = { "instruction",
        "length",
        lenstring,
        NULL
    };

    snprintf (lenstring, 6, "%i", jc->instr_len);
    lenstring[5] = '\0';

    if (jc->proc_mode & URJ_BSDL_MODE_INSTR_EXEC)
        urj_cmd_run (jc->chain, cmd);
    if (jc->proc_mode & URJ_BSDL_MODE_INSTR_PRINT)
        print_cmd (cmd);

    return 1;
}


/*****************************************************************************
 * int urj_bsdl_emit_ports( urj_bsdl_jtag_ctrl_t *jc )
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
 *   jc : jtag control structure
 *
 * Returns
 *   1 -> all ok
 *   0 -> error occured
 ****************************************************************************/
static int
urj_bsdl_emit_ports (urj_bsdl_jtag_ctrl_t *jc)
{
    urj_bsdl_port_desc_t *pd = jc->port_desc;
    struct string_elem *name;
    size_t str_len, name_len;
    char *port_string;
    int idx;
    int result = 0;
    char *cmd[] = { "signal",
        NULL,
        NULL
    };

    while (pd)
    {
        name = pd->names_list;
        while (name)
        {
            /* handle indexed port name:
               - names of scalar ports are simply copied from the port_desc structure
               to the final string that goes into ci
               - names of vectored ports are expanded with their decimal index as
               collected earlier in rule Scalar_or_Vector
             */
            name_len = strlen (name->string);
            str_len = name_len + 1 + 10 + 1 + 1;
            if ((port_string = malloc (str_len)) != NULL)
            {
                cmd[1] = port_string;

                for (idx = pd->low_idx; idx <= pd->high_idx; idx++)
                {
                    if (pd->is_vector)
                        snprintf (port_string, str_len - 1, "%s(%d)",
                                  name->string, idx);
                    else
                        strncpy (port_string, name->string, str_len - 1);
                    port_string[str_len - 1] = '\0';

                    if (jc->proc_mode & URJ_BSDL_MODE_INSTR_EXEC)
                        urj_cmd_run (jc->chain, cmd);
                    if (jc->proc_mode & URJ_BSDL_MODE_INSTR_PRINT)
                        print_cmd (cmd);
                }

                free (port_string);
                result = 1;
            }
            else
                urj_bsdl_msg (jc->proc_mode,
                              BSDL_MSG_FATAL,
                              _("Out of memory, %s line %i\n"), __FILE__,
                              __LINE__);

            name = name->next;
        }

        pd = pd->next;
    }

    return result;
}


/*****************************************************************************
 * int create_register( urj_bsdl_jtag_ctrl_t *jc, char *reg_name, size_t len )
 *
 * Generic function to create a jtag register via shell command
 *   register <reg_name> <len>
 *
 * Parameters
 *   jc       : jtag control structure
 *   reg_name : name of the new register
 *   len      : number of bits (= length) of new register
 *
 * Returns
 *   1 -> all ok
 *   0 -> error occured
 ****************************************************************************/
static int
create_register (urj_bsdl_jtag_ctrl_t *jc, char *reg_name, size_t len)
{
    const size_t str_len = 10;
    char len_str[str_len + 1];
    char *cmd[] = { "register",
        reg_name,
        len_str,
        NULL
    };

    if (urj_part_find_data_register (jc->part, reg_name))
        return 1;

    /* convert length information to string */
    snprintf (len_str, str_len, "%zu", len);

    if (jc->proc_mode & URJ_BSDL_MODE_INSTR_EXEC)
        urj_cmd_run (jc->chain, cmd);
    if (jc->proc_mode & URJ_BSDL_MODE_INSTR_PRINT)
        print_cmd (cmd);

    return 1;
}


/*****************************************************************************
 * int urj_bsdl_process_idcode( urj_bsdl_jtag_ctrl_t *jc )
 *
 * Creates the DIR register based on the extracted idcode.
 *
 * Parameters
 *   jc : jtag control structure
 *
 * Returns
 *   1 -> all ok
 *   0 -> error occured
 ****************************************************************************/
static int
urj_bsdl_process_idcode (urj_bsdl_jtag_ctrl_t *jc)
{
    if (jc->idcode)
        create_register (jc, "DIR", strlen (jc->idcode));
    else
        urj_bsdl_msg (jc->proc_mode,
                      BSDL_MSG_WARN, _("No IDCODE specification found.\n"));

    return 1;
}


/*****************************************************************************
 * int urj_bsdl_process_usercode( urj_bsdl_jtag_ctrl_t *jc )
 *
 * Creates the USERCODE register, the contents of the usercode string is
 * ignored.
 *
 * Parameters
 *   jc : jtag control structure
 *
 * Returns
 *   1 -> all ok
 *   0 -> error occured
 ****************************************************************************/
static int
urj_bsdl_process_usercode (urj_bsdl_jtag_ctrl_t *jc)
{
    if (jc->usercode)
        create_register (jc, "USERCODE", strlen (jc->usercode));

    /* we're not interested in the usercode value at all */

    return 1;
}


/*****************************************************************************
 * int urj_bsdl_set_bsr_length( urj_bsdl_jtag_ctrl_t *jc )
 *
 * Creates the BSR register based on the specified length.
 *
 * Parameters
 *   jc : jtag control structure
 *
 * Returns
 *   1 -> all ok
 *   0 -> error occured
 ****************************************************************************/
static int
urj_bsdl_set_bsr_length (urj_bsdl_jtag_ctrl_t *jc)
{
    create_register (jc, "BSR", jc->bsr_len);

    return 1;
}


/*****************************************************************************
 * int urj_bsdl_process_cell_info( urj_bsdl_jtag_ctrl_t *jc )
 * Cell Info management function
 *
 * Creates a BSR cell from the temporary storage variables via shell command
 *   bit <bit_num> <type> <default> <signal> [<cbit> <cval> Z]
 *
 * Parameters
 *   jc : jtag control structure
 *
 * Returns
 *   1 -> all ok
 *   0 -> error occured
 ****************************************************************************/
static int
urj_bsdl_process_cell_info (urj_bsdl_jtag_ctrl_t *jc)
{
    urj_bsdl_cell_info_t *ci = jc->cell_info_first;
    const size_t str_len = 10;
    char bit_num_str[str_len + 1];
    char ctrl_bit_num_str[str_len + 1];
    char disable_safe_value_str[str_len + 1];
    char *cmd[] = { "bit",
        bit_num_str,
        NULL,
        NULL,
        NULL,
        NULL,
        disable_safe_value_str,
        "Z",
        NULL
    };

    while (ci)
    {
        /* convert bit number to string */
        snprintf (bit_num_str, str_len, "%i", ci->bit_num);
        bit_num_str[str_len] = '\0';
        /* convert cell function from BSDL token to jtag syntax */
        switch (ci->cell_function)
        {
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
        cmd[3] =
            strcasecmp (ci->basic_safe_value,
                        "x") == 0 ? "?" : ci->basic_safe_value;
        /* apply port name */
        cmd[4] = ci->port_name;

        /* add disable spec if present */
        if (ci->ctrl_bit_num >= 0)
        {
            /* convert bit number to string */
            snprintf (ctrl_bit_num_str, str_len, "%i", ci->ctrl_bit_num);
            ctrl_bit_num_str[str_len] = '\0';
            /* convert disable safe value to string */
            snprintf (disable_safe_value_str, str_len, "%i",
                      ci->disable_safe_value);
            disable_safe_value_str[str_len] = '\0';
            cmd[5] = ctrl_bit_num_str;
        }
        else
            /* stop command procssing here */
            cmd[5] = NULL;

        if (jc->proc_mode & URJ_BSDL_MODE_INSTR_EXEC)
            urj_cmd_run (jc->chain, cmd);
        if (jc->proc_mode & URJ_BSDL_MODE_INSTR_PRINT)
            print_cmd (cmd);

        ci = ci->next;
    }

    return 1;
}


/*****************************************************************************
 * int urj_bsdl_process_register_access( urj_bsdl_jtag_ctrl_t *jc )
 * Register Access management function
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
 *   jc : jtag control structure
 *
 * Returns
 *   1 -> all ok
 *   0 -> error occured
 ****************************************************************************/
static int
urj_bsdl_process_register_access (urj_bsdl_jtag_ctrl_t *jc)
{
    urj_bsdl_types_ainfo_elem_t *ai;
    urj_bsdl_instr_elem_t *cinst;

    /* ensure that all mandatory registers are created prior to
       handling the instruction/register associations
       + BOUNDARY/BSR has been generated during the parsing process
       + DEVICE_ID/DIR has been generated during the parsing process
     */
    /* we need a BYPASS register */
    create_register (jc, "BYPASS", 1);

    /* next scan through all register_access definitions and create
       the non-standard registers */
    ai = jc->ainfo_list;
    while (ai)
    {
        int is_std = 0;

        if (strcasecmp (ai->reg, "BOUNDARY") == 0)
            is_std = 1;
        if (strcasecmp (ai->reg, "BYPASS") == 0)
            is_std = 1;
        if (strcasecmp (ai->reg, "DEVICE_ID") == 0)
            is_std = 1;
        if (strcasecmp (ai->reg, "USERCODE") == 0)
            is_std = 1;

        if (!is_std)
            create_register (jc, ai->reg, ai->reg_len);

        ai = ai->next;
    }


    /* next scan through all instruction/opcode definitions and resolve
       the instruction/register associations for these */
    cinst = jc->instr_list;
    while (cinst)
    {
        char *reg_name = NULL;
        char *instr_name = NULL;

        /* now see which of the register_access elements matches this instruction */
        ai = jc->ainfo_list;
        while (ai && (reg_name == NULL))
        {
            urj_bsdl_instr_elem_t *tinst = ai->instr_list;

            while (tinst && (reg_name == NULL))
            {
                if (strcasecmp (tinst->instr, cinst->instr) == 0)
                {
                    /* found the instruction inside the current access info,
                       now set the register name
                       map some standard register names to different internal names */
                    if (strcasecmp (ai->reg, "BOUNDARY") == 0)
                        reg_name = "BSR";
                    else if (strcasecmp (ai->reg, "DEVICE_ID") == 0)
                        reg_name = "DIR";
                    else
                        reg_name = ai->reg;
                }

                tinst = tinst->next;
            }

            ai = ai->next;
        }

        if (reg_name == NULL)
        {
            /* BSDL file didn't specify an explicit register_access definition
               if we're looking at a standard mandatory instruction, we should
               build the association ourselves */
            if (strcasecmp (cinst->instr, "BYPASS") == 0)
                reg_name = "BYPASS";
            else if (strcasecmp (cinst->instr, "CLAMP") == 0)
                reg_name = "BYPASS";
            else if (strcasecmp (cinst->instr, "EXTEST") == 0)
                reg_name = "BSR";
            else if (strcasecmp (cinst->instr, "HIGHZ") == 0)
                reg_name = "BYPASS";
            else if (strcasecmp (cinst->instr, "IDCODE") == 0)
                reg_name = "DIR";
            else if (strcasecmp (cinst->instr, "INTEST") == 0)
                reg_name = "BSR";
            else if (strcasecmp (cinst->instr, "PRELOAD") == 0)
                reg_name = "BSR";
            else if (strcasecmp (cinst->instr, "SAMPLE") == 0)
                reg_name = "BSR";
            else if (strcasecmp (cinst->instr, "USERCODE") == 0)
                reg_name = "USERCODE";
        }

        if (strcasecmp (cinst->instr, "SAMPLE") == 0)
            instr_name = "SAMPLE/PRELOAD";
        else
            instr_name = cinst->instr;

        if (reg_name)
        {
            char *cmd[] = { "instruction",
                instr_name,
                cinst->opcode,
                reg_name,
                NULL
            };

            if (jc->proc_mode & URJ_BSDL_MODE_INSTR_EXEC)
                urj_cmd_run (jc->chain, cmd);
            if (jc->proc_mode & URJ_BSDL_MODE_INSTR_PRINT)
                print_cmd (cmd);
        }

        cinst = cinst->next;
    }

    return 1;
}


/*****************************************************************************
 * int parse_vhdl_elem( urj_bsdl_parser_priv_t *priv, urj_bsdl_vhdl_elem_t *elem )
 *
 * Runs the specified vhdl element through the BSDl parser.
 *
 * Parameters
 *   priv : private data container for parser related tasks
 *   elem : vhdl element to be parsed
 *
 * Returns
 *   URJ_BSDL_MODE_SYN_CHECK -> parsing successful
 *   0                   -> error occured
 ****************************************************************************/
static int
parse_vhdl_elem (urj_bsdl_parser_priv_t *priv, urj_bsdl_vhdl_elem_t *elem)
{
    char *buf;
    size_t buf_len;
    size_t name_string_len;
    size_t elem_string_len;

    name_string_len = elem->name ? strlen (elem->name) : 0;
    elem_string_len = elem->payload ? strlen (elem->payload) : 0;

    /* allocate enough memory for total buffer */
    buf_len = name_string_len + 1 + elem_string_len + 1;
    buf = malloc (buf_len);
    if (!buf)
    {
        urj_bsdl_msg (priv->jtag_ctrl->proc_mode,
                      BSDL_MSG_FATAL, _("Out of memory, %s line %i\n"),
                      __FILE__, __LINE__);
        return -1;
    }
    buf[0] = '\0';

    if (name_string_len > 0)
        strncat (buf, elem->name, buf_len);
    strncat (buf, " ", buf_len - name_string_len);

    if (elem_string_len > 0)
        strncat (buf, elem->payload, buf_len - name_string_len - 1);

    buf[buf_len - 1] = '\0';

    priv->lineno = elem->line;

    /* buffer is prepared for string parsing */
    urj_bsdl_flex_switch_buffer (priv->scanner, buf, elem->line);
    urj_bsdl_parse (priv);

    free (buf);

    return urj_bsdl_flex_get_compile_errors (priv->scanner) ==
        0 ? URJ_BSDL_MODE_SYN_CHECK : 0;
}


/*****************************************************************************
 * int build_commands( urj_bsdl_parser_priv_t *priv )
 *
 * Calls the various functions that execute or print the information extracted
 * from the BSDL/vhdl elements.
 *
 * Parameters
 *   priv    : private data container for parser related tasks
 *
 * Returns
 *   bit field consisting of BSDL_MODE_* actions
 *   telling if INSTR_EXEC or INSTR_PRINT succeeded
 ****************************************************************************/
static int
build_commands (urj_bsdl_parser_priv_t *priv)
{
    urj_bsdl_jtag_ctrl_t *jc = priv->jtag_ctrl;
    int result = 1;

    result &= urj_bsdl_emit_ports (jc);

    result &= urj_bsdl_set_instruction_length (jc);

    result &= urj_bsdl_process_idcode (jc);

    result &= urj_bsdl_process_usercode (jc);

    result &= urj_bsdl_set_bsr_length (jc);

    result &= urj_bsdl_process_register_access (jc);

    result &= urj_bsdl_process_cell_info (jc);

    return result ? URJ_BSDL_MODE_INSTR_EXEC | URJ_BSDL_MODE_INSTR_PRINT : 0;
}


/*****************************************************************************
 * int compare_idcode( urj_bsdl_jtag_ctrl_t *jc, const char *idcode )
 *
 * Compares idcode versus jtag_ctrl->idcode.
 *
 * Parameters
 *   jc     : jtag_ctrl structure
 *   idcode : idcode string
 *
 * Returns
 *   1 -> idcodes match
 *   0 -> idcodes don't match
 ****************************************************************************/
static int
compare_idcode (urj_bsdl_jtag_ctrl_t *jc, const char *idcode)
{
    int idcode_match = 0;

    /* should we compare the idcodes? */
    if (idcode)
    {
        if (strlen (idcode) == strlen (jc->idcode))
        {
            int idx;

            /* compare given idcode with idcode from BSDL file */
            idcode_match = URJ_BSDL_MODE_IDCODE_CHECK;
            for (idx = 0; idx < strlen (idcode); idx++)
                if (jc->idcode[idx] != 'X')
                    if (idcode[idx] != jc->idcode[idx])
                        idcode_match = 0;

            if (idcode_match)
                urj_bsdl_msg (jc->proc_mode,
                              BSDL_MSG_NOTE, _("IDCODE matched\n"));
            else
                urj_bsdl_msg (jc->proc_mode,
                              BSDL_MSG_NOTE, _("IDCODE mismatch\n"));
        }
    }

    return idcode_match;
}


/*****************************************************************************
 * int urj_bsdl_process_elements( urj_bsdl_jtag_ctrl_t *jc, const char *idcode )
 *
 * If enabled, runs through the list of vhdl elements in jtag ctrl and parser
 * them as BSDL statements.
 * If enabled, compares idcode versus jc->idcode.
 * If enabled, prints or executes the resulting jtag commands.
 *
 * Parameters
 *   jc     : jtag_ctrl structure
 *   idcode : idcode string
 *
 * Returns
 *   < 0 : Error occured, parse/syntax problems or out of memory
 *   = 0 : No errors, idcode not checked or mismatching
 *   > 0 : No errors, idcode checked and matched
 *
 ****************************************************************************/
int
urj_bsdl_process_elements (urj_bsdl_jtag_ctrl_t *jc, const char *idcode)
{
    urj_bsdl_parser_priv_t *priv;
    urj_bsdl_vhdl_elem_t *el = jc->vhdl_elem_first;
    int result = URJ_BSDL_MODE_SYN_CHECK;

    if ((priv = urj_bsdl_parser_init (jc)) == NULL)
        return -1;

    if (jc->proc_mode & URJ_BSDL_MODE_SYN_CHECK)
    {
        while (el && (result & URJ_BSDL_MODE_SYN_CHECK))
        {
            result = parse_vhdl_elem (priv, el);

            el = el->next;
        }

        if (!(result & URJ_BSDL_MODE_SYN_CHECK))
        {
            urj_bsdl_msg (jc->proc_mode,
                          BSDL_MSG_ERR,
                          _("BSDL stage reported errors, aborting.\n"));
            urj_bsdl_parser_deinit (priv);
            return -1;
        }
    }

    if (jc->idcode)
        urj_bsdl_msg (jc->proc_mode,
                      BSDL_MSG_NOTE, _("Got IDCODE: %s\n"), jc->idcode);

    if (jc->proc_mode & URJ_BSDL_MODE_IDCODE_CHECK)
        result |= compare_idcode (jc, idcode);

    if (jc->
        proc_mode & (URJ_BSDL_MODE_INSTR_EXEC | URJ_BSDL_MODE_INSTR_PRINT))
        /* IDCODE check positive if requested? */
        if (((jc->proc_mode & URJ_BSDL_MODE_IDCODE_CHECK) &&
             (result & URJ_BSDL_MODE_IDCODE_CHECK))
            || (!(jc->proc_mode & URJ_BSDL_MODE_IDCODE_CHECK)))
            result |= build_commands (priv);

    if ((result & jc->proc_mode) ==
        (jc->proc_mode & URJ_BSDL_MODE_ACTION_ALL))
        if (jc->proc_mode & URJ_BSDL_MODE_IDCODE_CHECK)
            result = 1;
        else
            result = 0;
    else
        result = -1;

    urj_bsdl_parser_deinit (priv);

    return result;
}


/*
 Local Variables:
 mode:C
 c-default-style:gnu
 indent-tabs-mode:nil
 End:
*/
