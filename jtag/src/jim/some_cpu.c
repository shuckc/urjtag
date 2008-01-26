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
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <jim.h>
#include <bitmask.h>

#undef VERBOSE

extern jim_bus_device_t intel_28f800b3b;

jim_attached_part_t some_cpu_attached[] =
{
    /* 1. Address offset: base offset [bytes]
     * 2. Address shift: Distance between address LSB of device and CPU
     * 3. Data shift: Distance between D0 of device and CPU e.g. 0, 8, 16 or 24 bits
     * 4. Part: Pointer to part structure */
 
    { 0x00000000, 1, 0, &intel_28f800b3b },

    { 0xFFFFFFFF, 0, 0, NULL } /* Always end list with part == NULL */
};

#define BSR_LEN 202

void some_cpu_report_idcode(jim_device_t *dev)
{
  dev->sreg[0].reg[0] = 0x1; /* IDCODE instruction b0001 */
  dev->sreg[1].reg[0] = 0x87654321; /* Load IDR (fake) */
  dev->current_dr = 1; /* IDR */
}

void some_cpu_tck_rise(jim_device_t *dev, 
    int tms, int tdi, 
    uint8_t *shmem, size_t shmem_size )
{
  int i;

  switch(dev->tap_state)
  {
    case RESET:

      some_cpu_report_idcode(dev);
      break;

    case CAPTURE_DR:

      if(dev->current_dr==2) // if(dev->sreg[0].reg[0] == 0 && dev->current_dr == 2) /* EXTEST */
      {
        uint32_t a = dev->sreg[2].reg[0];
        uint32_t d = 0;
        uint32_t c = dev->sreg[2].reg[3];

#ifdef VERBOSE
        printf("CAPTURE_DR/EXTEST\n");
#endif

        for(i=0; some_cpu_attached[i].part; i++)
        {
          jim_attached_part_t *tp = &(((jim_attached_part_t*)(dev->state))[i]);
          jim_bus_device_t *b = tp->part;

          /* Address decoder */
          if(tp->offset <= a)
          {
            uint32_t as = (a - (tp->offset)) >> tp->adr_shift;
            if(as < b->size)
            {
              d |= b->capture(b, as, c, shmem, shmem_size) << tp->data_shift;
            }
          }
        }

        /* Store data into data "input" cells in BSR */
        dev->sreg[2].reg[2] = d;
      };
      break;

    case UPDATE_IR:

#ifdef VERBOSE
      printf("UPDATE_IR/");
#endif

      switch(dev->sreg[0].reg[0])
      {
        case 0x0: /* EXTEST */
#ifdef VERBOSE
          printf("EXTEST\n");
#endif
          dev->current_dr = 2;
          break;
        case 0x1: /* IDCODE */
#ifdef VERBOSE
          printf("IDCODE\n");
#endif
          some_cpu_report_idcode(dev);
          break;
        case 0x2: /* SAMPLE */
#ifdef VERBOSE
          printf("SAMPLE\n");
#endif
          dev->current_dr = 2;
          break;
        case 0x3: /* BYPASS */
#ifdef VERBOSE
          printf("BYPASS\n");
#endif
        default:
          dev->current_dr = 0; /* BYPASS */
          break;
      }
      break;

    default:
      break;
  }
}

void some_cpu_tck_fall(jim_device_t *dev,
    uint8_t *shmem, size_t shmem_size )
{
  int i;

  switch(dev->tap_state)
  {
    case UPDATE_DR:

      if(dev->sreg[0].reg[0] == 0 && dev->current_dr == 2) /* EXTEST */
      {
        uint32_t a = dev->sreg[2].reg[0];
        uint32_t d = dev->sreg[2].reg[1];
        uint32_t c = dev->sreg[2].reg[3];

#ifdef VERBOSE
        printf("UPDATE_DR/EXTEST\n");
#endif

        for(i=0; some_cpu_attached[i].part; i++)
        {
          jim_attached_part_t *tp = &(((jim_attached_part_t*)(dev->state))[i]);
          jim_bus_device_t *b = tp->part;

          /* Address decoder */
          if(tp->offset <= a)
          {
            uint32_t as = (a - (tp->offset)) >> tp->adr_shift;
            if(as < b->size)
            {
              b->update(b, as, d >> tp->data_shift, c, shmem, shmem_size);
            }
          }
        }
      };
      break;

    default:
      break;
  }
}

void some_cpu_free(jim_device_t *dev)
{
  int i;

  if(!dev) return;
  if(!dev->state) return;

  for(i=0;some_cpu_attached[i].part;i++)
  { 
    jim_bus_device_t *b = ((jim_attached_part_t*)(dev->state))[i].part;
    if(b->free != NULL) b->free(b);
    free(b);
  }
  free(dev->state);
}

jim_device_t *some_cpu(void)
{
  jim_device_t *dev;
  const int reg_size[3] = { 2 /* IR */, 32 /* IDR */, BSR_LEN /* BSR */ };

  dev = jim_alloc_device(3, reg_size);

  if(dev)
  {
    /* Allocate memory for copies of the original structure for dynamic
     * modifications (e.g. if bus width changes because of some signal) */

    dev->state = malloc(sizeof(some_cpu_attached));
    if(!dev->state)
    {
      free(dev);
      dev = NULL;
    }
    else
    {
      int i;
      dev->tck_rise = some_cpu_tck_rise;
      dev->tck_fall = some_cpu_tck_fall;
      dev->dev_free = some_cpu_free;
      memcpy(dev->state, some_cpu_attached, sizeof(some_cpu_attached));

     for(i=0;some_cpu_attached[i].part;i++)
      {
        jim_bus_device_t **b = &(((jim_attached_part_t*)(dev->state))[i].part);
        *b = malloc(sizeof(jim_bus_device_t));
        if(*b == NULL) break;
        memcpy(*b, some_cpu_attached[i].part, sizeof(jim_bus_device_t));
        (*b)->init(*b);
      };

      if(some_cpu_attached[i].part) /* loop broken; failed to malloc all parts */
      {
        for(i--;i>=0;i--) free(((jim_attached_part_t*)(dev->state))[i].part);
        free(dev->state);
        free(dev);
        dev = NULL;
      }
    }
  }
  return dev;
}

