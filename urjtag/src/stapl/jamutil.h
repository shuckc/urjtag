/****************************************************************************/
/*                                                                          */
/*  Module:         jamutil.h                                               */
/*                                                                          */
/*                  Copyright (C) Altera Corporation 1997                   */
/*                                                                          */
/*  Description:    Prototypes for miscelleneous utility functions          */
/*                                                                          */
/****************************************************************************/

#ifndef INC_JAMUTIL_H
#define INC_JAMUTIL_H

#include <stdint.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <sysdep.h>

static inline int
jam_is_name_char (char ch)
{
    return isalnum(ch) || (ch == '_');
}

static inline char
jam_todigit (uint32_t value)
{
    char c;
    snprintf (&c, 1, "%u", value % 16);
    return c;
}

static inline void
jam_ltoa (char *string, int32_t value)
{
    sprintf (string, "%i", value);
}

#endif /* INC_JAMUTIL_H */
