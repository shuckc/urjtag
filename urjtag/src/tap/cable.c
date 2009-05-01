/*
 * $Id$
 *
 * Copyright (C) 2003 ETC s.r.o.
 * Copyright (C) 2005 Hein Roehrig,
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
 *
 * Written by Marcel Telka <marcel@telka.sk>, 2003;
 * Busy loop waiting (*freq* functions) Hein Roehrig, 2005;
 * JTAG activity queuing and API (*defer* functions) K. Waschk, 2008
 *
 */

#include "sysdep.h"

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdint.h>
#include <string.h>

#include <urjtag/jtag.h>
#include <urjtag/cable.h>

#undef VERBOSE

extern urj_cable_driver_t urj_tap_cable_arcom_driver;
extern urj_cable_driver_t urj_tap_cable_byteblaster_driver;
extern urj_cable_driver_t urj_tap_cable_usbblaster_driver;
extern urj_cable_driver_t urj_tap_cable_ft2232_driver;
extern urj_cable_driver_t urj_tap_cable_ft2232_jtagkey_driver;
extern urj_cable_driver_t urj_tap_cable_ft2232_armusbocd_driver;
extern urj_cable_driver_t urj_tap_cable_ft2232_gnice_driver;
extern urj_cable_driver_t urj_tap_cable_ft2232_oocdlinks_driver;
extern urj_cable_driver_t urj_tap_cable_ft2232_signalyzer_driver;
extern urj_cable_driver_t urj_tap_cable_ft2232_turtelizer2_driver;
extern urj_cable_driver_t urj_tap_cable_ft2232_usbtojtagif_driver;
extern urj_cable_driver_t urj_tap_cable_ft2232_flyswatter_driver;
extern urj_cable_driver_t urj_tap_cable_ft2232_usbscarab2_driver;
extern urj_cable_driver_t urj_tap_cable_dlc5_driver;
extern urj_cable_driver_t urj_tap_cable_ea253_driver;
extern urj_cable_driver_t urj_tap_cable_ei012_driver;
extern urj_cable_driver_t urj_tap_cable_igloo_driver;
extern urj_cable_driver_t urj_tap_cable_keithkoep_driver;
extern urj_cable_driver_t urj_tap_cable_lattice_driver;
extern urj_cable_driver_t urj_tap_cable_mpcbdm_driver;
extern urj_cable_driver_t urj_tap_cable_triton_driver;
extern urj_cable_driver_t urj_tap_cable_jim_driver;
extern urj_cable_driver_t urj_tap_cable_wiggler_driver;
extern urj_cable_driver_t urj_tap_cable_wiggler2_driver;
extern urj_cable_driver_t urj_tap_cable_wiggler_driver;
extern urj_cable_driver_t urj_tap_cable_xpc_int_driver;
extern urj_cable_driver_t urj_tap_cable_xpc_ext_driver;
extern urj_cable_driver_t urj_tap_cable_jlink_driver;
extern urj_cable_driver_t urj_tap_cable_ep9307_driver;
extern urj_cable_driver_t urj_tap_cable_ts7800_driver;

urj_cable_driver_t *urj_tap_cable_drivers[] = {
#ifdef ENABLE_CABLE_ARCOM
    &urj_tap_cable_arcom_driver,
#endif
#ifdef ENABLE_CABLE_BYTEBLASTER
    &urj_tap_cable_byteblaster_driver,
#endif

#ifdef ENABLE_CABLE_USBBLASTER
    &urj_tap_cable_usbblaster_driver,
#endif

#ifdef ENABLE_CABLE_FT2232
    &urj_tap_cable_ft2232_driver,
    &urj_tap_cable_ft2232_jtagkey_driver,
    &urj_tap_cable_ft2232_armusbocd_driver,
    &urj_tap_cable_ft2232_gnice_driver,
    &urj_tap_cable_ft2232_oocdlinks_driver,
    &urj_tap_cable_ft2232_signalyzer_driver,
    &urj_tap_cable_ft2232_turtelizer2_driver,
    &urj_tap_cable_ft2232_usbtojtagif_driver,
    &urj_tap_cable_ft2232_flyswatter_driver,
    &urj_tap_cable_ft2232_usbscarab2_driver,
#endif

#ifdef ENABLE_CABLE_DLC5
    &urj_tap_cable_dlc5_driver,
#endif
#ifdef ENABLE_CABLE_EA253
    &urj_tap_cable_ea253_driver,
#endif
#ifdef ENABLE_CABLE_EI012
    &urj_tap_cable_ei012_driver,
#endif
#ifdef ENABLE_CABLE_IGLOO
    &urj_tap_cable_igloo_driver,
#endif
#ifdef ENABLE_CABLE_KEITHKOEP
    &urj_tap_cable_keithkoep_driver,
#endif
#ifdef ENABLE_CABLE_LATTICE
    &urj_tap_cable_lattice_driver,
#endif
#ifdef ENABLE_CABLE_MPCBDM
    &urj_tap_cable_mpcbdm_driver,
#endif
#ifdef ENABLE_CABLE_TRITON
    &urj_tap_cable_triton_driver,
#endif
#ifdef ENABLE_JIM
    &urj_tap_cable_jim_driver,
#endif
#ifdef ENABLE_CABLE_WIGGLER
    &urj_tap_cable_wiggler_driver,
    &urj_tap_cable_wiggler2_driver,
#endif

#ifdef ENABLE_CABLE_XPC
    &urj_tap_cable_xpc_int_driver,
    &urj_tap_cable_xpc_ext_driver,
#endif

#ifdef ENABLE_CABLE_JLINK
    &urj_tap_cable_jlink_driver,
#endif

#ifdef ENABLE_CABLE_EP9307
    &urj_tap_cable_ep9307_driver,
#endif

#ifdef ENABLE_CABLE_TS7800
    &urj_tap_cable_ts7800_driver,
#endif
    NULL                        /* last must be NULL */
};

void
urj_tap_cable_free (urj_cable_t *cable)
{
    cable->driver->cable_free (cable);
}

int
urj_tap_cable_init (urj_cable_t *cable)
{
    cable->delay = 0;
    cable->frequency = 0;

    cable->todo.max_items = 128;
    cable->todo.num_items = 0;
    cable->todo.next_item = 0;
    cable->todo.next_free = 0;
    cable->todo.data =
        malloc (cable->todo.max_items * sizeof (urj_cable_queue_t));

    cable->done.max_items = 128;
    cable->done.num_items = 0;
    cable->done.next_item = 0;
    cable->done.next_free = 0;
    cable->done.data =
        malloc (cable->done.max_items * sizeof (urj_cable_queue_t));

    if (cable->todo.data == NULL || cable->done.data == NULL)
    {
        printf (_("Failed to allocate memory for cable activity queue.\n"));
        if (cable->todo.data != NULL)
            free (cable->todo.data);
        if (cable->done.data != NULL)
            free (cable->done.data);
        return 1;
    }

    return cable->driver->init (cable);
}

void
urj_tap_cable_flush (urj_cable_t *cable, urj_cable_flush_amount_t how_much)
{
    cable->driver->flush (cable, how_much);
}

void
urj_tap_cable_done (urj_cable_t *cable)
{
    urj_tap_cable_flush (cable, URJ_TAP_CABLE_COMPLETELY);
    if (cable->todo.data != NULL)
    {
        free (cable->todo.data);
        free (cable->done.data);
    }
    return cable->driver->done (cable);
}

int
urj_tap_cable_add_queue_item (urj_cable_t *cable, urj_cable_queue_info_t *q)
{
    int i, j;
    if (q->num_items >= q->max_items)   /* queue full? */
    {
        int new_max_items;
        urj_cable_queue_t *resized;

#ifdef VERBOSE
        printf
            ("Queue %p needs resizing; n(%d) >= max(%d); free=%d, next=%d\n",
             q, q->num_items, q->max_items, q->next_free, q->next_item);
#endif

        new_max_items = q->max_items + 128;
        resized =
            realloc (q->data, new_max_items * sizeof (urj_cable_queue_t));
        if (resized == NULL)
        {
            printf (_
                    ("Out of memory: couldn't resize activity queue to %d\n"),
                    new_max_items);
            return -1;          /* report failure */
        }
#ifdef VERBOSE
        printf (_("(Resized JTAG activity queue to hold max %d items)\n"),
                new_max_items);
#endif
        q->data = resized;

        /* The queue was full. Except for the special case when next_item is 0,
         * resizing just introduced a gap between old and new max, which has to
         * be filled; either by moving data from next_item .. max_items, or
         * from 0 .. next_free (whatever is smaller). */

#define CHOOSE_SMALLEST_AREA_TO_MOVE 1

        if (q->next_item != 0)
        {
            int added_space = new_max_items - q->max_items;
            int num_to_move = q->max_items - q->next_item;

#ifdef CHOOSE_SMALLEST_AREA_TO_MOVE
            if (num_to_move <= q->next_free)
#endif
            {
                /* Move queue items at end of old array
                 * towards end of new array: 345612__ -> 3456__12 */

                int dest = new_max_items - num_to_move;
#ifdef VERBOSE
                printf
                    ("Resize: Move %d items towards end of queue memory (%d > %d)\n",
                     num_to_move, q->next_item, dest);
#endif
                memmove (&(q->data[dest]), &(q->data[q->next_item]),
                         num_to_move * sizeof (urj_cable_queue_t));

                q->next_item = dest;
            }
#ifdef CHOOSE_SMALLEST_AREA_TO_MOVE
            else
            {
                if (q->next_free <= added_space)
                {
                    /* Relocate queue items at beginning of old array
                     * to end of new array: 561234__ -> __123456 */

#ifdef VERBOSE
                    printf ("Resize: Move %d items from start to end\n",
                            q->next_free);
#endif
                    memcpy (&(q->data[q->max_items]), &(q->data[0]),
                            q->next_free * sizeof (urj_cable_queue_t));

                }
                else
                {
                    /* Same as above, but for the case if new space 
                     * isn't large enough to hold all relocated items */

                    /* Step 1: 456123__ -> __612345 */

#ifdef VERBOSE
                    printf ("Resize.A: Move %d items from start to end\n",
                            added_space);
#endif

                    memcpy (&(q->data[q->max_items]), &(q->data[0]),
                            added_space * sizeof (urj_cable_queue_t));

                    /* Step 2: __612345 -> 6__12345 */

#ifdef VERBOSE
                    printf
                        ("Resize.B: Move %d items towards start (offset %d)\n",
                         (q->next_free - added_space), added_space);
#endif

                    memmove (&(q->data[0]), &(q->data[added_space]),
                             (q->next_free -
                              added_space) * sizeof (urj_cable_queue_t));
                }
            }
#endif
        }
        q->max_items = new_max_items;
        q->next_free = q->next_item + q->num_items;
        if (q->next_free >= new_max_items)
            q->next_free -= new_max_items;

#ifdef VERBOSE
        printf
            ("Queue %p after resizing; n(%d) >= max(%d); free=%d, next=%d\n",
             q, q->num_items, q->max_items, q->next_free, q->next_item);
#endif
    }

    i = q->next_free;
    j = i + 1;
    if (j >= q->max_items)
        j = 0;
    q->next_free = j;
    q->num_items++;

    // printf("add_queue_item to %p: %d\n", q, i);
    return i;
}

int
urj_tap_cable_get_queue_item (urj_cable_t *cable, urj_cable_queue_info_t *q)
{
    if (q->num_items > 0)
    {
        int i = q->next_item;
        int j = i + 1;
        if (j >= q->max_items)
            j = 0;
        q->next_item = j;
        q->num_items--;
        // printf("get_queue_item from %p: %d\n", q, i);
        return i;
    }

    // printf("get_queue_item from %p: %d\n", q, -1);
    return -1;
}

void
urj_tap_cable_purge_queue (urj_cable_queue_info_t *q, int io)
{
    while (q->num_items > 0)
    {
        int i = q->next_item;
        if (q->data[i].action == URJ_TAP_CABLE_TRANSFER)
        {
            if (io == 0)        /* todo queue */
            {
                if (q->data[i].arg.transfer.in != NULL)
                    free (q->data[i].arg.transfer.in);
                if (q->data[i].arg.transfer.out != NULL)
                    free (q->data[i].arg.transfer.out);
            }
            else                /* done queue */
            {
                if (q->data[i].arg.xferred.out != NULL)
                    free (q->data[i].arg.xferred.out);
            }
        }

        i++;
        if (i >= q->max_items)
            i = 0;
        q->num_items--;
    }

    q->num_items = 0;
    q->next_item = 0;
    q->next_free = 0;
}

void
urj_tap_cable_clock (urj_cable_t *cable, int tms, int tdi, int n)
{
    urj_tap_cable_flush (cable, URJ_TAP_CABLE_COMPLETELY);
    cable->driver->clock (cable, tms, tdi, n);
}

int
urj_tap_cable_defer_clock (urj_cable_t *cable, int tms, int tdi, int n)
{
    int i = urj_tap_cable_add_queue_item (cable, &(cable->todo));
    if (i < 0)
        return 1;               /* report failure */
    cable->todo.data[i].action = URJ_TAP_CABLE_CLOCK;
    cable->todo.data[i].arg.clock.tms = tms;
    cable->todo.data[i].arg.clock.tdi = tdi;
    cable->todo.data[i].arg.clock.n = n;
    urj_tap_cable_flush (cable, URJ_TAP_CABLE_OPTIONALLY);
    return 0;                   /* success */
}

int
urj_tap_cable_get_tdo (urj_cable_t *cable)
{
    urj_tap_cable_flush (cable, URJ_TAP_CABLE_COMPLETELY);
    return cable->driver->get_tdo (cable);
}

int
urj_tap_cable_get_tdo_late (urj_cable_t *cable)
{
    int i;
    urj_tap_cable_flush (cable, URJ_TAP_CABLE_TO_OUTPUT);
    i = urj_tap_cable_get_queue_item (cable, &(cable->done));
    if (i >= 0)
    {
        if (cable->done.data[i].action != URJ_TAP_CABLE_GET_TDO)
        {
            printf (_
                    ("Internal error: Got wrong type of result from queue (%d? %p.%d)\n"),
                    cable->done.data[i].action, &(cable->done), i);
            urj_tap_cable_purge_queue (&(cable->done), 1);
        }
        else
        {
            return cable->done.data[i].arg.value.val;
        }
    }
    return cable->driver->get_tdo (cable);
}

int
urj_tap_cable_defer_get_tdo (urj_cable_t *cable)
{
    int i = urj_tap_cable_add_queue_item (cable, &(cable->todo));
    if (i < 0)
        return 1;               /* report failure */
    cable->todo.data[i].action = URJ_TAP_CABLE_GET_TDO;
    urj_tap_cable_flush (cable, URJ_TAP_CABLE_OPTIONALLY);
    return 0;                   /* success */
}

int
urj_tap_cable_set_signal (urj_cable_t *cable, int mask, int val)
{
    urj_tap_cable_flush (cable, URJ_TAP_CABLE_COMPLETELY);
    return cable->driver->set_signal (cable, mask, val);
}

int
urj_tap_cable_defer_set_signal (urj_cable_t *cable, int mask, int val)
{
    int i = urj_tap_cable_add_queue_item (cable, &(cable->todo));
    if (i < 0)
        return 1;               /* report failure */
    cable->todo.data[i].action = URJ_TAP_CABLE_SET_SIGNAL;
    cable->todo.data[i].arg.value.mask = mask;
    cable->todo.data[i].arg.value.val = val;
    urj_tap_cable_flush (cable, URJ_TAP_CABLE_OPTIONALLY);
    return 0;                   /* success */
}

int
urj_tap_cable_get_signal (urj_cable_t *cable, urj_pod_sigsel_t sig)
{
    urj_tap_cable_flush (cable, URJ_TAP_CABLE_COMPLETELY);
    return cable->driver->get_signal (cable, sig);
}

int
urj_tap_cable_get_signal_late (urj_cable_t *cable, urj_pod_sigsel_t sig)
{
    int i;
    urj_tap_cable_flush (cable, URJ_TAP_CABLE_TO_OUTPUT);
    i = urj_tap_cable_get_queue_item (cable, &(cable->done));
    if (i >= 0)
    {
        if (cable->done.data[i].action != URJ_TAP_CABLE_GET_SIGNAL)
        {
            printf (_
                    ("Internal error: Got wrong type of result from queue (%d? %p.%d)\n"),
                    cable->done.data[i].action, &(cable->done), i);
            urj_tap_cable_purge_queue (&(cable->done), 1);
        }
        else if (cable->done.data[i].arg.value.sig != sig)
        {
            printf (_
                    ("Internal error: Got wrong signal's value from queue (%d? %p.%d)\n"),
                    cable->done.data[i].action, &(cable->done), i);
            urj_tap_cable_purge_queue (&(cable->done), 1);
        }
        else
        {
            return cable->done.data[i].arg.value.val;
        }
    }
    return cable->driver->get_signal (cable, sig);
}

int
urj_tap_cable_defer_get_signal (urj_cable_t *cable, urj_pod_sigsel_t sig)
{
    int i = urj_tap_cable_add_queue_item (cable, &(cable->todo));
    if (i < 0)
        return 1;               /* report failure */
    cable->todo.data[i].action = URJ_TAP_CABLE_GET_SIGNAL;
    cable->todo.data[i].arg.value.sig = sig;
    urj_tap_cable_flush (cable, URJ_TAP_CABLE_OPTIONALLY);
    return 0;                   /* success */
}

int
urj_tap_cable_transfer (urj_cable_t *cable, int len, char *in, char *out)
{
    urj_tap_cable_flush (cable, URJ_TAP_CABLE_COMPLETELY);
    return cable->driver->transfer (cable, len, in, out);
}

int
urj_tap_cable_transfer_late (urj_cable_t *cable, char *out)
{
    int i;
    urj_tap_cable_flush (cable, URJ_TAP_CABLE_TO_OUTPUT);
    i = urj_tap_cable_get_queue_item (cable, &(cable->done));

    if (i >= 0 && cable->done.data[i].action == URJ_TAP_CABLE_TRANSFER)
    {
#if 0
        printf ("Got queue item (%p.%d) len=%d out=%p\n",
                &(cable->done), i,
                cable->done.data[i].arg.xferred.len,
                cable->done.data[i].arg.xferred.out);
#endif
        if (out)
            memcpy (out,
                    cable->done.data[i].arg.xferred.out,
                    cable->done.data[i].arg.xferred.len);
        free (cable->done.data[i].arg.xferred.out);
        return cable->done.data[i].arg.xferred.res;
    }

    if (cable->done.data[i].action != URJ_TAP_CABLE_TRANSFER)
    {
        printf (_
                ("Internal error: Got wrong type of result from queue (#%d %p.%d)\n"),
                cable->done.data[i].action, &(cable->done), i);
        urj_tap_cable_purge_queue (&(cable->done), 1);
    }
    else
    {
        printf (_
                ("Internal error: Wanted transfer result but none was queued\n"));
    }
    return 0;
}

int
urj_tap_cable_defer_transfer (urj_cable_t *cable, int len, char *in,
                              char *out)
{
    char *ibuf, *obuf = NULL;
    int i;

    ibuf = malloc (len);
    if (ibuf == NULL)
        return 1;

    if (out)
    {
        obuf = malloc (len);
        if (obuf == NULL)
        {
            free (ibuf);
            return 1;
        }
    }

    i = urj_tap_cable_add_queue_item (cable, &(cable->todo));
    if (i < 0)
    {
        free (ibuf);
        if (obuf)
            free (obuf);
        return 1;               /* report failure */
    }

    cable->todo.data[i].action = URJ_TAP_CABLE_TRANSFER;
    cable->todo.data[i].arg.transfer.len = len;
    if (in)
        memcpy (ibuf, in, len);
    cable->todo.data[i].arg.transfer.in = ibuf;
    cable->todo.data[i].arg.transfer.out = obuf;
    urj_tap_cable_flush (cable, URJ_TAP_CABLE_OPTIONALLY);
    return 0;                   /* success */
}

void
urj_tap_cable_set_frequency (urj_cable_t *cable, uint32_t new_frequency)
{
    urj_tap_cable_flush (cable, URJ_TAP_CABLE_COMPLETELY);
    cable->driver->set_frequency (cable, new_frequency);
}

uint32_t
urj_tap_cable_get_frequency (urj_cable_t *cable)
{
    return cable->frequency;
}

void
urj_tap_cable_wait (urj_cable_t *cable)
{
    int i;
    volatile int j;
    uint32_t delay = cable->delay;

    if (delay == 0)
        return;

    for (i = 0; i < delay; ++i)
    {
        j = i;
    }
}
