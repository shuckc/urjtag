/*
 * $Id$
 *
 * Copyright (C) 2008 Kolja Waschk <kawk>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <urjtag/log.h>
#include <urjtag/error.h>
#include <urjtag/jim.h>

static const urj_jim_tap_state_t next_tap_state[16][2] = {
    /* URJ_JIM_RESET       */ {URJ_JIM_IDLE,            URJ_JIM_RESET},
    /* URJ_JIM_SELECT_DR   */ {URJ_JIM_CAPTURE_DR,      URJ_JIM_SELECT_IR},
    /* URJ_JIM_CAPTURE_DR  */ {URJ_JIM_SHIFT_DR,        URJ_JIM_EXIT1_DR},
    /* URJ_JIM_SHIFT_DR    */ {URJ_JIM_SHIFT_DR,        URJ_JIM_EXIT1_DR},
    /* URJ_JIM_EXIT1_DR    */ {URJ_JIM_PAUSE_DR,        URJ_JIM_UPDATE_DR},
    /* URJ_JIM_PAUSE_DR    */ {URJ_JIM_PAUSE_DR,        URJ_JIM_EXIT2_DR},
    /* URJ_JIM_EXIT2_DR    */ {URJ_JIM_SHIFT_DR,        URJ_JIM_UPDATE_DR},
    /* URJ_JIM_UPDATE_DR   */ {URJ_JIM_IDLE,            URJ_JIM_SELECT_DR},
    /* URJ_JIM_IDLE        */ {URJ_JIM_IDLE,            URJ_JIM_SELECT_DR},
    /* URJ_JIM_SELECT_IR   */ {URJ_JIM_CAPTURE_IR,      URJ_JIM_RESET},
    /* URJ_JIM_CAPTURE_IR  */ {URJ_JIM_SHIFT_IR,        URJ_JIM_EXIT1_IR},
    /* URJ_JIM_SHIFT_IR    */ {URJ_JIM_SHIFT_IR,        URJ_JIM_EXIT1_IR},
    /* URJ_JIM_EXIT1_IR    */ {URJ_JIM_PAUSE_IR,        URJ_JIM_UPDATE_IR},
    /* URJ_JIM_PAUSE_IR    */ {URJ_JIM_EXIT2_IR,        URJ_JIM_EXIT2_IR},
    /* URJ_JIM_EXIT2_IR    */ {URJ_JIM_SHIFT_IR,        URJ_JIM_UPDATE_IR},
    /* URJ_JIM_UPDATE_IR   */ {URJ_JIM_IDLE,            URJ_JIM_SELECT_DR}
};

static void
urj_jim_print_sreg (urj_log_level_t ll, urj_jim_shift_reg_t *r)
{
    int i;
    for (i = (r->len + 31) / 32; i >= 0; i--)
        urj_log (ll, " %08X", r->reg[i]);
}

static void
urj_jim_print_tap_state (urj_log_level_t ll, char *rof, urj_jim_device_t *dev)
{
    urj_log (ll, " tck %s, state=", rof);
    switch (dev->tap_state & 7)
    {
    case 0:
        urj_log (ll, (dev->tap_state == URJ_JIM_RESET) ? "URJ_JIM_RESET"
                                                       : "URJ_JIM_IDLE");
        break;
    case 1:
        urj_log (ll, "SELECT");
        break;
    case 2:
        urj_log (ll, "CAPTURE");
        break;
    case 3:
        urj_log (ll, "SHIFT");
        break;
    case 4:
        urj_log (ll, "EXIT1");
        break;
    case 5:
        urj_log (ll, "PAUSE");
        break;
    case 6:
        urj_log (ll, "EXIT2");
        break;
    default:
        urj_log (ll, "UPDATE");
        break;
    }
    if (dev->tap_state & 7)
    {
        if (dev->tap_state & 8)
        {
            urj_log (ll, "_IR=");
            urj_jim_print_sreg (ll, &dev->sreg[0]);
        }
        else
        {
            urj_log (ll, "_DR");
            if (dev->current_dr != 0)
            {
                urj_log (ll, "(%d)=", dev->current_dr);
                urj_jim_print_sreg (ll, &dev->sreg[dev->current_dr]);
            }
        }
    }
    urj_log (ll, "\n");
}


void
urj_jim_set_trst (urj_jim_state_t *s, int trst)
{
    s->trst = trst;
}

int
urj_jim_get_trst (urj_jim_state_t *s)
{
    return s->trst;
}

int
urj_jim_get_tdo (urj_jim_state_t *s)
{
    if (s->last_device_in_chain == NULL)
        return 0;
    return s->last_device_in_chain->tdo;
}

void
urj_jim_tck_rise (urj_jim_state_t *s, int tms, int tdi)
{
    urj_jim_device_t *dev;

    for (dev = s->last_device_in_chain; dev; dev = dev->prev)
    {
        int dev_tdi;
        int i, n;
        urj_jim_shift_reg_t *sr;
        uint32_t *reg;

        urj_jim_print_tap_state (URJ_LOG_LEVEL_DETAIL, "rise", dev);

        dev_tdi = (dev->prev != NULL) ? dev->prev->tdo : tdi;

        if (dev->tck_rise != NULL)
            dev->tck_rise (dev, tms, dev_tdi, s->shmem, s->shmem_size);

        if (dev->tap_state & 8)
        {
            sr = &(dev->sreg[0]);
        }
        else
        {
            if (dev->current_dr == 0)
            {
                sr = NULL;      /* BYPASS */
            }
            else
            {
                sr = &(dev->sreg[dev->current_dr]);
            }
        }

        if (sr == NULL)         /* BYPASS */
        {
            dev->tdo_buffer = dev_tdi;
        }
        else
        {
            reg = sr->reg;

            if (dev->tap_state == URJ_JIM_SHIFT_IR
                || dev->tap_state == URJ_JIM_SHIFT_DR)
            {
                /* Start with LSW of shift register at index 0 */

                n = (sr->len - 1) / 32;
                for (i = 0; i < (sr->len - 1) / 32; i++)
                {
                    reg[i] >>= 1;
                    if (reg[i + 1] & 1)
                        reg[i] |= 0x80000000;
                }

                /* End with MSW at index i */

                reg[i] >>= 1;
                if (dev_tdi != 0)
                {
                    n = (sr->len & 31);
                    if (n == 0)
                        n = 32;
                    reg[i] |= (1 << (n - 1));
                }
            }

            dev->tdo_buffer = reg[0] & 1;
        }

        dev->tap_state = next_tap_state[dev->tap_state][tms];
    }
}

void
urj_jim_tck_fall (urj_jim_state_t *s)
{
    urj_jim_device_t *dev;

    for (dev = s->last_device_in_chain; dev; dev = dev->prev)
    {
        dev->tdo = dev->tdo_buffer;

        urj_jim_print_tap_state (URJ_LOG_LEVEL_DETAIL, "fall", dev);

        if (dev->tck_fall != NULL)
            dev->tck_fall (dev, s->shmem, s->shmem_size);
    }
}

urj_jim_device_t *
urj_jim_alloc_device (int num_sregs, const int reg_size[])
{
    int i, r;

    urj_jim_device_t *dev = malloc (sizeof (urj_jim_device_t));
    if (dev == NULL)
    {
        urj_error_set (URJ_ERROR_OUT_OF_MEMORY, "malloc(%zd) fails",
                       sizeof (urj_jim_device_t));
        return NULL;
    }

    dev->sreg = malloc (num_sregs * sizeof (urj_jim_shift_reg_t));
    if (dev->sreg == NULL)
    {
        free (dev);
        urj_error_set (URJ_ERROR_OUT_OF_MEMORY, "malloc(%zd) fails",
                       sizeof (urj_jim_shift_reg_t));
        return NULL;
    }

    for (r = 0, i = 0; i < num_sregs; i++)
    {
        dev->sreg[i].len = reg_size[i];
        dev->sreg[i].reg = calloc (((reg_size[i] + 31) / 32),
                                   sizeof (uint32_t));
        if (dev->sreg[i].reg == NULL)
            r++;
    }

    if (r > 0)
    {
        urj_error_set (URJ_ERROR_OUT_OF_MEMORY, "calloc() fails");
        for (i = 0; i < num_sregs; i++)
            if (dev->sreg[i].reg != NULL)
                free (dev->sreg[i].reg);
        free (dev->sreg);
        free (dev);
        return NULL;
    }

    dev->num_sregs = num_sregs;
    dev->current_dr = 0;
    dev->tck_rise = NULL;
    dev->tck_fall = NULL;
    dev->dev_free = NULL;
    dev->tap_state = URJ_JIM_RESET;
    dev->tdo = dev->tdo_buffer = 1;

    return dev;
}

urj_jim_state_t *
urj_jim_init (void)
{
    urj_jim_state_t *s;

    s = malloc (sizeof (urj_jim_state_t));
    if (s == NULL)
    {
        urj_error_set (URJ_ERROR_OUT_OF_MEMORY, "malloc(%zd) fails",
                       sizeof (urj_jim_state_t));
        return NULL;
    }

    s->shmem_size = (1 << 20) * 16;     /* 16 MByte */
    s->shmem = malloc (s->shmem_size);

    if (s->shmem != NULL)
    {
        memset (s->shmem, 0xFF, s->shmem_size);
        urj_log (URJ_LOG_LEVEL_NORMAL,
                 "Allocated %zd bytes for device memory simulation.\n",
                s->shmem_size);
    }
    else
    {
        free (s);
        urj_error_set (URJ_ERROR_OUT_OF_MEMORY, "malloc(%zd) fails",
                       sizeof (s->shmem_size));
        return NULL;
    }

    s->trst = 0;
    s->last_device_in_chain = urj_jim_some_cpu ();

    if (s->last_device_in_chain != NULL)
    {
        s->last_device_in_chain->prev = NULL;
    }
    else
    {
        free (s->shmem);
        free (s);
        // retain error state
        return NULL;
    }
    return s;
}

void
urj_jim_free (urj_jim_state_t *s)
{
    urj_jim_device_t *dev, *pre;

    if (s == NULL)
        return;

    for (dev = s->last_device_in_chain; dev; dev = pre)
    {
        int i;

        if (dev->dev_free != NULL)
            dev->dev_free (dev);
        for (i = 0; i < dev->num_sregs; i++)
        {
            free (dev->sreg[i].reg);
        }
        free (dev->sreg);
        pre = dev->prev;
        free (dev);
    }

    s->last_device_in_chain = NULL;
    free (s->shmem);
    free (s);
}
