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

#include <urjtag/cmd.h>
#include <urjtag/part.h>
#include <urjtag/bsbit.h>
#include <urjtag/data_register.h>
#include <urjtag/bssignal.h>
#include <urjtag/log.h>

#include "bsdl_sysdep.h"

#include "bsdl_types.h"
#include "bsdl_msg.h"

#include "bsdl_parser.h"
#include "bsdl_bison.h"

#ifdef DMALLOC
#include "dmalloc.h"
#endif


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
 *   URJ_STATUS_OK, URJ_STATUS_FAIL
 ****************************************************************************/
static int
urj_bsdl_set_instruction_length (urj_bsdl_jtag_ctrl_t *jc)
{
    if (jc->proc_mode & URJ_BSDL_MODE_INSTR_EXEC)
        (void) urj_part_instruction_length_set (jc->part, jc->instr_len);
    if (jc->proc_mode & URJ_BSDL_MODE_INSTR_PRINT)
        urj_log (URJ_LOG_LEVEL_NORMAL, "instruction length %i\n", jc->instr_len);

    return URJ_STATUS_OK;
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
 *   URJ_STATUS_OK, URJ_STATUS_FAIL
 ****************************************************************************/
static int
urj_bsdl_emit_ports (urj_bsdl_jtag_ctrl_t *jc)
{
    urj_bsdl_port_desc_t *pd = jc->port_desc;
    struct string_elem *name;
    size_t str_len, name_len;
    char *port_string;
    int idx;
    int result = URJ_STATUS_FAIL;

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
                for (idx = pd->low_idx; idx <= pd->high_idx; idx++)
                {
                    if (pd->is_vector)
                        snprintf (port_string, str_len - 1, "%s(%d)",
                                  name->string, idx);
                    else
                        strncpy (port_string, name->string, str_len - 1);
                    port_string[str_len - 1] = '\0';

                    if (jc->proc_mode & URJ_BSDL_MODE_INSTR_EXEC)
                        (void) urj_part_signal_define (jc->chain, port_string);
                    if (jc->proc_mode & URJ_BSDL_MODE_INSTR_PRINT)
                        urj_log (URJ_LOG_LEVEL_NORMAL, "signal %s\n", port_string);
                }

                free (port_string);
                result = URJ_STATUS_OK;
            }
            else
            {
                urj_bsdl_err_set (jc->proc_mode, URJ_ERROR_OUT_OF_MEMORY,
                                  "No memory");
                return URJ_STATUS_FAIL;
            }

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
 *   URJ_STATUS_OK, URJ_STATUS_FAIL
 ****************************************************************************/
static int
create_register (urj_bsdl_jtag_ctrl_t *jc, char *reg_name, size_t len)
{
    int result = URJ_STATUS_OK;

    if (urj_part_find_data_register (jc->part, reg_name) != NULL)
        return URJ_STATUS_OK;

    if (jc->proc_mode & URJ_BSDL_MODE_INSTR_EXEC)
        result = urj_part_data_register_define (jc->part, reg_name, len);
    if (jc->proc_mode & URJ_BSDL_MODE_INSTR_PRINT)
        urj_log (URJ_LOG_LEVEL_NORMAL, "register %s %zd\n", reg_name, len);

    return result;
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
 *   URJ_STATUS_OK, URJ_STATUS_FAIL
 ****************************************************************************/
static int
urj_bsdl_process_idcode (urj_bsdl_jtag_ctrl_t *jc)
{
    int result = URJ_STATUS_OK;

    if (jc->idcode)
        result = create_register (jc, "DIR", strlen (jc->idcode));
    else
        urj_bsdl_warn (jc->proc_mode,
                       _("No IDCODE specification found.\n"));

    return result;
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
 *   URJ_STATUS_OK, URJ_STATUS_FAIL
 ****************************************************************************/
static int
urj_bsdl_process_usercode (urj_bsdl_jtag_ctrl_t *jc)
{
    int result = URJ_STATUS_OK;

    if (jc->usercode)
        result = create_register (jc, "USERCODE", strlen (jc->usercode));

    /* we're not interested in the usercode value at all */

    return result;
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
 *   URJ_STATUS_OK, URJ_STATUS_FAIL
 ****************************************************************************/
static int
urj_bsdl_set_bsr_length (urj_bsdl_jtag_ctrl_t *jc)
{
    return create_register (jc, "BSR", jc->bsr_len);
}


static char
bsbit_type_char (int type)
{
    switch (type)
    {
    case URJ_BSBIT_INPUT:
        return 'I';
    case URJ_BSBIT_OUTPUT:
        return 'O';
    case URJ_BSBIT_CONTROL:
        return 'C';
    case URJ_BSBIT_INTERNAL:
        return 'X';
    case URJ_BSBIT_BIDIR:
        return 'B';
    default:
        return '?';
    }
}

static char
bsbit_safe_char (int safe)
{
    switch (safe)
    {
    case 0:
        return '0';
    case 1:
        return '1';
    default:
        return '?';
    }
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
 *   URJ_STATUS_OK, URJ_STATUS_FAIL
 ****************************************************************************/
static int
urj_bsdl_process_cell_info (urj_bsdl_jtag_ctrl_t *jc)
{
    urj_bsdl_cell_info_t *ci = jc->cell_info_first;
    int type;
    int safe;

    while (ci)
    {
        /* convert cell function from BSDL token to jtag syntax */
        switch (ci->cell_function)
        {
        case INTERNAL:
            /* fall through */
        case OUTPUT2:
            /* fall through */
        case OUTPUT3:
            type = URJ_BSBIT_OUTPUT;
            break;
        case OBSERVE_ONLY:
            /* fall through */
        case INPUT:
            /* fall through */
        case CLOCK:
            type = URJ_BSBIT_INPUT;
            break;
        case CONTROL:
            /* fall through */
        case CONTROLR:
            type = URJ_BSBIT_CONTROL;
            break;
        case BIDIR:
            type = URJ_BSBIT_BIDIR;
            break;
        default:
            /* spoil command */
            type = -1;
            break;
        }
        /* convert basic safe value */
        safe = strcasecmp (ci->basic_safe_value, "x") == 0 ? URJ_BSBIT_DONTCARE
                   : (ci->basic_safe_value[0] - '0');

        if (ci->ctrl_bit_num >= 0)
        {
            if (jc->proc_mode & URJ_BSDL_MODE_INSTR_EXEC)
                if (urj_part_bsbit_alloc_control (jc->part, ci->bit_num,
                                                  ci->port_name, type, safe,
                                                  ci->ctrl_bit_num,
                                                  ci->disable_safe_value,
                                                  URJ_BSBIT_STATE_Z) !=
                    URJ_STATUS_OK)
                    return URJ_STATUS_FAIL;
            if (jc->proc_mode & URJ_BSDL_MODE_INSTR_PRINT)
                urj_log (URJ_LOG_LEVEL_NORMAL,
                         "bit %d %c %c %s %d %d %c\n", ci->bit_num,
                         bsbit_type_char (type), bsbit_safe_char(safe), ci->port_name,
                         ci->ctrl_bit_num, ci->disable_safe_value,
                         'Z');
        }
        else
        {
            if (jc->proc_mode & URJ_BSDL_MODE_INSTR_EXEC)
                if (urj_part_bsbit_alloc (jc->part, ci->bit_num,
                                          ci->port_name, type, safe) !=
                    URJ_STATUS_OK)
                    return URJ_STATUS_FAIL;
            if (jc->proc_mode & URJ_BSDL_MODE_INSTR_PRINT)
                urj_log (URJ_LOG_LEVEL_NORMAL,
                         "bit %d %c %c %s\n", ci->bit_num,
                         bsbit_type_char (type), bsbit_safe_char(safe), ci->port_name);
        }

        ci = ci->next;
    }

    return URJ_STATUS_OK;
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
 *   URJ_STATUS_OK, URJ_STATUS_FAIL
 ****************************************************************************/
static int
urj_bsdl_process_register_access (urj_bsdl_jtag_ctrl_t *jc)
{
    urj_bsdl_types_ainfo_elem_t *ai;
    urj_bsdl_instr_elem_t *cinst;
    int result;

    /* ensure that all mandatory registers are created prior to
       handling the instruction/register associations
       + BOUNDARY/BSR has been generated during the parsing process
       + DEVICE_ID/DIR has been generated during the parsing process
     */
    /* we need a BYPASS register */
    result = create_register (jc, "BYPASS", 1);
    if (result != URJ_STATUS_OK)
        return result;

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
            if (create_register (jc, ai->reg, ai->reg_len) != URJ_STATUS_OK)
                result = URJ_STATUS_FAIL;

        ai = ai->next;
    }

    if (result != URJ_STATUS_OK)
        return result;


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
            if (jc->proc_mode & URJ_BSDL_MODE_INSTR_EXEC)
                // @@@@ RFHH check if jc->part equals chain_active_part
                if (urj_part_instruction_define (jc->part, instr_name,
                                                 cinst->opcode, reg_name) ==
                    NULL)
                    return URJ_STATUS_FAIL;
            if (jc->proc_mode & URJ_BSDL_MODE_INSTR_PRINT)
                urj_log (URJ_LOG_LEVEL_NORMAL,
                         "instruction %s %s %s\n", instr_name, cinst->opcode,
                         reg_name);
        }

        cinst = cinst->next;
    }

    return URJ_STATUS_OK;
}


/*****************************************************************************
 * int parse_vhdl_elem( urj_bsdl_parser_priv_t *priv, urj_vhdl_elem_t *elem )
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
parse_vhdl_elem (urj_bsdl_parser_priv_t *priv, urj_vhdl_elem_t *elem)
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
        urj_bsdl_err_set (priv->jtag_ctrl->proc_mode,
                          URJ_ERROR_OUT_OF_MEMORY,
                          "No memory");
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

    result = urj_bsdl_emit_ports (jc)              == URJ_STATUS_OK ? result : 0;

    result = urj_bsdl_set_instruction_length (jc)  == URJ_STATUS_OK ? result : 0;

    result = urj_bsdl_process_idcode (jc)          == URJ_STATUS_OK ? result : 0;

    result = urj_bsdl_process_usercode (jc)        == URJ_STATUS_OK ? result : 0;

    result = urj_bsdl_set_bsr_length (jc)          == URJ_STATUS_OK ? result : 0;

    result = urj_bsdl_process_register_access (jc) == URJ_STATUS_OK ? result : 0;

    result = urj_bsdl_process_cell_info (jc)       == URJ_STATUS_OK ? result : 0;

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
                urj_bsdl_msg (jc->proc_mode, _("IDCODE matched\n"));
            else
                urj_bsdl_msg (jc->proc_mode, _("IDCODE mismatch\n"));
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
    urj_vhdl_elem_t *el = jc->vhdl_elem_first;
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
            urj_bsdl_err (jc->proc_mode,
                          _("BSDL stage reported errors, aborting.\n"));
            urj_bsdl_parser_deinit (priv);
            return -1;
        }
    }

    if (jc->idcode)
        urj_bsdl_msg (jc->proc_mode, _("Got IDCODE: %s\n"), jc->idcode);

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
 c-default-style:java
 indent-tabs-mode:nil
 End:
*/
