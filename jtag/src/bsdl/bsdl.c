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

#include "sysdep.h"

#include <jtag.h>
#include <cmd.h>

#include "bsdl.h"


static char **bsdl_path_list = NULL;

int bsdl_debug = 0;


void bsdl_msg(int type, const char *format, ...)
{
  va_list lst;

  va_start(lst, format);
  switch (type) {
    case BSDL_MSG_NOTE:
      printf("-N- ");
      break;
    case BSDL_MSG_WARN:
      printf("-W- ");
      break;
    case BSDL_MSG_ERR:
      printf("-E- ");
      break;
    case BSDL_MSG_FATAL:
      printf("-F- ");
      break;
    default:
      printf("-?- ");
      break;
  }
  vprintf(format, lst);
  va_end(lst);
}


/*****************************************************************************
 * bsdl_read_file(BSDL_File_Name, mode, idcode)
 *
 * mode: -1 -> read file
 *             no further action based on components
 *        0 -> read file and extract all components
 *             dump commands to stdout, do not execute commands
 *        1 -> read file and extract all components
 *             execute commands
 *
 * Return value:
 * < 0 : Error occured, parse/syntax problems or out of memory
 * = 0 : No errors, idcode not checked or mismatching
 * > 0 : No errors, idcode checked and matched
 *
 ****************************************************************************/
int bsdl_read_file(const char *BSDL_File_Name, int mode, const char *idcode)
{
  FILE *BSDL_File;
  parser_priv_t *parser_priv;
  int Compile_Errors = 1;
  int idcode_match = 0;

  BSDL_File = fopen(BSDL_File_Name, "r");

  if (bsdl_debug || (mode == 0))
    bsdl_msg(BSDL_MSG_NOTE, _("Reading file '%s'\n"), BSDL_File_Name);

  if (BSDL_File == NULL) {
    bsdl_msg(BSDL_MSG_ERR, _("Unable to open BSDL_file '%s'\n"), BSDL_File_Name);
    return -1;
  }

  if ((parser_priv = bsdl_parser_init(BSDL_File))) {
    if (mode >= 0) {
      if (mode >= 1) {
	if (chain == NULL) {
	  bsdl_msg(BSDL_MSG_ERR, _("No JTAG chain available\n"));
	  bsdl_parser_deinit(parser_priv);
	  fclose(BSDL_File);
	  return -1;
	}
	if (chain->parts == NULL) {
	  bsdl_msg(BSDL_MSG_ERR, _("Chain without any parts\n"));
	  bsdl_parser_deinit(parser_priv);
	  fclose(BSDL_File);
	  return -1;
	}
	if (!(chain && chain->parts)) {
	  bsdl_parser_deinit(parser_priv);
	  fclose(BSDL_File);
	  return -1;
	}
	parser_priv->jtag_ctrl.part = chain->parts->parts[chain->active_part];
      } else
	parser_priv->jtag_ctrl.part = NULL;
    } else
      parser_priv->jtag_ctrl.part = NULL;

    parser_priv->jtag_ctrl.mode   = mode;
    parser_priv->jtag_ctrl.debug  = bsdl_debug;
    parser_priv->jtag_ctrl.idcode = NULL;

    bsdlparse(parser_priv);

    Compile_Errors = bsdl_flex_get_compile_errors(parser_priv->scanner);
    if (Compile_Errors == 0) {
      if (bsdl_debug)
        bsdl_msg(BSDL_MSG_NOTE, _("BSDL file '%s' compiled correctly\n"), BSDL_File_Name);
    } else {
      if (bsdl_debug || (mode >= 0))
        bsdl_msg(BSDL_MSG_ERR, _("BSDL file '%s' contains errors, stopping\n"), BSDL_File_Name); 
    }

    if (Compile_Errors == 0)
      bsdl_ac_finalize(parser_priv);

    if ((Compile_Errors == 0) && parser_priv->jtag_ctrl.idcode) {
      if (bsdl_debug)
        bsdl_msg(BSDL_MSG_NOTE, _("Got IDCODE: %s\n"), parser_priv->jtag_ctrl.idcode);

      /* should be compare the idcodes? */
      if (idcode) {
        if (strlen(idcode) == strlen(parser_priv->jtag_ctrl.idcode)) {
          int idx;

          /* compare given idcode with idcode from BSDL file
             including the end of string character */
          idcode_match = 1;
          for (idx = 0; idx <= strlen(idcode); idx++)
            if (parser_priv->jtag_ctrl.idcode[idx] != 'X')
              if (idcode[idx] != parser_priv->jtag_ctrl.idcode[idx])
                idcode_match = 0;

          if (bsdl_debug) {
            if (idcode_match)
              bsdl_msg(BSDL_MSG_NOTE, _("IDCODE matched\n") );
            else
              bsdl_msg(BSDL_MSG_NOTE, _("IDCODE mismatch\n") );
          }
        }
      }

      if (parser_priv->jtag_ctrl.idcode)
        free(parser_priv->jtag_ctrl.idcode);
      parser_priv->jtag_ctrl.idcode = NULL;
    }
    bsdl_parser_deinit(parser_priv);
  }

  return Compile_Errors == 0 ? idcode_match : -1;
}


/*****************************************************************************
 * void bsdl_set_path(const char *pathlist)
 *
 * Dissects pathlist and enters its elements to the global variable
 * bsdl_path_list.
 *
 * Return value:
 * void
 ****************************************************************************/
void bsdl_set_path(const char *pathlist)
{
  char *delim;
  char *elem;
  int num;

  /* free memory of current path list */
  if (bsdl_path_list) {
    for (num = 0; bsdl_path_list[num]; num++)
      if (bsdl_path_list[num])
        free(bsdl_path_list[num]);
    free(bsdl_path_list);
    bsdl_path_list = NULL;
  }

  /* run through path list and determine number of elements */
  for (num = 0, elem = (char *)pathlist; strlen(elem) > 0; ) {
    delim = strchr(elem, ';');
    if ((delim - elem > 0) || (delim == NULL)) {
      num++;
      /* extend path list array */
      bsdl_path_list = (char **)realloc(bsdl_path_list, (num+1) * sizeof(char *));
      /* enter path element up to the delimeter */
      bsdl_path_list[num-1] = strndup(elem, (size_t)(delim - elem));
      bsdl_path_list[num] = NULL;
    }
    elem = delim ? delim + 1 : elem + strlen(elem);
  }

  if (bsdl_debug)
    for (num = 0; bsdl_path_list[num] != NULL; num++) {
      bsdl_msg(BSDL_MSG_NOTE, "%s\n", bsdl_path_list[num]);
    }
}


/*****************************************************************************
 * int bsdl_scan_files(const char *idcode, int mode)
 *
 * Scans through all files found via the elements in bsdl_path_list
 * and does a test read on each of them.
 * If mode >= 1 is requested, it will read the first BSDL file with matching
 * idcode in "execute" mode. I.e. all extracted statements are applied to
 * the current part.
 * 
 * mode: -1 -> read file
 *             no further action based on components
 *        0 -> read file and extract all components
 *             dump commands to stdout, do not execute commands
 *        1 -> read file and extract all components
 *             execute commands
 *
 * Return value:
 * < 0 : Error occured, parse/syntax problems or out of memory
 * = 0 : No errors, idcode not checked or mismatching
 * > 0 : No errors, idcode checked and matched
 *
 ****************************************************************************/
int bsdl_scan_files(const char *idcode, int mode)
{
  int idx = 0;
  int result = 0;

  /* abort if no path list was specified */
  if (bsdl_path_list == NULL)
    return 0;

  while (bsdl_path_list[idx] && (result <= 0)) {
    DIR *dir;

    if ((dir = opendir(bsdl_path_list[idx]))) {
      struct dirent *elem;

      /* run through all elements in the current directory */
      while ((elem = readdir(dir)) && (result <= 0)) {
        char *name;

        name = (char *)malloc(strlen(bsdl_path_list[idx]) + strlen(elem->d_name) + 1 + 1);
        if (name) {
          struct stat buf;

          strcpy(name, bsdl_path_list[idx]);
          strcat(name, "/");
          strcat(name, elem->d_name);

          if (stat(name, &buf) == 0) {
            if (buf.st_mode & S_IFREG) {
	      if (mode >= 1) {
		/* now we know we can finally read the file */
		/* do a test read first */
		result = bsdl_read_file(name, -1, idcode);
		if (result > 0) {
		  /* read in BSDL file if IDCODE matched */
		  printf( _("  Filename:     %s\n"), name );
		  result = bsdl_read_file(name, 1, idcode);
		}
	      } else
		result = bsdl_read_file(name, mode, idcode);
            }
          }

          free(name);
        }
      }

      closedir(dir);
    } else
      bsdl_msg(BSDL_MSG_WARN, _("Cannot open directory %s\n"), bsdl_path_list[idx]);

    idx++;
  }

  return result;
}
