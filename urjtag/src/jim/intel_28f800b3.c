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
 *
 * This code simulates an Intel Advanced Boot Block Flash Memory (B3) 28FxxxB3.
 * The simulation is based on documentation found in the corresponding datasheet,
 * Order Number 290580, Revision: 020, 18 Aug 2005.
 */

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>

#include <urjtag/types.h>
#include <urjtag/log.h>
#include <urjtag/error.h>
#include <urjtag/jim.h>

typedef enum
{
    READ_ARRAY = 0,
    READ_STATUS = 1,
    READ_ID = 2,
    PROG_SETUP = 3,
    PROG_CONTINUE = 4,
    PROG_SUSP_TO_READ_STATUS = 5,
    PROG_SUSP_TO_READ_ARRAY = 6,
    PROG_SUSP_TO_READ_ID = 7,
    PROG_COMPLETE = 8,
    ERASE_SETUP = 9,
    ERASE_ERROR = 10,
    ERASE_CONTINUE = 11,
    ERASE_SUSP_TO_READ_STATUS = 12,
    ERASE_SUSP_TO_READ_ARRAY = 13,
    ERASE_SUSP_TO_READ_ID = 14,
    ERASE_COMPLETE = 15
}
intel_f28xxxb3_op_state_t;

static const char *intel_28fxxx_opstate_name[16] = {
    "READ_ARRAY",
    "READ_STATUS",
    "READ_ID",
    "PROG_SETUP",
    "PROG_CONTINUE",
    "PROG_SUSP_TO_READ_STATUS",
    "PROG_SUSP_TO_READ_ARRAY",
    "PROG_SUSP_TO_READ_ID",
    "PROG_COMPLETE",
    "ERASE_SETUP",
    "ERASE_ERROR",
    "ERASE_CONTINUE",
    "ERASE_SUSP_TO_READ_STATUS",
    "ERASE_SUSP_TO_READ_ARRAY",
    "ERASE_SUSP_TO_READ_ID",
    "ERASE_COMPLETE"
};

typedef enum
{
    TOP = 0,
    BOTTOM = 1
}
b3_boot_type_t;

#define I28F_WSM_READY          0x80
#define I28F_ERASE_SUSPENDED    0x40
#define I28F_ERASE_ERROR        0x20
#define I28F_PROG_ERROR         0x10
#define I28F_VPP_LOW            0x08
#define I28F_PROG_SUSPENDED     0x04
#define I28F_BLOCK_LOCKED       0x02
#define I28F_RESERVED           0x01


typedef struct
{
    uint16_t identifier;
    uint32_t data_buffer;
    uint32_t address_buffer;
    uint32_t control_buffer;
    uint8_t status, status_buffer;
    intel_f28xxxb3_op_state_t opstate;
    b3_boot_type_t boot_type;
    struct timeval prog_start_time;
}
intel_f28xxxb3_state_t;

static int
urj_jim_intel_28fxxxb3_init (urj_jim_bus_device_t *d, uint16_t id,
                             b3_boot_type_t bt)
{
    d->state = malloc (sizeof (intel_f28xxxb3_state_t));
    if (d->state == NULL)
    {
        urj_error_set (URJ_ERROR_OUT_OF_MEMORY, "malloc(%zd) fails",
                       sizeof (intel_f28xxxb3_state_t));
        return URJ_STATUS_FAIL;
    }

    intel_f28xxxb3_state_t *is = d->state;
    is->opstate = READ_ARRAY;
    is->identifier = id;
    is->boot_type = bt;
    is->status = 0x00;
    is->status_buffer = 0x00;
    is->control_buffer = 0x00000000;

    return URJ_STATUS_OK;
}

static int
urj_jim_intel_28f800b3b_init (urj_jim_bus_device_t *d)
{
    return urj_jim_intel_28fxxxb3_init (d, 0x8893, BOTTOM);
}

static void
urj_jim_intel_28fxxxb3_free (urj_jim_bus_device_t *d)
{
    if (d->state != NULL)
        free (d->state);
}

static uint32_t
urj_jim_intel_28fxxxb3_capture (urj_jim_bus_device_t *d,
                                uint32_t address, uint32_t control,
                                uint8_t *shmem, size_t shmem_size)
{
    uint32_t data = 0;

    if ((control & 7) == 5)     /* OE and CS: READ */
    {
        intel_f28xxxb3_state_t *is = d->state;

        switch (is->opstate)
        {
        case READ_STATUS:
        case PROG_CONTINUE:
        case ERASE_CONTINUE:
        case PROG_SUSP_TO_READ_STATUS:
        case ERASE_SUSP_TO_READ_STATUS:
            data = is->status_buffer;
            break;

        case READ_ID:
        case PROG_SUSP_TO_READ_ID:
        case ERASE_SUSP_TO_READ_ID:
            if (address == 1)
                data = is->identifier;
            else if (address == 0)
                data = 0x0089;
            break;

        case READ_ARRAY:
        case PROG_SUSP_TO_READ_ARRAY:
        case ERASE_SUSP_TO_READ_ARRAY:
            data = shmem[(address << 1)] << 8;
            data |= shmem[(address << 1)] + 1;
            break;

        default:
            break;
        }
        urj_log (URJ_LOG_LEVEL_DETAIL,
                 "i28fxxxb3: read %04X from %08X (in %s)\n",
                 data & 0xFFFF, address,
                 intel_28fxxx_opstate_name[is->opstate]);
    }

    urj_log (URJ_LOG_LEVEL_COMM, "capture A=%08X, D=%08X%s%s%s\n",
             address, data, (control & 1) ? ", OE" : "",
             (control & 2) ? ", WE" : "", (control & 4) ? ", CS" : "");

    return data;
}

static void
urj_jim_intel_28fxxxb3_update (urj_jim_bus_device_t *d,
                               uint32_t address, uint32_t data,
                               uint32_t control, uint8_t *shmem,
                               size_t shmem_size)
{
    urj_log (URJ_LOG_LEVEL_COMM, "update  A=%08X, D=%08X%s%s%s\n",
             address, data, (control & 1) ? ", OE" : "",
             (control & 2) ? ", WE" : "", (control & 4) ? ", CS" : "");

    if (d->state != NULL)
    {
        intel_f28xxxb3_state_t *is = d->state;
        if ((((is->control_buffer & 1) == 0) && ((control & 1) == 1))   /* OE rise */
            || (((is->control_buffer & 4) == 0) && ((control & 4) == 1)))       /* CS rise */
        {
            if (is->opstate == PROG_CONTINUE || is->opstate == ERASE_CONTINUE)
            {
                long dusecs;
                struct timeval yet, diff;
                gettimeofday (&yet, NULL);

                diff.tv_sec = yet.tv_sec - is->prog_start_time.tv_sec;
                if (yet.tv_usec >= is->prog_start_time.tv_usec)
                {
                    diff.tv_usec = yet.tv_usec - is->prog_start_time.tv_usec;
                }
                else
                {
                    diff.tv_usec =
                        yet.tv_usec + 1E6 - is->prog_start_time.tv_usec;
                    diff.tv_sec--;
                }
                dusecs = 1E6 * diff.tv_sec + diff.tv_usec;
                if (is->opstate == PROG_CONTINUE)
                {
                    if (dusecs > 40)
                    {
                        shmem[(is->address_buffer << 1)] &=
                            ((data >> 8) & 0xFF);
                        shmem[(is->address_buffer << 1) + 1] &= (data & 0xFF);
                        is->status |= I28F_WSM_READY;
                    }
                }
                else if (is->opstate == ERASE_CONTINUE)
                {
                    if (dusecs > 600E3)
                    {
                        is->status |= I28F_WSM_READY;
                    }
                }
            }
            is->status_buffer = is->status;     /* latch status */
        }

        if (((control & 7) == 6) && ((is->control_buffer & 2) != 2))    /* WE rise, CS active: WRITE */
        {
            intel_f28xxxb3_state_t *is = d->state;
            uint8_t dl = data & 0xFF;

            urj_log (URJ_LOG_LEVEL_DETAIL,
                     "i28fxxxb3: write %04X to %08X (in %s)\n",
                     data & 0xFFFF, address,
                     intel_28fxxx_opstate_name[is->opstate]);

            if (dl == 0x50)
            {
                is->status &=
                    ~(I28F_BLOCK_LOCKED | I28F_VPP_LOW | I28F_PROG_ERROR |
                      I28F_ERASE_ERROR);
            }

            switch (is->opstate)
            {
            case READ_STATUS:
            case READ_ARRAY:
            case READ_ID:
                switch (dl)
                {
                case 0x10:
                case 0x40:
                    is->opstate = PROG_SETUP;
                    break;
                case 0x20:
                    is->opstate = ERASE_SETUP;
                    break;
                case 0x70:
                    is->opstate = READ_STATUS;
                    break;
                case 0x90:
                    is->opstate = READ_ID;
                    break;
                default:
                    is->opstate = READ_ARRAY;
                    break;
                }
                break;

            case PROG_SETUP:
                if (dl != 0x10 && dl != 0x40)
                {
                    is->status |= I28F_PROG_ERROR | I28F_ERASE_ERROR;
                    is->opstate = READ_STATUS;
                }
                else
                {
                    is->status &= ~I28F_WSM_READY;
                    is->data_buffer = data;
                    is->address_buffer = address;
                    is->opstate = PROG_CONTINUE;
                    gettimeofday (&(is->prog_start_time), NULL);
                }
                break;

            case PROG_CONTINUE:
                if (dl == 0xB0)
                {
                    is->opstate = PROG_SUSP_TO_READ_STATUS;
                    is->status |= I28F_PROG_SUSPENDED;
                }
                break;

            case PROG_SUSP_TO_READ_STATUS:
            case PROG_SUSP_TO_READ_ARRAY:
            case PROG_SUSP_TO_READ_ID:
                switch (dl)
                {
                case 0xD0:
                    is->status &= ~I28F_PROG_SUSPENDED;
                    is->opstate = PROG_CONTINUE;
                    break;
                case 0x70:
                    is->opstate = PROG_SUSP_TO_READ_STATUS;
                    break;
                case 0x90:
                    is->opstate = PROG_SUSP_TO_READ_ID;
                    break;
                default:
                    is->opstate = PROG_SUSP_TO_READ_ARRAY;
                    break;
                }
                break;

            case ERASE_SETUP:
                if (dl != 0xD0)
                {
                    is->status |= I28F_PROG_ERROR | I28F_ERASE_ERROR;
                    is->opstate = READ_STATUS;
                }
                else
                {
                    is->status &= 0x7F;
                    is->data_buffer = data;
                    is->address_buffer = address;
                    is->opstate = ERASE_CONTINUE;
                    gettimeofday (&(is->prog_start_time), NULL);
                }
                break;

            case ERASE_CONTINUE:
                if (dl == 0xB0)
                {
                    is->opstate = ERASE_SUSP_TO_READ_STATUS;
                    is->status |= I28F_ERASE_SUSPENDED;
                }
                break;

            case ERASE_SUSP_TO_READ_STATUS:
            case ERASE_SUSP_TO_READ_ARRAY:
            case ERASE_SUSP_TO_READ_ID:
                switch (dl)
                {
                case 0xD0:
                    is->status &= ~I28F_ERASE_SUSPENDED;
                    is->opstate = ERASE_CONTINUE;
                    break;
                case 0x70:
                    is->opstate = ERASE_SUSP_TO_READ_STATUS;
                    break;
                case 0x90:
                    is->opstate = ERASE_SUSP_TO_READ_ID;
                    break;
                default:
                    is->opstate = ERASE_SUSP_TO_READ_ARRAY;
                    break;
                }
                break;

            case PROG_COMPLETE:
            case ERASE_ERROR:
            case ERASE_COMPLETE:
                switch (dl)
                {
                case 0x10:
                case 0x40:
                    is->opstate = PROG_SETUP;
                    break;
                case 0x20:
                    is->opstate = ERASE_SETUP;
                    break;
                case 0x70:
                    is->opstate = READ_STATUS;
                    break;
                case 0x90:
                    is->opstate = READ_ID;
                    break;
                default:
                    is->opstate = READ_ARRAY;
                    break;
                }
                break;
            }
        }
        is->control_buffer = control;
    }
}

urj_jim_bus_device_t urj_jim_intel_28f800b3b = {
    2,                          /* width [bytes] */
    0x80000,                    /* size [words, each <width> bytes] */
    NULL,                       /* state */
    urj_jim_intel_28f800b3b_init,       /* init() */
    urj_jim_intel_28fxxxb3_capture,     /* access() */
    urj_jim_intel_28fxxxb3_update,      /* access() */
    urj_jim_intel_28fxxxb3_free /* free() */
};
