/*
 * $Id$
 *
 * JTAG target simulator JIM "cable" driver
 *
 * Copyright (C) 2008 Kolja Waschk
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
 */

#include <sysdep.h>

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <urjtag/cable.h>
#include <urjtag/parport.h>
#include <urjtag/chain.h>

#include "generic.h"

#include <urjtag/cmd.h>

#include <urjtag/jim.h>

/* private parameters of this cable driver */
typedef struct
{
    urj_jim_state_t *s;
}
jim_cable_params_t;

static int
jim_cable_connect (urj_cable_t *cable, const urj_param_t *params[])
{
    jim_cable_params_t *cable_params;
    urj_jim_state_t *s;

    if (urj_param_num (params) > 0)
    {
        urj_error_set (URJ_ERROR_SYNTAX, _("too many arguments"));
        return URJ_STATUS_FAIL;
    }

    urj_warning (_("JTAG target simulator JIM - work in progress!\n"));

    s = urj_jim_init ();
    if (!s)
    {
        // retain error state
        return URJ_STATUS_FAIL;
    }

    cable_params = malloc (sizeof (jim_cable_params_t));
    if (!cable_params)
    {
        urj_error_set (URJ_ERROR_OUT_OF_MEMORY, _("malloc(%zd) fails"),
                       sizeof (jim_cable_params_t));
        urj_jim_free (s);
        return URJ_STATUS_FAIL;
    }

    cable->params = cable_params;
    ((jim_cable_params_t *) (cable->params))->s = s;
    cable->chain = NULL;

    return URJ_STATUS_OK;
}

static void
jim_cable_disconnect (urj_cable_t *cable)
{
    urj_tap_cable_done (cable);
    urj_tap_chain_disconnect (cable->chain);
}

static void
jim_cable_free (urj_cable_t *cable)
{
    if (cable->params != NULL)
    {
        urj_jim_free (((jim_cable_params_t *) (cable->params))->s);
        free (cable->params);
    }
    free (cable);
}

static void
jim_cable_done (urj_cable_t *cable)
{
}

static int
jim_cable_init (urj_cable_t *cable)
{
    return URJ_STATUS_OK;
}

static void
jim_cable_clock (urj_cable_t *cable, int tms, int tdi, int n)
{
    int i;
    jim_cable_params_t *jcp = cable->params;

    for (i = 0; i < n; i++)
    {
        urj_jim_tck_rise (jcp->s, tms, tdi);
        urj_jim_tck_fall (jcp->s);
    }
}

static int
jim_cable_get_tdo (urj_cable_t *cable)
{
    jim_cable_params_t *jcp = cable->params;

    return urj_jim_get_tdo (jcp->s);
}

static int
jim_cable_get_trst (urj_cable_t *cable, urj_pod_sigsel_t sig)
{
    /* XXX: Doesn't handle sig ? */
    jim_cable_params_t *jcp = cable->params;

    return urj_jim_get_trst (jcp->s);
}

static int
jim_cable_set_trst (urj_cable_t *cable, int mask, int val)
{
    /* XXX: Doesn't handle mask ? */
    jim_cable_params_t *jcp = cable->params;

    urj_jim_set_trst (jcp->s, val);
    return urj_jim_get_trst (jcp->s);
}

static void
jim_cable_help (urj_log_level_t ll, const char *cablename)
{
    urj_log (ll, _("Usage: cable %s\n"), cablename);
}

const urj_cable_driver_t urj_tap_cable_jim_driver = {
    "JIM",
    N_("JTAG target simulator JIM"),
    URJ_CABLE_DEVICE_OTHER,
    { .other = jim_cable_connect, },
    jim_cable_disconnect,
    jim_cable_free,
    jim_cable_init,
    jim_cable_done,
    urj_tap_cable_generic_set_frequency,
    jim_cable_clock,
    jim_cable_get_tdo,
    urj_tap_cable_generic_transfer,
    jim_cable_set_trst,
    jim_cable_get_trst,
    urj_tap_cable_generic_flush_using_transfer,
    jim_cable_help
};
