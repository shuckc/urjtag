/*
 * $Id: tap.c $
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

#include <jim.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

typedef enum
{
  READ_ARRAY,
  READ_STATUS,
  READ_ID,
  PROG_SETUP,
  PROG_CONTINUE,
  PROG_SUSP_TO_READ_STATUS,
  PROG_SUSP_TO_READ_ARRAY,
  PROG_SUSP_TO_READ_ID,
  PROG_COMPLETE,
  ERASE_SETUP,
  ERASE_ERROR,
  ERASE_CONTINUE,
  ERASE_SUSP_TO_READ_STATUS,
  ERASE_SUSP_TO_READ_ARRAY,
  ERASE_SUSP_TO_READ_ID,
  ERASE_COMPLETE
}
intel_f28xxxb3_op_state_t;

typedef enum
{
  TOP = 0,
  BOTTOM = 1
}
b3_boot_type_t;

typedef struct
{
  uint16_t identifier;
  uint32_t control_buffer;
  uint8_t status, status_buffer;
  intel_f28xxxb3_op_state_t opstate;
  b3_boot_type_t boot_type;
}
intel_f28xxxb3_state_t;

void intel_28fxxxb3_init(jim_bus_device_t *d, uint16_t id, b3_boot_type_t bt)
{
  d->state = malloc(sizeof(intel_f28xxxb3_state_t));
  if(d->state != NULL)
  {
    intel_f28xxxb3_state_t *is = d->state;
    is->opstate = READ_ARRAY;
    is->identifier = id;
    is->boot_type = bt;
    is->status = 0x00;
    is->status_buffer = 0x00;
    is->control_buffer = 0x00000000;
  }
}

void intel_28f800b3t_init(jim_bus_device_t *d)
{
  intel_28fxxxb3_init(d, 0x8893, BOTTOM);
}

void intel_28fxxxb3_free(jim_bus_device_t *d)
{
  if(d->state != NULL) free(d->state);
}

uint32_t intel_28fxxxb3_capture(jim_bus_device_t *d,
    uint32_t address, uint32_t control,
    uint8_t *shmem, size_t shmem_size)
{
  uint32_t data = 0;

  if((control&7) == 5) /* OE and CS: READ */
  {
    intel_f28xxxb3_state_t *is = d->state;
    switch(is->opstate)
    {
      case READ_STATUS:
        data = is->status_buffer; 
        break;
      case READ_ID:
        if(address == 0) data = is->identifier;
        else if(address == 1) data = 0x0089;
        break;
      case READ_ARRAY:
        data = shmem[(address<<1)]<<8;
        data |= shmem[(address<<1)]+1;
        break;
      default:
        break;
    }
  }

  printf("capture A=%08X, D=%08X%s%s%s\n", address, data,
    (control & 1) ? ", OE":"",
    (control & 2) ? ", WE":"",
    (control & 4) ? ", CS":"");

  return data;
}
 
void intel_28fxxxb3_update(jim_bus_device_t *d,
    uint32_t address, uint32_t data, uint32_t control,
    uint8_t *shmem, size_t shmem_size)
{
  printf("update  A=%08X, D=%08X%s%s%s\n", address, data,
    (control & 1) ? ", OE":"",
    (control & 2) ? ", WE":"",
    (control & 4) ? ", CS":"");

  if(d->state != NULL)
  {
    intel_f28xxxb3_state_t *is = d->state;
    if( (((is->control_buffer&1)==0) && ((control&1)==1)) /* OE rise */
      ||(((is->control_buffer&4)==0) && ((control&4)==1))) /* CS rise */
    {
      is->status_buffer = is->status; /* latch status */
    };

    if(((control&7)==6)&&((is->control_buffer&2)!=2)) /* WE rise, CS active: WRITE */
    {
      intel_f28xxxb3_state_t *is = d->state;
      uint8_t dl = data & 0xFF;
      switch(is->opstate)
      {
        case READ_ARRAY:
        case READ_STATUS:
        case READ_ID:
          switch(dl)
          {
            case 0x10: 
            case 0x40: is->opstate = PROG_SETUP; break;
            case 0x20: is->opstate = ERASE_SETUP; break;
            case 0x70: is->opstate = READ_STATUS; break;
            case 0x90: is->opstate = READ_ID; break;
            case 0xD0: is->opstate = READ_ARRAY; break;
            case 0xB0: is->opstate = READ_ARRAY; break;
            case 0xFF: is->opstate = READ_ARRAY; break;

            default:   is->opstate = READ_ARRAY; break;
          }
          break;
        default:
          break;
      }
    };
    is->control_buffer = control;
  }
}

jim_bus_device_t intel_28f800b3t =
{
    2,       /* width [bytes] */
    0x80000, /* size [words, each <width> bytes] */
    NULL,    /* state */
    intel_28f800b3t_init,  /* init() */
    intel_28fxxxb3_capture,  /* access() */
    intel_28fxxxb3_update,  /* access() */
    intel_28fxxxb3_free  /* free() */
};

