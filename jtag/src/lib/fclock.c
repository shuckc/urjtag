/*
 * fclock.c
 *
 * Copyright (C) 2005 Hein Roehrig
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
 */

#define _ISOC99_SOURCE
#define _POSIX_C_SOURCE 199309L
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/times.h>
#include <math.h>
#include <assert.h>


#ifndef CLK_TCK
static clock_t CLK_TCK = 0;
static void set_clk_tck(void) __attribute__ ((constructor));
static void set_clk_tck(void)
{
    long v = sysconf(_SC_CLK_TCK);
    if (v == -1) {
        perror("sysconf(_SC_CLK_TCK)");
        exit(EXIT_FAILURE);
    }
    CLK_TCK = v;
}
#endif


long double
frealtime()
{
    long double result;
#ifdef _POSIX_TIMERS
    struct timespec t;    
    if (clock_gettime(CLOCK_REALTIME, &t)==-1) {
        perror("frealtime (clock_gettime)");
        exit(EXIT_FAILURE);
    }
    result = (long double)t.tv_sec + (long double)t.tv_nsec*(long double)1e-9;
#else
    struct tms t;
    clock_t c=times(&t);
    if (c==(clock_t)-1) {
        perror("frealtime (times)");
        exit(EXIT_FAILURE);
    }
    result = (long double)c/CLK_TCK;
#endif
    assert(isnormal(result));
    assert(result > 0);
    return result;
}


long double
fcputime()
{
    struct tms t;
    clock_t c=times(&t);
    if (c==(clock_t)-1) {
        perror("fcputime (times)");
        exit(EXIT_FAILURE);
    }
    return ((long double)t.tms_utime+t.tms_stime)/CLK_TCK;
}

