/*
 * $Id$
 *
 * Copyright (C) 2003 ETC s.r.o.
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
 * Written by Marcel Telka <marcel@telka.sk>, 2003.
 *
 */

#include "sysdep.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cable.h"
#include "chain.h"
#include "generic.h"
#include "generic_usbconn.h"

#include <cmd.h>

#undef VERBOSE

#ifdef ENABLE_CABLE_XPC
extern usbconn_cable_t usbconn_cable_xpc_int;
extern usbconn_cable_t usbconn_cable_xpc_ext;
#endif
#ifdef ENABLE_CABLE_JLINK
extern usbconn_cable_t usbconn_cable_jlink;
#endif
#ifdef ENABLE_CABLE_FT2232
#ifdef ENABLE_LOWLEVEL_FTD2XX
extern usbconn_cable_t usbconn_cable_ft2232_ftd2xx;
extern usbconn_cable_t usbconn_cable_armusbocd_ftd2xx;
extern usbconn_cable_t usbconn_cable_gnice_ftd2xx;
extern usbconn_cable_t usbconn_cable_jtagkey_ftd2xx;
extern usbconn_cable_t usbconn_cable_oocdlinks_ftd2xx;
extern usbconn_cable_t usbconn_cable_turtelizer2_ftd2xx;
extern usbconn_cable_t usbconn_cable_usbtojtagif_ftd2xx;
extern usbconn_cable_t usbconn_cable_signalyzer_ftd2xx;
#endif
#ifdef ENABLE_LOWLEVEL_FTDI
extern usbconn_cable_t usbconn_cable_ft2232_ftdi;
extern usbconn_cable_t usbconn_cable_armusbocd_ftdi;
extern usbconn_cable_t usbconn_cable_gnice_ftdi;
extern usbconn_cable_t usbconn_cable_jtagkey_ftdi;
extern usbconn_cable_t usbconn_cable_oocdlinks_ftdi;
extern usbconn_cable_t usbconn_cable_turtelizer2_ftdi;
extern usbconn_cable_t usbconn_cable_usbtojtagif_ftdi;
extern usbconn_cable_t usbconn_cable_signalyzer_ftdi;
#endif
#endif
#ifdef ENABLE_CABLE_USBBLASTER
#ifdef ENABLE_LOWLEVEL_FTD2XX
extern usbconn_cable_t usbconn_cable_usbblaster_ftd2xx;
extern usbconn_cable_t usbconn_cable_cubic_cyclonium_ftd2xx;
extern usbconn_cable_t usbconn_cable_nios_eval_ftd2xx;
extern usbconn_cable_t usbconn_cable_usb_jtag_ftd2xx;
#endif
#ifdef ENABLE_LOWLEVEL_FTDI
extern usbconn_cable_t usbconn_cable_usbblaster_ftdi;
extern usbconn_cable_t usbconn_cable_cubic_cyclonium_ftdi;
extern usbconn_cable_t usbconn_cable_nios_eval_ftdi;
extern usbconn_cable_t usbconn_cable_usb_jtag_ftdi;
#endif
#endif

usbconn_cable_t *usbconn_cables[] =
{
#ifdef ENABLE_CABLE_XPC
	&usbconn_cable_xpc_int,
	&usbconn_cable_xpc_ext,
#endif
#ifdef ENABLE_CABLE_JLINK
	&usbconn_cable_jlink,
#endif
#ifdef ENABLE_CABLE_FT2232
#ifdef ENABLE_LOWLEVEL_FTD2XX
	&usbconn_cable_ft2232_ftd2xx,
	&usbconn_cable_armusbocd_ftd2xx,
	&usbconn_cable_gnice_ftd2xx,
	&usbconn_cable_jtagkey_ftd2xx,
	&usbconn_cable_oocdlinks_ftd2xx,
	&usbconn_cable_turtelizer2_ftd2xx,
	&usbconn_cable_usbtojtagif_ftd2xx,
	&usbconn_cable_signalyzer_ftd2xx,
#endif
#ifdef ENABLE_LOWLEVEL_FTDI
	&usbconn_cable_ft2232_ftdi,
	&usbconn_cable_armusbocd_ftdi,
	&usbconn_cable_gnice_ftdi,
	&usbconn_cable_jtagkey_ftdi,
	&usbconn_cable_oocdlinks_ftdi,
	&usbconn_cable_turtelizer2_ftdi,
	&usbconn_cable_usbtojtagif_ftdi,
	&usbconn_cable_signalyzer_ftdi,
#endif
#endif
#ifdef ENABLE_CABLE_USBBLASTER
#ifdef ENABLE_LOWLEVEL_FTD2XX
	&usbconn_cable_usbblaster_ftd2xx,
	&usbconn_cable_cubic_cyclonium_ftd2xx,
	&usbconn_cable_nios_eval_ftd2xx,
	&usbconn_cable_usb_jtag_ftd2xx,
#endif
#ifdef ENABLE_LOWLEVEL_FTDI
	&usbconn_cable_usbblaster_ftdi,
	&usbconn_cable_cubic_cyclonium_ftdi,
	&usbconn_cable_nios_eval_ftdi,
	&usbconn_cable_usb_jtag_ftdi,
#endif
#endif
	NULL
};

int
generic_usbconn_connect( char *params[], cable_t *cable )
{
	usbconn_cable_t user_specified = {
		NULL,	/* no name */
		NULL,	/* no string pattern */
		NULL,	/* no specific driver */
		-1,		/* no VID */
		-1,		/* no PID */
	};

	int paramc = cmd_params( params );
	generic_params_t *cable_params;
	usbconn_t *conn = NULL;
	int i;

	if(strcasecmp(params[0], "usb") != 0)
	{
		user_specified.name = params[0];
	}

	/* parse arguments beyond the cable name */
	for (i = 1; i < paramc; i++)
	{
		if(strncasecmp("pid=", params[i], 4) == 0)
		{
			user_specified.pid = strtol( params[i] + 4, NULL, 16 );
		}
		else if(strncasecmp("vid=", params[i], 4) == 0)
		{
			user_specified.vid = strtol( params[i] + 4, NULL, 16 );
		}
		else if(strncasecmp("desc=", params[i], 5) == 0)
		{
			user_specified.desc = params[i] + 5;
		}
		else if(strncasecmp("driver=", params[i], 7) == 0)
		{
			user_specified.driver = params[i] + 7;
		}
	}

	/* search usbconn driver list */
	for (i = 0; usbconn_drivers[i] && !conn; i++)
	{
		if ((user_specified.driver == NULL)
			|| (strcasecmp(user_specified.driver, usbconn_drivers[i]->type) == 0))
		{
			int j;

			/* search cable list */
			for (j = 0; usbconn_cables[j] && !conn; j++)
			{
				if((user_specified.name == NULL)
					|| (strcasecmp(user_specified.name, usbconn_cables[j]->name) == 0))
				{
					if(strcasecmp(usbconn_cables[j]->driver, usbconn_drivers[i]->type) == 0)
					{
						usbconn_cable_t cable_try = *(usbconn_cables[j]);

						if(user_specified.vid >= 0)  cable_try.vid  = user_specified.vid;
						if(user_specified.pid >= 0)  cable_try.pid  = user_specified.pid;
						if(user_specified.desc != 0) cable_try.desc = user_specified.desc;

						conn = usbconn_drivers[i]->connect( (const char **) &params[1],
							paramc - 1, &cable_try );
					}
				}
			}
		}
	}

	if (!conn) {
		printf( _("Couldn't connect to suitable USB device.\n") );
		return 2;
	}

	cable_params = malloc( sizeof(generic_params_t) );
	if (!cable_params) {
		printf( _("%s(%d) malloc failed!\n"), __FILE__, __LINE__);
		usbconn_drivers[i]->free( conn );
		return 4;
	}

	cable->link.usb = conn;
	cable->params = cable_params;
	cable->chain = NULL;

	return 0;
}

void
generic_usbconn_free( cable_t *cable )
{
	cable->link.usb->driver->free( cable->link.usb );
	free( cable->params );
	free( cable );
}

void
generic_usbconn_done( cable_t *cable )
{
	usbconn_close( cable->link.usb );
}

void
generic_usbconn_help( const char *cablename )
{
	printf( _(
		"Usage: cable %s [vid=VID] [pid=PID] [desc=DESC] [...]\n"
		"\n"
		"VID        USB Device Vendor ID (hex, e.g. 0abc)\n"
		"PID        USB Device Product ID (hex, e.g. 0abc)\n"
		"DESC       Some string to match in description or serial no.\n"
		"\n"
	),
    cablename 
    );
}

