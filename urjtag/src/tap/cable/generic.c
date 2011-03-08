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
#include <string.h>
#include <assert.h>
#include <math.h>

#include <urjtag/cable.h>
#include <urjtag/parport.h>
#include <urjtag/chain.h>
#include <urjtag/fclock.h>

#include "generic.h"

#include <urjtag/cmd.h>

static void
print_vector (urj_log_level_t ll, int len, char *vec)
{
    int i;
    for (i = 0; i < len; i++)
        urj_log (ll, "%c", vec[i] ? '1' : '0');
}


void
urj_tap_cable_generic_disconnect (urj_cable_t *cable)
{
    urj_tap_cable_done (cable);
    urj_tap_chain_disconnect (cable->chain);
    cable->chain = NULL;
}

int
urj_tap_cable_generic_transfer (urj_cable_t *cable, int len, const char *in,
                                char *out)
{
    int i;

    if (out)
        for (i = 0; i < len; i++)
        {
            out[i] = cable->driver->get_tdo (cable);
            cable->driver->clock (cable, 0, in[i], 1);
        }
    else
        for (i = 0; i < len; i++)
        {
            cable->driver->clock (cable, 0, in[i], 1);
        }

    return i;
}

int
urj_tap_cable_generic_get_signal (urj_cable_t *cable, urj_pod_sigsel_t sig)
{
    return (((PARAM_SIGNALS (cable)) & sig) != 0) ? 1 : 0;
}

static int
do_one_queued_action (urj_cable_t *cable)
{
    int i;

    urj_log (URJ_LOG_LEVEL_DEBUG, "do_one_queued\n");

    if ((i = urj_tap_cable_get_queue_item (cable, &cable->todo)) >= 0)
    {
        int j;

        if (cable->done.num_items >= cable->done.max_items)
        {
            if (cable->todo.data[i].action == URJ_TAP_CABLE_GET_TDO
                || cable->todo.data[i].action == URJ_TAP_CABLE_GET_SIGNAL
                || cable->todo.data[i].action == URJ_TAP_CABLE_TRANSFER)
            {
                urj_error_set (URJ_ERROR_OUT_OF_BOUNDS,
                               _("No space in cable activity results queue"));
                urj_tap_cable_purge_queue (&cable->done, 1);
                /* @@@@ RFHH shouldn't we bail out? */
            }
        }

        switch (cable->todo.data[i].action)
        {
        case URJ_TAP_CABLE_CLOCK:
            cable->driver->clock (cable,
                                  cable->todo.data[i].arg.clock.tms,
                                  cable->todo.data[i].arg.clock.tdi,
                                  cable->todo.data[i].arg.clock.n);
            break;
        case URJ_TAP_CABLE_SET_SIGNAL:
            urj_tap_cable_set_signal (cable,
                                      cable->todo.data[i].arg.value.sig,
                                      cable->todo.data[i].arg.value.val);
            break;
        case URJ_TAP_CABLE_TRANSFER:
            {
                /* @@@@ RFHH check result */
                int r = cable->driver->transfer (cable,
                                                 cable->todo.data[i].arg.
                                                 transfer.len,
                                                 cable->todo.data[i].arg.
                                                 transfer.in,
                                                 cable->todo.data[i].arg.
                                                 transfer.out);

                free (cable->todo.data[i].arg.transfer.in);
                if (cable->todo.data[i].arg.transfer.out != NULL)
                {
                    /* @@@@ RFHH check result */
                    j = urj_tap_cable_add_queue_item (cable, &cable->done);
                    urj_log (URJ_LOG_LEVEL_DEBUG,
                             "add result from transfer to %p.%d (out=%p)\n",
                             &cable->done, j,
                             cable->todo.data[i].arg.transfer.out);
                    cable->done.data[j].action = URJ_TAP_CABLE_TRANSFER;
                    cable->done.data[j].arg.xferred.len =
                        cable->todo.data[i].arg.transfer.len;
                    cable->done.data[j].arg.xferred.res = r;
                    cable->done.data[j].arg.xferred.out =
                        cable->todo.data[i].arg.transfer.out;
                }
                break;
            }
        case URJ_TAP_CABLE_GET_TDO:
            /* @@@@ RFHH check result */
            j = urj_tap_cable_add_queue_item (cable, &cable->done);
            urj_log (URJ_LOG_LEVEL_DEBUG,
                     "add result from get_tdo to %p.%d\n", &cable->done, j);
            cable->done.data[j].action = URJ_TAP_CABLE_GET_TDO;
            cable->done.data[j].arg.value.val =
                cable->driver->get_tdo (cable);
            break;
        case URJ_TAP_CABLE_GET_SIGNAL:
            /* @@@@ RFHH check result */
            j = urj_tap_cable_add_queue_item (cable, &cable->done);
            urj_log (URJ_LOG_LEVEL_DEBUG,
                     "add result from get_signal to %p.%d\n", &cable->done,
                     j);
            cable->done.data[j].action = URJ_TAP_CABLE_GET_SIGNAL;
            cable->done.data[j].arg.value.sig =
                cable->todo.data[i].arg.value.sig;
            cable->done.data[j].arg.value.val =
                cable->driver->get_signal (cable,
                                           cable->todo.data[i].arg.value.sig);
            break;
        case URJ_TAP_CABLE_CLOCK_COMPACT: /* Turn off GCC warning */
            break;
        }
        urj_log (URJ_LOG_LEVEL_DEBUG, "do_one_queued done\n");

        return 1;
    }
    urj_log (URJ_LOG_LEVEL_DEBUG, "do_one_queued abort\n");

    return 0;
}

void
urj_tap_cable_generic_flush_one_by_one (urj_cable_t *cable,
                                        urj_cable_flush_amount_t how_much)
{
    /* This will flush always, even if how_much == URJ_TAP_CABLE_OPTIONALLY,
     * because there is no reason to let the queue grow */

    while (do_one_queued_action (cable));
}

void
urj_tap_cable_generic_flush_using_transfer (urj_cable_t *cable,
                                            urj_cable_flush_amount_t how_much)
{
    int i, j, n;
    char *in, *out;

    if (how_much == URJ_TAP_CABLE_OPTIONALLY)
        return;

    if (cable->todo.num_items == 0)
        return;

    do
    {
        int r, bits = 0, tdo = 0, savbits;

        urj_log (URJ_LOG_LEVEL_DETAIL, "flush(%d)\n", cable->todo.num_items);

        /* Combine as much as possible into transfer() */

        /* Step 1: Count clocks. Can do only clock(TMS=0), get_tdo, transfer */

        for (i = cable->todo.next_item, n = 0; n < cable->todo.num_items; n++)
        {
            if (cable->todo.data[i].action != URJ_TAP_CABLE_CLOCK
                && cable->todo.data[i].action != URJ_TAP_CABLE_TRANSFER
                && cable->todo.data[i].action != URJ_TAP_CABLE_GET_TDO)
            {
                urj_log (URJ_LOG_LEVEL_DETAIL,
                         "cutoff at n=%d because action unsuitable for transfer\n",
                         n);
                break;
            }
            if (cable->todo.data[i].action == URJ_TAP_CABLE_CLOCK
                && cable->todo.data[i].arg.clock.tms != 0)
            {
                urj_log (URJ_LOG_LEVEL_DETAIL,
                         "cutoff at n=%d because clock.tms=1 is unsuitable for transfer\n",
                         n);
                break;
            }
            if (cable->todo.data[i].action == URJ_TAP_CABLE_CLOCK)
            {
                int k = cable->todo.data[i].arg.clock.n;
                urj_log (URJ_LOG_LEVEL_DETAIL, "%d clock(s)\n", k);
                bits += k;
            }
            else if (cable->todo.data[i].action == URJ_TAP_CABLE_TRANSFER)
            {
                int k = cable->todo.data[i].arg.transfer.len;
                urj_log (URJ_LOG_LEVEL_DETAIL, "%d transfer\n", k);
                bits += k;
            }
            i++;
            if (i >= cable->todo.max_items)
                i = 0;
        }

        urj_log (URJ_LOG_LEVEL_DETAIL, "%d combined into one (%d bits)\n",
                 n, bits);

        savbits = bits;

        if (bits == 0 || n <= 1)
        {
            do_one_queued_action (cable);
        }
        else
        {
            /* Step 2: Combine into single transfer. */

            in = malloc (bits);
            out = malloc (bits);

            if (in == NULL || out == NULL)
            {
                /* @@@@ RFHH free(NULL) is correct */
                if (in != NULL)
                    free (in);
                if (out != NULL)
                    free (out);
                urj_tap_cable_generic_flush_one_by_one (cable, how_much);
                break;
            }

            for (j = 0, bits = 0, i = cable->todo.next_item; j < n; j++)
            {
                if (cable->todo.data[i].action == URJ_TAP_CABLE_CLOCK)
                {
                    int k;
                    for (k = 0; k < cable->todo.data[i].arg.clock.n; k++)
                        in[bits++] = cable->todo.data[i].arg.clock.tdi;
                }
                else if (cable->todo.data[i].action == URJ_TAP_CABLE_TRANSFER)
                {
                    int len = cable->todo.data[i].arg.transfer.len;
                    if (len > 0)
                    {
                        memcpy (in + bits,
                                cable->todo.data[i].arg.transfer.in, len);
                        bits += len;
                    }
                }
                i++;
                if (i >= cable->todo.max_items)
                    i = 0;
            }

            /* Step 3: Do the transfer */

            /* @@@@ RFHH check result */
            r = cable->driver->transfer (cable, bits, in, out);
            urj_log (URJ_LOG_LEVEL_DETAIL, "in: ");
            print_vector (URJ_LOG_LEVEL_DETAIL, bits, in);
            urj_log (URJ_LOG_LEVEL_DETAIL, "\n");
            // @@@@ RFHH here always: out != NULL
            if (out)
            {
                urj_log (URJ_LOG_LEVEL_DETAIL, "out: ");
                print_vector (URJ_LOG_LEVEL_DETAIL, bits, out);
                urj_log (URJ_LOG_LEVEL_DETAIL, "\n");
            }

            /* Step 4: Pick results from transfer */

            for (j = 0, bits = 0, i = cable->todo.next_item; j < n; j++)
            {
                if (cable->todo.data[i].action == URJ_TAP_CABLE_CLOCK)
                {
                    int k;
                    for (k = 0; k < cable->todo.data[i].arg.clock.n; k++)
                        tdo = out[bits++];
                }
                else if (cable->todo.data[i].action == URJ_TAP_CABLE_GET_TDO)
                {
                    int c = urj_tap_cable_add_queue_item (cable,
                                                          &cable->done);
                    urj_log (URJ_LOG_LEVEL_DETAIL,
                             "add result from transfer to %p.%d\n",
                             &cable->done, c);
                    cable->done.data[c].action = URJ_TAP_CABLE_GET_TDO;
                    if (bits < savbits)
                        tdo = out[bits];
                    else
                        tdo = cable->driver->get_tdo(cable);
                    cable->done.data[c].arg.value.val = tdo;
                }
                else if (cable->todo.data[i].action == URJ_TAP_CABLE_TRANSFER)
                {
                    char *p = cable->todo.data[i].arg.transfer.out;
                    int len = cable->todo.data[i].arg.transfer.len;
                    free (cable->todo.data[i].arg.transfer.in);
                    if (p != NULL)
                    {
                        int c = urj_tap_cable_add_queue_item (cable,
                                                              &cable->done);
                        urj_log (URJ_LOG_LEVEL_DETAIL,
                                 "add result from transfer to %p.%d\n",
                                 &cable->done, c);
                        cable->done.data[c].action = URJ_TAP_CABLE_TRANSFER;
                        cable->done.data[c].arg.xferred.len = len;
                        cable->done.data[c].arg.xferred.res = r;
                        cable->done.data[c].arg.xferred.out = p;
                        if (len > 0)
                            memcpy (p, out + bits, len);
                    }
                    if (len > 0)
                        bits += len;
                    if (bits > 0)
                        tdo = out[bits - 1];
                }
                i++;
                if (i >= cable->todo.max_items)
                    i = 0;
            }

            cable->todo.next_item = i;
            cable->todo.num_items -= n;

            free (in);
            free (out);
        }
    }
    while (cable->todo.num_items > 0);
}

void
urj_tap_cable_generic_set_frequency (urj_cable_t *cable,
                                     uint32_t new_frequency)
{
    if (new_frequency == 0)
    {
        cable->delay = 0;
        cable->frequency = 0;
    }
    else
    {
        const double tolerance = 0.1;
        uint32_t loops = 2048;
        uint32_t delay = cable->delay;
        uint32_t frequency = cable->frequency;

        if (new_frequency > (1.0 - tolerance) * frequency &&
            new_frequency < (1.0 + tolerance) * frequency)
            return;

        urj_log (URJ_LOG_LEVEL_NORMAL,
                 "requested frequency %lu, now calibrating delay loop\n",
                (long unsigned) new_frequency);

        while (1)
        {
            uint32_t i, new_delay;
            long double start, end, real_frequency;

            cable->delay = delay;
            start = urj_lib_frealtime ();
            for (i = 0; i < loops; ++i)
            {
                cable->driver->clock (cable, 0, 0, 1);
            }
            end = urj_lib_frealtime ();

            if (end < start)
            {
                urj_log (URJ_LOG_LEVEL_ERROR,
                         _("calibration error, wall clock is not monotonically increasing\n"));
                break;
            }
            if (end == start)
            {
                /* retry with higher loop count
                   if the timer is not fine grained enough */
                loops *= 2;
                continue;
            }
            real_frequency = (long double) loops / (end - start);
            urj_log (URJ_LOG_LEVEL_NORMAL,
                     "new real frequency %Lg, delay %lu\n",
                     real_frequency, (long unsigned) delay);

            new_delay = (long double) delay *real_frequency / new_frequency;

            if (real_frequency >= (1.0 - tolerance) * new_frequency)
            {
                if (real_frequency <= (1.0 + tolerance) * new_frequency)
                {
                    frequency = real_frequency;
                    break;
                }
                if (new_delay > delay)
                {
                    delay = new_delay;
                }
                else
                {
                    delay++;
                }

            }
            else
            {
                if (delay == 0)
                {
                    urj_log (URJ_LOG_LEVEL_NORMAL, "operating without delay\n");
                    frequency = real_frequency;
                    break;
                }

                if (new_delay < delay)
                {
                    delay = new_delay;
                }
                else
                {
                    if (delay > 0)
                        delay--;
                }
            }
        }

        urj_log (URJ_LOG_LEVEL_NORMAL, "done\n");

        cable->delay = delay;
        cable->frequency = frequency;
    }
}
