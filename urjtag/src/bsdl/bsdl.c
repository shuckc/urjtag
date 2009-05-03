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

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#define __USE_GNU
#include <string.h>

#include <dirent.h>
#include <sys/stat.h>

#include <urjtag/sysdep.h>

#include <urjtag/chain.h>
#include <urjtag/part.h>
#include <urjtag/cmd.h>

//#include "bsdl_local.h"
#include "bsdl_types.h"
#include "vhdl_parser.h"
#include "bsdl_parser.h"

#include "bsdl_msg.h"

#ifdef DMALLOC
#include "dmalloc.h"
#endif


/*****************************************************************************
 * urj_bsdl_msg( proc_mode, type, format, ... )
 *
 * Main printing function for the BSDL subsystem.
 *
 * Parameters
 *   proc_mode : processing mode, consisting of BSDL_MODE_* bits
 *   type      : one of the BSDL_MSG_* defines, determines message tag
 *   format    : printf format
 *   ...       : additional parameters to fill the printf format string
 *
 * Returns
 *   void
 ****************************************************************************/
void
urj_bsdl_msg (int proc_mode, int type, const char *format, ...)
{
    va_list lst;

    va_start (lst, format);
    switch (type)
    {
    case BSDL_MSG_NOTE:
        if (!(proc_mode & URJ_BSDL_MODE_MSG_NOTE))
            return;
        printf ("-N- ");
        break;
    case BSDL_MSG_WARN:
        if (!(proc_mode & URJ_BSDL_MODE_MSG_WARN))
            return;
        printf ("-W- ");
        break;
    case BSDL_MSG_ERR:
        if (!(proc_mode & URJ_BSDL_MODE_MSG_ERR))
            return;
        printf ("-E- ");
        break;
    case BSDL_MSG_FATAL:
        if (!(proc_mode & URJ_BSDL_MODE_MSG_FATAL))
            return;
        printf ("-F- ");
        break;
    default:
        printf ("-?- ");
        break;
    }
    vprintf (format, lst);
    va_end (lst);
}


/*****************************************************************************
 * urj_bsdl_read_file( chain, BSDL_File_Name, proc_mode, idcode )
 *
 * Read, parse and optionally apply contents of BSDL file.
 *
 * Parameters
 *   chain     : pointer to active chain structure
 *   BSDL_File_Name : name of BSDL file to read
 *   proc_mode : processing mode, consisting of BSDL_MODE_* bits
 *   idcode    : reference idcode string
 *
 * Returns
 *   < 0 : Error occured, parse/syntax problems or out of memory
 *   = 0 : No errors, idcode not checked or mismatching
 *   > 0 : No errors, idcode checked and matched
 *
 ****************************************************************************/
int
urj_bsdl_read_file (urj_chain_t *chain, const char *BSDL_File_Name,
                    int proc_mode, const char *idcode)
{
    urj_bsdl_globs_t *globs = &(chain->bsdl);
    FILE *BSDL_File;
    urj_vhdl_parser_priv_t *vhdl_parser_priv;
    urj_bsdl_jtag_ctrl_t jtag_ctrl;
    int Compile_Errors = 1;
    int result;

    if (globs->debug)
        proc_mode |= URJ_BSDL_MODE_MSG_ALL;

    jtag_ctrl.proc_mode = proc_mode;

    /* perform some basic checks */
    if (proc_mode & URJ_BSDL_MODE_INSTR_EXEC)
    {
        if (chain == NULL)
        {
            urj_bsdl_msg (proc_mode, BSDL_MSG_ERR,
                          _("No JTAG chain available\n"));
            return -1;
        }
        if (chain->parts == NULL)
        {
            urj_bsdl_msg (proc_mode, BSDL_MSG_ERR,
                          _("Chain without any parts\n"));
            return -1;
        }
        if (!(chain && chain->parts))
            return -1;

        jtag_ctrl.chain = chain;
        jtag_ctrl.part = chain->parts->parts[chain->active_part];
    }
    else
    {
        jtag_ctrl.chain = NULL;
        jtag_ctrl.part = NULL;
    }

    BSDL_File = fopen (BSDL_File_Name, "r");

    urj_bsdl_msg (proc_mode, BSDL_MSG_NOTE, _("Reading file '%s'\n"),
                  BSDL_File_Name);

    if (BSDL_File == NULL)
    {
        urj_bsdl_msg (proc_mode,
                      BSDL_MSG_ERR, _("Unable to open BSDL file '%s'\n"),
                      BSDL_File_Name);
        return -1;
    }

    if ((vhdl_parser_priv = urj_vhdl_parser_init (BSDL_File, &jtag_ctrl)))
    {
        vhdl_parser_priv->jtag_ctrl->idcode = NULL;

        urj_vhdl_parse (vhdl_parser_priv);

        Compile_Errors =
            urj_vhdl_flex_get_compile_errors (vhdl_parser_priv->scanner);
        if (Compile_Errors == 0)
        {
            urj_bsdl_msg (proc_mode,
                          BSDL_MSG_NOTE,
                          _("BSDL file '%s' passed VHDL stage correctly\n"),
                          BSDL_File_Name);

            result = urj_bsdl_process_elements (&jtag_ctrl, idcode);

            if (result >= 0)
                urj_bsdl_msg (proc_mode,
                              BSDL_MSG_NOTE,
                              _
                              ("BSDL file '%s' passed BSDL stage correctly\n"),
                              BSDL_File_Name);

        }
        else
        {
            urj_bsdl_msg (proc_mode,
                          BSDL_MSG_ERR,
                          _
                          ("BSDL file '%s' contains errors in VHDL stage, stopping\n"),
                          BSDL_File_Name);
        }


        urj_vhdl_parser_deinit (vhdl_parser_priv);
    }

    return Compile_Errors == 0 ? result : -1;
}


/*****************************************************************************
 * void urj_bsdl_set_path( chain, pathlist )
 *
 * Dissects pathlist and enters its elements to the global variable
 * bsdl.path_list.
 *
 * Parameters
 *   chain    : pointer to active chain structure
 *   pathlist : string containing the paths to be stored, format:
 *              <path1>;<path2>;<path3>;...
 *
 * Returns
 *   void
 ****************************************************************************/
void
urj_bsdl_set_path (urj_chain_t *chain, const char *pathlist)
{
    urj_bsdl_globs_t *globs = &(chain->bsdl);
    char *delim;
    char *elem;
    char *pathelem;
    int num;
    size_t len;

    /* free memory of current path list */
    if (globs->path_list)
    {
        for (num = 0; globs->path_list[num]; num++)
            if (globs->path_list[num])
                free (globs->path_list[num]);
        free (globs->path_list);
        globs->path_list = NULL;
    }

    /* run through path list and determine number of elements */
    for (num = 0, elem = (char *) pathlist; strlen (elem) > 0;)
    {
        delim = strchr (elem, ';');
        if ((delim - elem > 0) || (delim == NULL))
        {
            num++;
            /* extend path list array */
            /* @@@@ RFHH check realloc result */
            globs->path_list = realloc (globs->path_list,
                                        (num + 1) * sizeof (char *));
            /* enter path element up to the delimeter */
            if (delim == NULL)
                len = strlen (elem);
            else
                len = delim - elem;
            /* @@@@ ToDo check malloc result RFHH */
            pathelem = malloc (len + 1);
            memcpy (pathelem, elem, len);
            pathelem[len] = '\0';
            globs->path_list[num - 1] = pathelem;
            globs->path_list[num] = NULL;
        }
        elem = delim ? delim + 1 : elem + strlen (elem);
    }

    if (globs->debug)
        for (num = 0; globs->path_list[num] != NULL; num++)
            urj_bsdl_msg (URJ_BSDL_MODE_MSG_ALL,
                          BSDL_MSG_NOTE, "%s\n", globs->path_list[num]);
}


/*****************************************************************************
 * urj_bsdl_scan_files( chain, idcode, proc_mode )
 *
 * Scans through all files found via the elements in bsdl_path_list
 * and does a test read on each of them.
 * If mode >= 1 is requested, it will read the first BSDL file with matching
 * idcode in "execute" mode. I.e. all extracted statements are applied to
 * the current part.
 *
 * Parameters
 *   chain     : pointer to active chain structure
 *   idcode    : reference idcode string
 *   proc_mode : processing mode, consisting of BSDL_MODE_* bits
 *
 * Returns
 *   < 0 : Error occured, parse/syntax problems or out of memory
 *   = 0 : No errors, idcode not checked or mismatching
 *   > 0 : No errors, idcode checked and matched
 *
 ****************************************************************************/
int
urj_bsdl_scan_files (urj_chain_t *chain, const char *idcode, int proc_mode)
{
    urj_bsdl_globs_t *globs = &(chain->bsdl);
    int idx = 0;
    int result = 0;

    /* abort if no path list was specified */
    if (globs->path_list == NULL)
        return 0;

    while (globs->path_list[idx] && (result <= 0))
    {
        DIR *dir;

        if ((dir = opendir (globs->path_list[idx])))
        {
            struct dirent *elem;

            /* run through all elements in the current directory */
            while ((elem = readdir (dir)) && (result <= 0))
            {
                char *name;

                /* @@@@ ToDo handle malloc error result RFHH */
                name = malloc (strlen (globs->path_list[idx])
                               + strlen (elem->d_name) + 1 + 1);
                if (name)
                {
                    struct stat buf;

                    strcpy (name, globs->path_list[idx]);
                    strcat (name, "/");
                    strcat (name, elem->d_name);

                    if (stat (name, &buf) == 0)
                    {
                        if (buf.st_mode & S_IFREG)
                        {
                            result =
                                urj_bsdl_read_file (chain, name, proc_mode,
                                                    idcode);
                            if (result == 1)
                                printf (_("  Filename:     %s\n"), name);
                        }
                    }

                    free (name);
                }
            }

            closedir (dir);
        }
        else
            urj_bsdl_msg (proc_mode,
                          BSDL_MSG_WARN, _("Cannot open directory %s\n"),
                          globs->path_list[idx]);

        idx++;
    }

    return result;
}


/*
 Local Variables:
 mode:C
 c-default-style:gnu
 indent-tabs-mode:nil
 End:
*/
