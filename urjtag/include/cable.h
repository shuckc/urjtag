/*
 * $Id$
 *
 * Cable driver interface
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

#ifndef URJ_CABLE_H
#define URJ_CABLE_H

#include <stdint.h>

typedef struct urj_cable urj_cable_t;

#include "usbconn.h"
#include "parport.h"
#include "chain.h"
#include "pod.h"

typedef struct urj_cable_driver urj_cable_driver_t;

typedef enum urj_cable_flush_amount
{
    URJ_TAP_CABLE_OPTIONALLY,
    URJ_TAP_CABLE_TO_OUTPUT,
    URJ_TAP_CABLE_COMPLETELY
}
urj_cable_flush_amount_t;

struct urj_cable_driver
{
    const char *name;
    const char *description;
    int (*connect) (char *params[], urj_cable_t *cable);
    void (*disconnect) (urj_cable_t *cable);
    void (*cable_free) (urj_cable_t *cable);
    int (*init) (urj_cable_t *);
    void (*done) (urj_cable_t *);
    void (*set_frequency) (urj_cable_t *, uint32_t freq);
    void (*clock) (urj_cable_t *, int, int, int);
    int (*get_tdo) (urj_cable_t *);
    int (*transfer) (urj_cable_t *, int, char *, char *);
    int (*set_signal) (urj_cable_t *, int, int);
    int (*get_signal) (urj_cable_t *, urj_pod_sigsel_t);
    void (*flush) (urj_cable_t *, urj_cable_flush_amount_t);
    void (*help) (const char *);
};

typedef struct urj_cable_queue urj_cable_queue_t;

struct urj_cable_queue
{
    enum
    {
        URJ_TAP_CABLE_CLOCK,
        URJ_TAP_CABLE_GET_TDO,
        URJ_TAP_CABLE_TRANSFER,
        URJ_TAP_CABLE_SET_SIGNAL,
        URJ_TAP_CABLE_GET_SIGNAL
    } action;
    union
    {
        struct
        {
            int tms;
            int tdi;
            int n;
        } clock;
        struct
        {
            urj_pod_sigsel_t sig;
            int mask;
            int val;
        } value;
        struct
        {
            int len;
            char *in;
            char *out;
        } transfer;
        struct
        {
            int len;
            int res;
            char *out;
        } xferred;
    } arg;
};

typedef struct urj_cable_queue_info urj_cable_queue_info_t;

struct urj_cable_queue_info
{
    urj_cable_queue_t *data;
    int max_items;
    int num_items;
    int next_item;
    int next_free;
};

struct urj_cable
{
    urj_cable_driver_t *driver;
    union
    {
        urj_usbconn_t *usb;
        urj_parport_t *port;
    } link;
    void *params;
    urj_chain_t *chain;
    urj_cable_queue_info_t todo;
    urj_cable_queue_info_t done;
    uint32_t delay;
    uint32_t frequency;
};

void urj_tap_cable_free (urj_cable_t *cable);
int urj_tap_cable_init (urj_cable_t *cable);
void urj_tap_cable_done (urj_cable_t *cable);
void urj_tap_cable_flush (urj_cable_t *cable, urj_cable_flush_amount_t);
void urj_tap_cable_clock (urj_cable_t *cable, int tms, int tdi, int n);
int urj_tap_cable_defer_clock (urj_cable_t *cable, int tms, int tdi, int n);
int urj_tap_cable_get_tdo (urj_cable_t *cable);
int urj_tap_cable_get_tdo_late (urj_cable_t *cable);
int urj_tap_cable_defer_get_tdo (urj_cable_t *cable);
int urj_tap_cable_set_signal (urj_cable_t *cable, int mask, int val);
int urj_tap_cable_defer_set_signal (urj_cable_t *cable, int mask, int val);
int urj_tap_cable_get_signal (urj_cable_t *cable, urj_pod_sigsel_t sig);
int urj_tap_cable_get_signal_late (urj_cable_t *cable, urj_pod_sigsel_t sig);
int urj_tap_cable_defer_get_signal (urj_cable_t *cable, urj_pod_sigsel_t sig);
int urj_tap_cable_transfer (urj_cable_t *cable, int len, char *in, char *out);
int urj_tap_cable_transfer_late (urj_cable_t *cable, char *out);
int urj_tap_cable_defer_transfer (urj_cable_t *cable, int len, char *in,
                                  char *out);

void urj_tap_cable_set_frequency (urj_cable_t *cable, uint32_t frequency);
uint32_t urj_tap_cable_get_frequency (urj_cable_t *cable);
void urj_tap_cable_wait (urj_cable_t *cable);
void urj_tap_cable_purge_queue (urj_cable_queue_info_t *q, int io);
int urj_tap_cable_add_queue_item (urj_cable_t *cable,
                                  urj_cable_queue_info_t *q);
int urj_tap_cable_get_queue_item (urj_cable_t *cable,
                                  urj_cable_queue_info_t *q);

extern urj_cable_driver_t *urj_tap_cable_drivers[];

#endif /* URJ_CABLE_H */
