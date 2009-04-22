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

#include "cable.h"
#include "tap.h"
#include "chain.h"

#include "jtag.h"

#define	DETECT_PATTERN_SIZE	8
#define	MAX_REGISTER_LENGTH	1024
#define	TEST_COUNT		1
#define	TEST_THRESHOLD		100     /* in % */

#undef VERY_LOW_LEVEL_DEBUG

int
detect_register_size (chain_t *chain)
{
    int len;
    tap_register *rz;
    tap_register *rout;
    tap_register *rpat;

    /* This seems to be a good place to check if TDO changes at all */
    int tdo, tdo_stuck = -2;

    for (len = 1; len <= MAX_REGISTER_LENGTH; len++)
    {
        int p;
        int ok = 0;

        rz = register_fill (register_alloc (len), 0);
        rout = register_alloc (DETECT_PATTERN_SIZE + len);
        rpat =
            register_inc (register_fill
                          (register_alloc (DETECT_PATTERN_SIZE + len), 0));

        for (p = 1; p < (1 << DETECT_PATTERN_SIZE); p++)
        {
            int i;
            const char *s;
            ok = 0;

            s = register_get_string (rpat);
            while (*s)
                s++;

            for (i = 0; i < TEST_COUNT; i++)
            {
                tap_shift_register (chain, rz, NULL, 0);
                tap_shift_register (chain, rpat, rout, 0);

#ifdef VERY_LOW_LEVEL_DEBUG
                printf (">>> %s\n", register_get_string (rz));
                printf ("  + %s\n", register_get_string (rpat));
#endif
                tdo = register_all_bits_same_value (rout);
                if (tdo_stuck == -2)
                    tdo_stuck = tdo;
                if (tdo_stuck != tdo)
                    tdo_stuck = -1;

                register_shift_right (rout, len);
                if (register_compare (rpat, rout) == 0)
                    ok++;
#ifdef VERY_LOW_LEVEL_DEBUG
                printf ("  = %s => %d\n", register_get_string (rout), ok);
#endif
            }
            if (100 * ok / TEST_COUNT < TEST_THRESHOLD)
            {
                ok = 0;
                break;
            }

            register_inc (rpat);
        }

        register_free (rz);
        register_free (rout);
        register_free (rpat);

        if (ok)
            return len;
    }

    if (tdo_stuck >= 0)
    {
        printf (_("Warning: TDO seems to be stuck at %d\n"), tdo_stuck);
    }

    return -1;
}

static void
jtag_reset (chain_t *chain)
{
    chain_set_trst (chain, 0);
    chain_set_trst (chain, 1);

    tap_reset (chain);
}

void
discovery (chain_t *chain)
{
    int irlen;
    tap_register *ir;
    tap_register *irz;

    /* detecting IR size */
    jtag_reset (chain);

    printf (_("Detecting IR length ... "));
    fflush (stdout);

    tap_capture_ir (chain);
    irlen = detect_register_size (chain);

    printf (_("%d\n"), irlen);

    if (irlen < 1)
    {
        printf (_("Error: Invalid IR length!\n"));
        return;
    }

    /* all 1 is BYPASS in all parts, so DR length gives number of parts */
    ir = register_fill (register_alloc (irlen), 1);
    irz = register_duplicate (ir);

    if (!ir || !irz)
    {
        register_free (ir);
        register_free (irz);
        printf (_("Error: Out of memory!\n"));
        return;
    }

    for (;;)
    {
        int rs;

        jtag_reset (chain);

        tap_capture_ir (chain);
        tap_shift_register (chain, ir, NULL, 1);

        printf (_("Detecting DR length for IR %s ... "),
                register_get_string (ir));
        fflush (stdout);

        tap_capture_dr (chain);
        rs = detect_register_size (chain);

        printf (_("%d\n"), rs);

        register_inc (ir);
        if (register_compare (ir, irz) == 0)
            break;
    }
    register_free (ir);
    register_free (irz);
}
