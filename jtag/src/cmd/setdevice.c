/*
 * $Id: setdevice.c,v 1.0 20/09/2006 12:38:01  $
 *
 * Copyright (C) 2006 Kila Medical Systems.
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
 * Written by Ajith Kumar P.C <ajithpc@kila.com>
 *
*/


#include "sysdep.h"
#include <stdio.h>
#include <string.h>
#include "jtag.h"
#include "cmd.h"
#include "bus.h"
#include "setdevice.h"

extern chain_t *chain;
extern forced_detection_t var_forced_detection;

static int cmd_setdevice_run( char *params[] )
{
	if(cmd_params(params) != 2)
		return -1;
	if (strcmp( params[1], "sharc21065L" ) == 0)
	{
#if 0
		buses_free();
		parts_free( chain->parts );
		chain->parts = NULL;
#endif
		var_forced_detection.deviceID = SHARC_21065L;
		printf( _("Device is set to SHARC 21065L\n") );
	}
#if 0
	else
	{
		buses_free();
		parts_free( chain->parts );
		chain->parts = NULL;
		var_forced_detection.deviceID = AUTO_DETECT;
		printf( _("Automatic Device selection\n") );	
	}
#endif

#if 1
	else
	{	
				
		if(var_forced_detection.deviceID == SHARC_21065L)
		{
			printf( _("Support is only for SHARC 21065L\n") );
			printf( _("If you want to try another device:\n") );	
			printf( _("\tPlease restart the JTAG tool\n\n") );	
		}
		else
		{
			var_forced_detection.deviceID = AUTO_DETECT;
			//printf( _("Supports devices which posses device ID\n") );
			printf( _("If you want to try SHARC 21065L, type command:\n") );	
			printf( _("\tsetdevice sharc21065L  \n\n") );
		}
	}
#endif
	return 1;
}

static void cmd_setdevice_help( void )
{
	printf( _("Provision for supporting devices which do not have device ID\n"
		"Usage: %s sharc21065L\n"
			"\t For setting to SHARC Processor\n"
#if 0
		"Or %s auto\n"	 
			"\t For auto selection of the device\n"
		"Default is Auto Selection\n"
#endif
		),"setdevice" );
}

cmd_t cmd_setdevice = {
	"setdevice",
	N_("Provision for supporting devices which do not have device ID\n"),
	cmd_setdevice_help,
	cmd_setdevice_run
};

	
