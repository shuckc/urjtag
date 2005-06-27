/*
 * $Id$
 *
 * stdint.h - integer types for Hitachi Workbench
 * Copyright (C) 2005 Elcom s.r.o.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holders nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Written by Marcel Telka <marcel@telka.sk>, 2005.
 *
 */

#ifndef STDINT_H
#define	STDINT_H

#include <limits.h>

/*
 * Integer Types
 */

/* Exact-width integer types */

typedef signed char int8_t;
typedef short int16_t;
typedef long int32_t;
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned long uint32_t;

/* Minimum-width integer types */

typedef signed char int_least8_t;
typedef short int_least16_t;
typedef long int_least32_t;
/* int_least64_t not supported */
typedef unsigned char uint_least8_t;
typedef unsigned short uint_least16_t;
typedef unsigned long uint_least32_t;
/* uint_least64_t not supported */

/* Fastest minimum-width integer types */

typedef signed char int_fast8_t;
typedef short int_fast16_t;
typedef long int_fast32_t;
/* int_fast64_t not supported */
typedef unsigned char uint_fast8_t;
typedef unsigned short uint_fast16_t;
typedef unsigned long uint_fast32_t;
/* uint_least64_t not supported */

/* Integer types capable of holding object pointers */

typedef long intptr_t;
typedef unsigned long uintptr_t;

/* Greatest-width integer types */

typedef long intmax_t;
typedef unsigned long uintmax_t;

/*
 * Limits of Specified-Width Interger Types
 */

/* Limits of exact-width integer types */

#define	INT8_MIN		(-127 - 1)
#define	INT16_MIN		(-32767 - 1)
#define	INT32_MIN		(-2147483647 - 1)

#define	INT8_MAX		127
#define	INT16_MAX		32767
#define	INT32_MAX               2147483647

#define	UINT8_MAX		0xFF
#define	UINT16_MAX		0xFFFF
#define	UINT32_MAX		0xFFFFFFFF

/* Limits of minimum-width integer types */

#define	INT_LEAST8_MIN		INT8_MIN
#define	INT_LEAST16_MIN		INT16_MIN
#define	INT_LEAST32_MIN		INT32_MIN

#define	INT_LEAST8_MAX		INT8_MAX
#define	INT_LEAST16_MAX		INT16_MAX
#define	INT_LEAST32_MAX		INT32_MAX

#define	UINT_LEAST8_MAX		UINT8_MAX
#define	UINT_LEAST16_MAX	UINT16_MAX
#define	UINT_LEAST32_MAX	UINT32_MAX

/* Limits of fastest minimum-width integer types */

#define	INT_FAST8_MIN		INT8_MIN
#define	INT_FAST16_MIN		INT16_MIN
#define	INT_FAST32_MIN		INT32_MIN

#define	INT_FAST8_MAX		INT8_MAX
#define	INT_FAST16_MAX		INT16_MAX
#define	INT_FAST32_MAX		INT32_MAX

#define	UINT_FAST8_MAX		UINT8_MAX
#define	UINT_FAST16_MAX		UINT16_MAX
#define	UINT_FAST32_MAX		UINT32_MAX

/* Limits of integer types capable of holding object pointers */

#define	INTPTR_MIN		INT32_MIN
#define	INTPTR_MAX		INT32_MAX
#define	UINTPTR_MAX		UINT32_MAX

/* Limits of greatest-width integer types */

#define	INTMAX_MIN		INT32_MIN
#define	INTMAX_MAX		INT32_MAX
#define	UINTMAX_MAX		UINT32_MAX

/*
 * Limits of Other Integer Types
 */

/* Limits of ptrdiff_t */

#define	PTRDIFF_MIN		INT32_MIN
#define	PTRDIFF_MAX		UINT32_MAX

/* Limits of sig_atomic_t */

/* N/A for Hitachi Workbench */

/* Limit of size_t */

#define	SIZE_MAX		UINT32_MAX

/* Limits of wchar_t */

#define	WCHAR_MIN		CHAR_MIN
#define	WCHAR_MAX		CHAR_MAX

/* Limits of wint_t */

/* wint_t not supported in Hitachi Workbench */

/*
 * Macros for Integer Constant Expressions
 */

/* Macros for minimum-width integer constant expressions */

#define INT8_C(value)           (value)
#define INT16_C(value)          (value)
#define INT32_C(value)          (value)

#define UINT8_C(value)          (value)
#define UINT16_C(value)         (value)
#define UINT32_C(value)         (value)

/* Macros for greatest-width integer constant expressions */

#define INTMAX_C(value)         INT32_C(value)
#define UINTMAX_C(value)        UINT32_C(value)

#endif /* STDINT_H */
