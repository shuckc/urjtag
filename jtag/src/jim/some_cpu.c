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

extern jim_bus_device_t intel_28f800b3;

jim_attached_part_t some_cpu_attached[] =
{
    { 0x00000000, &intel_28f800b3 },
    { 0xFFFFFFFF, NULL }
};

#define BSR_LEN 202

void some_cpu_report_idcode(jim_device_t *dev)
{
  dev->sreg[0].reg[0] = 0x1; /* IDCODE instruction b0001 */
  dev->sreg[1].reg[0] = 0x87654321; /* Load IDR (fake) */
  dev->current_dr = 1; /* IDR */
}

void some_cpu_extest(char *st, jim_device_t *dev)
{
  int i;

  printf("EXTEST/%s with A=%08X, D=%08X%s%s%s\n", st,
    dev->sreg[2].reg[0], dev->sreg[2].reg[1],
    (dev->sreg[2].reg[2] & 1) ? ", OE":"",
    (dev->sreg[2].reg[2] & 2) ? ", WE":"",
    (dev->sreg[2].reg[2] & 4) ? ", CS":"");

  for(i=0; some_cpu_attached[i].part; i++)
  {
    jim_bus_device_t *b = ((jim_attached_part_t*)(dev->state))[i].part;

    b->access(b, dev->sreg[2].reg[0],
                 dev->sreg[2].reg[1],
                 dev->sreg[2].reg[2]);
  }
}

void some_cpu_tck_rise(jim_device_t *dev, int tms, int tdi)
{
  // jim_print_tap_state(dev);

  switch(dev->tap_state)
  {
    case RESET:
      some_cpu_report_idcode(dev);
      break;

    case UPDATE_DR:
      if(dev->current_dr == 2)
      {
        if(dev->sreg[0].reg[0] == 0) some_cpu_extest("UPDATE_DR", dev);
      };
      break;
 
    case UPDATE_IR:
      switch(dev->sreg[0].reg[0])
      {
        case 0x0: /* EXTEST */
          printf("EXTEST\n");
          dev->current_dr = 2;
          some_cpu_extest("UPDATE_IR", dev);
          break;
        case 0x1: /* IDCODE */
          printf("IDCODE\n");
          some_cpu_report_idcode(dev);
          break;
        case 0x2: /* SAMPLE */
          printf("SAMPLE\n");
          dev->current_dr = 2;
          break;
        case 0x3: /* BYPASS */
          printf("BYPASS\n");
        default:
          dev->current_dr = 0; /* BYPASS */
          break;
      }
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
    dev->state    = malloc(sizeof(some_cpu_attached));
    if(!dev->state)
    {
      free(dev);
      dev = NULL;
    }
    else
    {
      int i;
      dev->tck_rise = some_cpu_tck_rise;
      dev->dev_free = some_cpu_free;
      memcpy(dev->state, some_cpu_attached, sizeof(some_cpu_attached));
      for(i=0;some_cpu_attached[i].part;i++)
      {
        jim_bus_device_t *b = ((jim_attached_part_t*)(dev->state))[i].part;
        b->init(b);
      }
    }
  }

  return dev;
}

