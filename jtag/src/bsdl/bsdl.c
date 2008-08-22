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

#include "jtag.h"
#include "cmd.h"

//#include "bsdl_local.h"
#include "bsdl_types.h"
#include "vhdl_parser.h"
#include "bsdl_parser.h"

#include "bsdl_msg.h"

#ifdef DMALLOC
#include "dmalloc.h"
#endif


/*****************************************************************************
 * bsdl_msg( type, format, ... )
 *
 * Main printing function for the BSDL subsystem.
 *
 * Parameters
 *   type   : one of the BSDL_MSG_* defines, determines message tag
 *   format : printf format
 *   ...    : additional parameters to fill the printf format string
 *
 * Returns
 *   void
 ****************************************************************************/
void bsdl_msg( int type, const char *format, ... )
{
  va_list lst;

  va_start( lst, format );
  switch (type)
  {
    case BSDL_MSG_NOTE:
      printf( "-N- " );
      break;
    case BSDL_MSG_WARN:
      printf( "-W- " );
      break;
    case BSDL_MSG_ERR:
      printf( "-E- " );
      break;
    case BSDL_MSG_FATAL:
      printf( "-F- " );
      break;
    default:
      printf( "-?- " );
      break;
  }
  vprintf( format, lst );
  va_end( lst );
}


/*****************************************************************************
 * bsdl_read_file( chain, BSDL_File_Name, mode, idcode )
 *
 * Read, parse and optionally apply contents of BSDL file.
 *
 * Parameters
 *    chain  : pointer to active chain structure
 *    BSDL_File_Name : name of BSDL file to read
 *    mode   : -1 -> read file
 *                   no further action based on components
 *              0 -> read file and extract all components
 *                   dump commands to stdout, do not execute commands
 *              1 -> read file and extract all components
 *                   execute commands
 *    idcode : reference idcode string
 *
 * Returns
 *   < 0 : Error occured, parse/syntax problems or out of memory
 *   = 0 : No errors, idcode not checked or mismatching
 *   > 0 : No errors, idcode checked and matched
 *
 ****************************************************************************/
int bsdl_read_file( chain_t *chain, const char *BSDL_File_Name, int mode,
                    const char *idcode )
{
  bsdl_globs_t *globs = &(chain->bsdl);
  FILE *BSDL_File;
  vhdl_parser_priv_t *vhdl_parser_priv;
  bsdl_parser_priv_t *bsdl_parser_priv;
  jtag_ctrl_t jtag_ctrl;
  int Compile_Errors = 1;
  int idcode_match = 0;

  jtag_ctrl.mode = mode;
  jtag_ctrl.debug = globs->debug;

  /* perform some basic checks */
  if (mode >= 0)
  {
    if (mode >= 1)
    {
      if (chain == NULL)
      {
	bsdl_msg( BSDL_MSG_ERR, _("No JTAG chain available\n") );
	return -1;
      }
      if (chain->parts == NULL)
      {
	bsdl_msg( BSDL_MSG_ERR, _("Chain without any parts\n") );
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
  }
  else
  {
    jtag_ctrl.chain = NULL;
    jtag_ctrl.part = NULL;
  }

  BSDL_File = fopen( BSDL_File_Name, "r" );

  if (globs->debug || (mode == 0))
    bsdl_msg( BSDL_MSG_NOTE, _("Reading file '%s'\n"), BSDL_File_Name );

  if (BSDL_File == NULL) {
    bsdl_msg( BSDL_MSG_ERR, _("Unable to open BSDL file '%s'\n"), BSDL_File_Name );
    return -1;
  }

  if ((vhdl_parser_priv = vhdl_parser_init( BSDL_File, &jtag_ctrl )))
  {
    vhdl_parser_priv->jtag_ctrl->idcode = NULL;

    vhdlparse( vhdl_parser_priv );

    Compile_Errors = vhdl_flex_get_compile_errors( vhdl_parser_priv->scanner );
    if (Compile_Errors == 0)
    {
      if (globs->debug)
        bsdl_msg( BSDL_MSG_NOTE, _("BSDL file '%s' passed VHDL stage correctly\n"),
		 BSDL_File_Name );

      if ((bsdl_parser_priv = bsdl_parser_init( &jtag_ctrl )))
      {

        Compile_Errors = bsdl_sem_process_elements( bsdl_parser_priv );

        if ((Compile_Errors == 0) && globs->debug)
          bsdl_msg( BSDL_MSG_NOTE, _("BSDL file '%s' passed BSDL stage correctly\n"),
                    BSDL_File_Name );


        /* handle IDCODE comparison */
        if ((Compile_Errors == 0) && jtag_ctrl.idcode)
        {
          if (globs->debug)
            bsdl_msg( BSDL_MSG_NOTE, _("Got IDCODE: %s\n"), jtag_ctrl.idcode );

          /* should we compare the idcodes? */
          if (idcode)
          {
            if (strlen( idcode ) == strlen(jtag_ctrl.idcode))
            {
              int idx;

              /* compare given idcode with idcode from BSDL file
                 including the end of string character */
              idcode_match = 1;
              for (idx = 0; idx <= strlen( idcode ); idx++)
                if (jtag_ctrl.idcode[idx] != 'X')
                  if (idcode[idx] != jtag_ctrl.idcode[idx])
                    idcode_match = 0;

              if (globs->debug)
              {
                if (idcode_match)
                  bsdl_msg( BSDL_MSG_NOTE, _("IDCODE matched\n") );
                else
                  bsdl_msg( BSDL_MSG_NOTE, _("IDCODE mismatch\n") );
              }
            }
          }
        }


        bsdl_parser_deinit( bsdl_parser_priv );
      }
    }
    else
    {
      if (globs->debug || (mode >= 0))
        bsdl_msg( BSDL_MSG_ERR, _("BSDL file '%s' contains errors in VHDL stage, stopping\n"),
		 BSDL_File_Name );
    }


    vhdl_parser_deinit( vhdl_parser_priv );
  }

  return Compile_Errors == 0 ? idcode_match : -1;
}


/*****************************************************************************
 * void bsdl_set_path( chain, pathlist )
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
void bsdl_set_path( chain_t *chain, const char *pathlist )
{
  bsdl_globs_t *globs = &(chain->bsdl);
  char  *delim;
  char  *elem;
  char  *pathelem;
  int    num;
  size_t len;

  /* free memory of current path list */
  if (globs->path_list)
  {
    for (num = 0; globs->path_list[num]; num++)
      if (globs->path_list[num])
        free( globs->path_list[num] );
    free( globs->path_list );
    globs->path_list = NULL;
  }

  /* run through path list and determine number of elements */
  for (num = 0, elem = (char *)pathlist; strlen(elem) > 0; )
  {
    delim = strchr( elem, ';' );
    if ((delim - elem > 0) || (delim == NULL))
    {
      num++;
      /* extend path list array */
      globs->path_list = (char **)realloc( globs->path_list, (num+1) * sizeof(char *) );
      /* enter path element up to the delimeter */
      if (delim == NULL)
        len = strlen( elem );
      else
        len = delim-elem;
      pathelem = malloc( len + 1 );
      memcpy( pathelem, elem, len );
      pathelem[len] = '\0';
      globs->path_list[num-1] = pathelem;
      globs->path_list[num] = NULL;
    }
    elem = delim ? delim + 1 : elem + strlen( elem );
  }

  if (globs->debug)
    for (num = 0; globs->path_list[num] != NULL; num++)
      bsdl_msg( BSDL_MSG_NOTE, "%s\n", globs->path_list[num] );
}


/*****************************************************************************
 * bsdl_scan_files( chain, idcode, mode )
 *
 * Scans through all files found via the elements in bsdl_path_list
 * and does a test read on each of them.
 * If mode >= 1 is requested, it will read the first BSDL file with matching
 * idcode in "execute" mode. I.e. all extracted statements are applied to
 * the current part.
 *
 * Parameters
 *   chain  : pointer to active chain structure
 *   idcode : reference idcode string
 *   mode   : -1 -> read file
 *                  no further action based on components
 *             0 -> read file and extract all components
 *                  dump commands to stdout, do not execute commands
 *             1 -> read file and extract all components
 *                  execute commands
 *
 * Returns
 *   < 0 : Error occured, parse/syntax problems or out of memory
 *   = 0 : No errors, idcode not checked or mismatching
 *   > 0 : No errors, idcode checked and matched
 *
 ****************************************************************************/
int bsdl_scan_files( chain_t *chain, const char *idcode, int mode )
{
  bsdl_globs_t *globs = &(chain->bsdl);
  int idx = 0;
  int result = 0;

  /* abort if no path list was specified */
  if (globs->path_list == NULL)
    return 0;

  while (globs->path_list[idx] && (result <= 0))
  {
    DIR *dir;

    if ((dir = opendir( globs->path_list[idx] )))
    {
      struct dirent *elem;

      /* run through all elements in the current directory */
      while ((elem = readdir( dir )) && (result <= 0))
      {
        char *name;

        name = (char *)malloc( strlen( globs->path_list[idx] )
                               + strlen( elem->d_name ) + 1 + 1 );
        if (name)
        {
          struct stat buf;

          strcpy( name, globs->path_list[idx] );
          strcat( name, "/" );
          strcat( name, elem->d_name );

          if (stat( name, &buf ) == 0)
          {
            if (buf.st_mode & S_IFREG)
            {
	      if (mode >= 1)
              {
		/* now we know we can finally read the file */
		/* do a test read first */
		result = bsdl_read_file( chain, name, -1, idcode );
		if (result > 0)
                {
		  /* read in BSDL file if IDCODE matched */
		  printf( _("  Filename:     %s\n"), name );
		  result = bsdl_read_file( chain, name, 1, idcode );
		}
	      }
              else
		result = bsdl_read_file( chain, name, mode, idcode );
            }
          }

          free( name );
        }
      }

      closedir( dir );
    }
    else
      bsdl_msg( BSDL_MSG_WARN, _("Cannot open directory %s\n"), globs->path_list[idx] );

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
