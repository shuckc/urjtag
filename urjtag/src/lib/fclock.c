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
#define _POSIX_C_SOURCE 200112L
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#ifndef __MINGW32__
#include <sys/times.h>
#endif
#include <math.h>
#include <assert.h>

#include <urjtag/fclock.h>

/* ------------------------------------------------------------------ */

#ifdef __APPLE__
#include <mach/mach_time.h>
#include <sys/types.h>
#include <sys/time.h>

long double
urj_lib_frealtime (void)
{
    long double result;
    static uint64_t start_mat;
    static long double start_time;
    static double multiplier;

    mach_timebase_info_data_t mtid;
    struct timeval tv;

    if (!mtid.denom == 0)
    {
        mach_timebase_info (&mtid);
        multiplier = (double) mtid.numer / (double) mtid.denom;
        gettimeofday (&tv, NULL);
        start_time =
            (long double) tv.tv_sec + (long double) tv.tv_usec * 1000.0;
        start_mat = mach_absolute_time ();
    }

    result = start_time + (mach_absolute_time () - start_mat) * multiplier;

    assert (isnormal (result));
    assert (result > 0);
    return result;
}
#else /* def __APPLE__ */

/* ------------------------------------------------------------------ */

#ifdef _POSIX_TIMERS

long double
urj_lib_frealtime (void)
{
    long double result;

    struct timespec t;
    if (clock_gettime (CLOCK_REALTIME, &t) == -1)
    {
        perror ("urj_lib_frealtime (clock_gettime)");
        exit (EXIT_FAILURE);
    }
    result =
        (long double) t.tv_sec + (long double) t.tv_nsec * (long double) 1e-9;

    assert (isnormal (result));
    assert (result > 0);
    return result;
}

#else /* def _POSIX_TIMERS */

/* ------------------------------------------------------------------ */

#ifdef __MINGW32__

#include <sys/timeb.h>

long double
urj_lib_frealtime (void)
{
    long double result;

    struct timeb t;

    ftime (&t);
    result =
        (long double) t.time + (long double) t.millitm * (long double) 1e-3;

    assert (isnormal (result));
    assert (result > 0);
    return result;
}


#else /* def __MINGW32__ */

/* ------------------------------------------------------------------ */

#ifndef CLK_TCK
static clock_t CLK_TCK = 0;
static void set_clk_tck (void) __attribute__ ((constructor));
static void
set_clk_tck (void)
{
    long v = sysconf (_SC_CLK_TCK);
    if (v == -1)
    {
        perror ("sysconf(_SC_CLK_TCK)");
        exit (EXIT_FAILURE);
    }
    CLK_TCK = v;
}
#endif

long double
urj_lib_frealtime (void)
{
    long double result;


    struct tms t;
    clock_t c = times (&t);
    if (c == (clock_t) - 1)
    {
        perror ("urj_lib_frealtime (times)");
        exit (EXIT_FAILURE);
    }
    result = (long double) c / CLK_TCK;

    assert (isnormal (result));
    assert (result > 0);
    return result;
}

#endif
#endif
#endif
