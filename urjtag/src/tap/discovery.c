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

#include <sysdep.h>

#include <stdio.h>
#include <stdlib.h>

#include <urjtag/tap.h>
#include <urjtag/tap_register.h>
#include <urjtag/chain.h>


#define DETECT_PATTERN_SIZE     8
#define DEFAULT_MAX_REGISTER_LENGTH 1024
#define TEST_COUNT              1
#define TEST_THRESHOLD          100     /* in % */

#undef VERY_LOW_LEVEL_DEBUG

int
urj_tap_detect_register_size (urj_chain_t *chain, int maxlen)
{
    int len;
    urj_tap_register_t *rz;
    urj_tap_register_t *rout;
    urj_tap_register_t *rpat;

    if (maxlen == 0)
        maxlen = DEFAULT_MAX_REGISTER_LENGTH;

    /* This seems to be a good place to check if TDO changes at all */
    int tdo, tdo_stuck = -2;

    for (len = 1; len <= maxlen; len++)
    {
        int p;
        int ok = 0;

        rz = urj_tap_register_fill (urj_tap_register_alloc (len), 0);
        rout = urj_tap_register_alloc (DETECT_PATTERN_SIZE + len);
        rpat =
            urj_tap_register_inc (urj_tap_register_fill
                                  (urj_tap_register_alloc
                                   (DETECT_PATTERN_SIZE + len), 0));

        for (p = 1; p < (1 << DETECT_PATTERN_SIZE); p++)
        {
            int i;
            const char *s;
            ok = 0;

            s = urj_tap_register_get_string (rpat);
            while (*s)
                s++;

            for (i = 0; i < TEST_COUNT; i++)
            {
                urj_tap_shift_register (chain, rz, NULL, 0);
                urj_tap_shift_register (chain, rpat, rout, 0);

#ifdef VERY_LOW_LEVEL_DEBUG
                urj_log (URJ_LOG_LEVEL_ALL, ">>> %s\n", urj_tap_register_get_string (rz));
                urj_log (URJ_LOG_LEVEL_ALL, "  + %s\n", urj_tap_register_get_string (rpat));
#endif
                tdo = urj_tap_register_all_bits_same_value (rout);
                if (tdo_stuck == -2)
                    tdo_stuck = tdo;
                if (tdo_stuck != tdo)
                    tdo_stuck = -1;

                urj_tap_register_shift_right (rout, len);
                if (urj_tap_register_compare (rpat, rout) == 0)
                    ok++;
#ifdef VERY_LOW_LEVEL_DEBUG
                urj_log (URJ_LOG_LEVEL_ALL, "  = %s => %d\n", urj_tap_register_get_string (rout),
                        ok);
#endif
            }
            if (100 * ok / TEST_COUNT < TEST_THRESHOLD)
            {
                ok = 0;
                break;
            }

            urj_tap_register_inc (rpat);
        }

        urj_tap_register_free (rz);
        urj_tap_register_free (rout);
        urj_tap_register_free (rpat);

        if (ok)
            return len;
    }

    if (tdo_stuck >= 0)
    {
        urj_warning (_("TDO seems to be stuck at %d\n"), tdo_stuck);
    }

    return -1;
}

int
urj_tap_discovery (urj_chain_t *chain)
{
    int irlen;
    urj_tap_register_t *ir;
    urj_tap_register_t *irz;

    /* detecting IR size */
    urj_tap_trst_reset (chain);

    urj_log (URJ_LOG_LEVEL_NORMAL, _("Detecting IR length ... "));
    fflush (stdout);

    urj_tap_capture_ir (chain);
    irlen = urj_tap_detect_register_size (chain, 0);

    urj_log (URJ_LOG_LEVEL_NORMAL, _("%d\n"), irlen);

    if (irlen < 1)
    {
        // retain error state
        urj_log (URJ_LOG_LEVEL_NORMAL, _("Error: Invalid IR length!\n"));
        return URJ_STATUS_FAIL;
    }

    /* all 1 is BYPASS in all parts, so DR length gives number of parts */
    ir = urj_tap_register_fill (urj_tap_register_alloc (irlen), 1);
    irz = urj_tap_register_duplicate (ir);

    if (!ir || !irz)
    {
        urj_tap_register_free (ir);
        urj_tap_register_free (irz);
        return URJ_STATUS_FAIL;
    }

    for (;;)
    {
        int rs;

        urj_tap_trst_reset (chain);

        urj_tap_capture_ir (chain);
        urj_tap_shift_register (chain, ir, NULL, 1);

        urj_log (URJ_LOG_LEVEL_NORMAL, _("Detecting DR length for IR %s ... "),
                urj_tap_register_get_string (ir));
        fflush (stdout);

        urj_tap_capture_dr (chain);
        rs = urj_tap_detect_register_size (chain, 0);

        urj_log (URJ_LOG_LEVEL_NORMAL, _("%d\n"), rs);

        urj_tap_register_inc (ir);
        if (urj_tap_register_compare (ir, irz) == 0)
            break;
    }
    urj_tap_register_free (ir);
    urj_tap_register_free (irz);

    return URJ_STATUS_OK;
}
