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

#include <sysdep.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdint.h>
#include <string.h>

#include <urjtag/log.h>
#include <urjtag/error.h>
#include <urjtag/bus.h>
#include <urjtag/bus_driver.h>
#include <urjtag/chain.h>
#include <urjtag/tap.h>
#include <urjtag/cable.h>

#include "cable.h"

const urj_cable_driver_t * const urj_tap_cable_drivers[] = {
#define _URJ_CABLE(cable) &urj_tap_cable_##cable##_driver,
#include "cable_list.h"
    NULL                        /* last must be NULL */
};

const urj_cable_driver_t *
urj_tap_cable_find (const char *cname)
{
    size_t i;

    for (i = 0; urj_tap_cable_drivers[i]; ++i)
        if (strcasecmp (cname, urj_tap_cable_drivers[i]->name) == 0)
            break;

    return urj_tap_cable_drivers[i];
}

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
        urj_error_set (URJ_ERROR_OUT_OF_MEMORY,
                       _("malloc(%zd)/malloc(%zd) fails"),
                       cable->todo.max_items * sizeof (urj_cable_queue_t),
                       cable->done.max_items * sizeof (urj_cable_queue_t));
        if (cable->todo.data != NULL)
            free (cable->todo.data);
        if (cable->done.data != NULL)
            free (cable->done.data);
        return URJ_STATUS_FAIL;
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
    cable->driver->done (cable);
}

int
urj_tap_cable_add_queue_item (urj_cable_t *cable, urj_cable_queue_info_t *q)
{
    int i, j;
    if (q->num_items >= q->max_items)   /* queue full? */
    {
        int new_max_items;
        urj_cable_queue_t *resized;

        urj_log (URJ_LOG_LEVEL_DETAIL,
            "Queue %p needs resizing; n(%d) >= max(%d); free=%d, next=%d\n",
             q, q->num_items, q->max_items, q->next_free, q->next_item);

        new_max_items = q->max_items + 128;
        resized = realloc (q->data, new_max_items * sizeof (urj_cable_queue_t));
        if (resized == NULL)
        {
            urj_error_set (URJ_ERROR_OUT_OF_MEMORY, "realloc(%s,%zd) fails",
                           "q->data",
                           new_max_items * sizeof (urj_cable_queue_t));
            return -1;          /* report failure */
        }
        urj_log (URJ_LOG_LEVEL_DETAIL,
                 _("(Resized JTAG activity queue to hold max %d items)\n"),
                 new_max_items);
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
#endif /* def CHOOSE_SMALLEST_AREA_TO_MOVE */
            {
                /* Move queue items at end of old array
                 * towards end of new array: 345612__ -> 3456__12 */

                int dest = new_max_items - num_to_move;
                urj_log (URJ_LOG_LEVEL_DETAIL,
                    "Resize: Move %d items towards end of queue memory (%d > %d)\n",
                    num_to_move, q->next_item, dest);
                memmove (&q->data[dest], &q->data[q->next_item],
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

                    urj_log (URJ_LOG_LEVEL_DETAIL,
                             "Resize: Move %d items from start to end\n",
                             q->next_free);
                    memcpy (&q->data[q->max_items], &q->data[0],
                            q->next_free * sizeof (urj_cable_queue_t));

                }
                else
                {
                    /* Same as above, but for the case if new space
                     * isn't large enough to hold all relocated items */

                    /* Step 1: 456123__ -> __612345 */

                    urj_log (URJ_LOG_LEVEL_DETAIL,
                             "Resize.A: Move %d items from start to end\n",
                            added_space);

                    memcpy (&q->data[q->max_items], &q->data[0],
                            added_space * sizeof (urj_cable_queue_t));

                    /* Step 2: __612345 -> 6__12345 */

                    urj_log (URJ_LOG_LEVEL_DETAIL,
                         "Resize.B: Move %d items towards start (offset %d)\n",
                         (q->next_free - added_space), added_space);

                    memmove (&q->data[0], &q->data[added_space],
                             (q->next_free -
                              added_space) * sizeof (urj_cable_queue_t));
                }
            }
#endif /* def CHOOSE_SMALLEST_AREA_TO_MOVE */
        }
        q->max_items = new_max_items;
        q->next_free = q->next_item + q->num_items;
        if (q->next_free >= new_max_items)
            q->next_free -= new_max_items;

        urj_log (URJ_LOG_LEVEL_DETAIL,
             "Queue %p after resizing; n(%d) >= max(%d); free=%d, next=%d\n",
             q, q->num_items, q->max_items, q->next_free, q->next_item);
    }

    i = q->next_free;
    j = i + 1;
    if (j >= q->max_items)
        j = 0;
    q->next_free = j;
    q->num_items++;

    // urj_log (URJ_LOG_LEVEL_DEBUG, "add_queue_item to %p: %d\n", q, i);
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
        // urj_log (URJ_LOG_LEVEL_DEBUG, "get_queue_item from %p: %d\n", q, i);
        return i;
    }

    urj_error_set (URJ_ERROR_NOTFOUND, "queue is empty");
    // urj_log (URJ_LOG_LEVEL_DEBUG, "get_queue_item from %p: %d\n", q, -1);
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
    int i = urj_tap_cable_add_queue_item (cable, &cable->todo);
    if (i < 0)
        return URJ_STATUS_FAIL;               /* report failure */
    cable->todo.data[i].action = URJ_TAP_CABLE_CLOCK;
    cable->todo.data[i].arg.clock.tms = tms;
    cable->todo.data[i].arg.clock.tdi = tdi;
    cable->todo.data[i].arg.clock.n = n;
    urj_tap_cable_flush (cable, URJ_TAP_CABLE_OPTIONALLY);
    return URJ_STATUS_OK;                   /* success */
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
    i = urj_tap_cable_get_queue_item (cable, &cable->done);
    if (i >= 0)
    {
        if (cable->done.data[i].action != URJ_TAP_CABLE_GET_TDO)
        {
            urj_warning (
                 _("Internal error: Got wrong type of result from queue (%d? %p.%d)\n"),
                 cable->done.data[i].action, &cable->done, i);
            urj_tap_cable_purge_queue (&cable->done, 1);
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
    int i = urj_tap_cable_add_queue_item (cable, &cable->todo);
    if (i < 0)
        return URJ_STATUS_FAIL;               /* report failure */
    cable->todo.data[i].action = URJ_TAP_CABLE_GET_TDO;
    urj_tap_cable_flush (cable, URJ_TAP_CABLE_OPTIONALLY);
    return URJ_STATUS_OK;                   /* success */
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
    int i = urj_tap_cable_add_queue_item (cable, &cable->todo);
    if (i < 0)
        return URJ_STATUS_FAIL;               /* report failure */
    cable->todo.data[i].action = URJ_TAP_CABLE_SET_SIGNAL;
    cable->todo.data[i].arg.value.mask = mask;
    cable->todo.data[i].arg.value.val = val;
    urj_tap_cable_flush (cable, URJ_TAP_CABLE_OPTIONALLY);
    return URJ_STATUS_OK;                   /* success */
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
    i = urj_tap_cable_get_queue_item (cable, &cable->done);
    if (i >= 0)
    {
        if (cable->done.data[i].action != URJ_TAP_CABLE_GET_SIGNAL)
        {
            urj_warning (
                 _("Internal error: Got wrong type of result from queue (%d? %p.%d)\n"),
                cable->done.data[i].action, &cable->done, i);
            urj_tap_cable_purge_queue (&cable->done, 1);
        }
        else if (cable->done.data[i].arg.value.sig != sig)
        {
            urj_warning (
                 _("Internal error: Got wrong signal's value from queue (%d? %p.%d)\n"),
                cable->done.data[i].action, &cable->done, i);
            urj_tap_cable_purge_queue (&cable->done, 1);
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
    int i = urj_tap_cable_add_queue_item (cable, &cable->todo);
    if (i < 0)
        return URJ_STATUS_FAIL;               /* report failure */
    cable->todo.data[i].action = URJ_TAP_CABLE_GET_SIGNAL;
    cable->todo.data[i].arg.value.sig = sig;
    urj_tap_cable_flush (cable, URJ_TAP_CABLE_OPTIONALLY);
    return URJ_STATUS_OK;                   /* success */
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
    i = urj_tap_cable_get_queue_item (cable, &cable->done);

    if (i >= 0 && cable->done.data[i].action == URJ_TAP_CABLE_TRANSFER)
    {
#if 0
        urj_log (URJ_LOG_LEVEL_DEBUG, "Got queue item (%p.%d) len=%d out=%p\n",
                &cable->done, i,
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
        urj_warning (
             _("Internal error: Got wrong type of result from queue (#%d %p.%d)\n"),
             cable->done.data[i].action, &cable->done, i);
        urj_tap_cable_purge_queue (&cable->done, 1);
    }
    else
    {
        urj_warning (
             _("Internal error: Wanted transfer result but none was queued\n"));
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
    {
        urj_error_set (URJ_ERROR_OUT_OF_MEMORY, "malloc(%zd) fails",
                       (size_t) len);
        return URJ_STATUS_FAIL;
    }

    if (out)
    {
        obuf = malloc (len);
        if (obuf == NULL)
        {
            free (ibuf);
            urj_error_set (URJ_ERROR_OUT_OF_MEMORY, "malloc(%zd) fails",
                           (size_t) len);
            return URJ_STATUS_FAIL;
        }
    }

    i = urj_tap_cable_add_queue_item (cable, &cable->todo);
    if (i < 0)
    {
        free (ibuf);
        if (obuf)
            free (obuf);
        return URJ_STATUS_FAIL;               /* report failure */
    }

    cable->todo.data[i].action = URJ_TAP_CABLE_TRANSFER;
    cable->todo.data[i].arg.transfer.len = len;
    if (in)
        memcpy (ibuf, in, len);
    cable->todo.data[i].arg.transfer.in = ibuf;
    cable->todo.data[i].arg.transfer.out = obuf;
    urj_tap_cable_flush (cable, URJ_TAP_CABLE_OPTIONALLY);
    return URJ_STATUS_OK;                   /* success */
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

    j = 0;
    for (i = 0; i < cable->delay; ++i)
        j = i;

    /* Avoid gcc set-but-unused warnings */
    cable->delay = j + 1;
}

static urj_cable_t *
urj_tap_cable_create (urj_chain_t *chain, const urj_cable_driver_t *driver)
{
    urj_cable_t *cable;

    if (urj_bus)
        urj_bus_buses_delete (urj_bus);

    urj_tap_chain_disconnect (chain);

    cable = calloc (1, sizeof (urj_cable_t));
    if (!cable)
    {
        urj_error_set (URJ_ERROR_OUT_OF_MEMORY, "calloc(%zd,%zd) fails",
                       (size_t) 1, sizeof (urj_cable_t));
        return NULL;
    }

    cable->driver = driver;

    return cable;
}

static int
urj_tap_cable_start (urj_chain_t *chain, urj_cable_t *cable)
{
    chain->cable = cable;

    if (urj_tap_cable_init (chain->cable) != URJ_STATUS_OK)
    {
        urj_tap_chain_disconnect (chain);
        return URJ_STATUS_FAIL;
    }

    urj_tap_chain_set_trst (chain, 0);
    urj_tap_chain_set_trst (chain, 1);
    urj_tap_reset (chain);

    return URJ_STATUS_OK;
}

urj_cable_t *
urj_tap_cable_parport_connect (urj_chain_t *chain, const urj_cable_driver_t *driver,
                               urj_cable_parport_devtype_t devtype,
                               const char *devname, const urj_param_t *params[])
{
    urj_cable_t *cable;

    if (driver->device_type != URJ_CABLE_DEVICE_PARPORT)
    {
        urj_error_set (URJ_ERROR_INVALID,
                       "parport cable needs parport_connect");
        return NULL;
    }

    cable = urj_tap_cable_create (chain, driver);
    if (cable == NULL)
        return NULL;

    if (cable->driver->connect.parport (cable, devtype, devname,
                                        params) != URJ_STATUS_OK)
    {
        free (cable);
        return NULL;
    }

    if (urj_tap_cable_start (chain, cable) != URJ_STATUS_OK)
        return NULL;

    return cable;
}

urj_cable_t *
urj_tap_cable_usb_connect (urj_chain_t *chain, const urj_cable_driver_t *driver,
                           const urj_param_t *params[])
{
    urj_cable_t *cable;

    if (driver->device_type != URJ_CABLE_DEVICE_USB)
    {
        urj_error_set (URJ_ERROR_INVALID, "USB cable needs usb_connect");
        return NULL;
    }

    cable = urj_tap_cable_create (chain, driver);
    if (cable == NULL)
        return NULL;

    if (cable->driver->connect.usb (cable, params) != URJ_STATUS_OK)
    {
        free (cable);
        return NULL;
    }

    if (urj_tap_cable_start (chain, cable) != URJ_STATUS_OK)
        return NULL;

    return cable;
}

urj_cable_t *
urj_tap_cable_other_connect (urj_chain_t *chain, const urj_cable_driver_t *driver,
                             const urj_param_t *params[])
{
    urj_cable_t *cable;

    if (driver->device_type != URJ_CABLE_DEVICE_OTHER)
    {
        urj_error_set (URJ_ERROR_INVALID, "'other' cable needs other_connect");
        return NULL;
    }

    cable = urj_tap_cable_create (chain, driver);
    if (cable == NULL)
        return NULL;

    if (cable->driver->connect.other (cable, params) != URJ_STATUS_OK)
    {
        free (cable);
        return NULL;
    }

    if (urj_tap_cable_start (chain, cable) != URJ_STATUS_OK)
        return NULL;

    return cable;
}

static const urj_param_descr_t cable_param[] =
{
    { URJ_CABLE_PARAM_KEY_PID,          URJ_PARAM_TYPE_LU,      "pid", },
    { URJ_CABLE_PARAM_KEY_VID,          URJ_PARAM_TYPE_LU,      "vid", },
    { URJ_CABLE_PARAM_KEY_DESC,         URJ_PARAM_TYPE_STRING,  "desc", },
    { URJ_CABLE_PARAM_KEY_DRIVER,       URJ_PARAM_TYPE_STRING,  "driver", },
    { URJ_CABLE_PARAM_KEY_BITMAP,       URJ_PARAM_TYPE_STRING,  "bitmap", },
    { URJ_CABLE_PARAM_KEY_TDI,          URJ_PARAM_TYPE_LU,      "tdi", },
    { URJ_CABLE_PARAM_KEY_TDO,          URJ_PARAM_TYPE_LU,      "tdo", },
    { URJ_CABLE_PARAM_KEY_TMS,          URJ_PARAM_TYPE_LU,      "tms", },
    { URJ_CABLE_PARAM_KEY_TCK,          URJ_PARAM_TYPE_LU,      "tck", },
    { URJ_CABLE_PARAM_KEY_INTERFACE,    URJ_PARAM_TYPE_LU,      "interface", },
};

const urj_param_list_t urj_cable_param_list =
{
    .list = cable_param,
    .n    = ARRAY_SIZE (cable_param),
};
