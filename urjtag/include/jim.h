/*
 * $Id: jim.h $
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
 * Documentation used while writing this code:
 *
 * http://www.inaccessnetworks.com/projects/ianjtag/jtag-intro/jtag-intro.html
 *   "A Brief Introduction to the JTAG Boundary Scan Interface", Nick Patavalis
 *
 * http://www.xjtag.com/support-jtag/jtag-technical-guide.php
 *   "JTAG - A technical overview", XJTAG Ltd.
 *
 */

#ifndef JIM_H
#define JIM_H 1

#include <stdint.h>
#include <stdlib.h>

typedef enum 
{
  RESET       = 0,
  SELECT_DR   = 0+1,
  CAPTURE_DR  = 0+2,
  SHIFT_DR    = 0+3,
  EXIT1_DR    = 0+4,
  PAUSE_DR    = 0+5,
  EXIT2_DR    = 0+6,
  UPDATE_DR   = 0+7,
  IDLE        = 8,
  SELECT_IR   = 8+1,
  CAPTURE_IR  = 8+2,
  SHIFT_IR    = 8+3,
  EXIT1_IR    = 8+4,
  PAUSE_IR    = 8+5,
  EXIT2_IR    = 8+6,
  UPDATE_IR   = 8+7,
}
tap_state_t;

typedef struct
{
  uint32_t *reg;
  int len;
} shift_reg_t;

typedef struct jim_device
{
  struct jim_device *prev;

  tap_state_t tap_state;
  void (*tck_rise)(struct jim_device *dev, int tms, int tdi, uint8_t *shmem, size_t shmem_size);
  void (*tck_fall)(struct jim_device *dev, uint8_t *shmem, size_t shmem_size);
  void (*dev_free)(struct jim_device *dev);
  void *state;
  int num_sregs;
  int current_dr;
  shift_reg_t *sreg;
  int tdo;
  int tdo_buffer;
}
jim_device_t;

typedef struct jim_state
{
  int trst;
  uint8_t *shmem;
  size_t shmem_size;
  jim_device_t *last_device_in_chain;
}
jim_state_t;

typedef struct jim_bus_device
{
    int width; /* bytes */
    int size ; /* words (each <width> bytes) */
	void *state;    /* device-dependent */
    void (*init)(struct jim_bus_device *x);
    uint32_t (*capture)(struct jim_bus_device *x,
				uint32_t address, uint32_t control,
                uint8_t *shmem, size_t shmem_size);
    void (*update)(struct jim_bus_device *x,
				uint32_t address, uint32_t data, uint32_t control,
                uint8_t *shmem, size_t shmem_size);
	void (*free)(struct jim_bus_device *x);
}
jim_bus_device_t;

typedef struct
{
    uint32_t offset;
    int adr_shift;
    int data_shift;
	jim_bus_device_t *part;
}
jim_attached_part_t;

void jim_set_trst(jim_state_t *s, int trst);
int jim_get_trst(jim_state_t *s);
int jim_get_tdo(jim_state_t *s);
void jim_tck_rise(jim_state_t *s, int tms, int tdi);
void jim_tck_fall(jim_state_t *s);
jim_device_t *jim_alloc_device(int num_sregs, const int reg_size[]);
jim_state_t *jim_init(void);
void jim_free(jim_state_t *s);
void jim_print_sreg(shift_reg_t *r);
void jim_print_tap_state(char *rof, jim_device_t *dev);

#endif

