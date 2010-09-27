/*
 * $Id$
 *
 * Functions to parse Xilinx bitstream files
 *
 * Copyright (C) 2010, Michael Walle
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
 * Written by Michael Walle <michael@walle.cc>, 2010
 *
 */

#include <sysdep.h>
#include <string.h>
#include <stdlib.h>

#include <urjtag/log.h>
#include <urjtag/error.h>

#include "xilinx.h"

static int
xlx_read_section (FILE *bit_file, char *id, uint8_t **data, uint32_t *len)
{
    uint8_t buf[4];
    int lenbytes;

    /* first read 1 bytes, the section key */
    if (fread (buf, 1, 1, bit_file) != 1)
        return URJ_STATUS_FAIL;

    *id = buf[0];

    /* section 'e' has 4 bytes indicating section length */
    if (*id == 'e')
        lenbytes = 4;
    else
        lenbytes = 2;

    /* first read 1 bytes */
    if (fread (buf, 1, lenbytes, bit_file) != lenbytes)
        return URJ_STATUS_FAIL;

    /* second and third is section length */
    if (*id != 'e')
        *len = buf[0] << 8 | buf[1];
    else
        *len = buf[0] << 24 | buf[1] << 16 | buf[2] << 8 | buf[3];

    /* now allocate memory for data */
    *data = malloc (*len);

    if (fread (*data, 1, *len, bit_file) != *len)
        return URJ_STATUS_FAIL;

    return URJ_STATUS_OK;
}

int
xlx_bitstream_load_bit (FILE *bit_file, xlx_bitstream_t *bs)
{
    char sid = 0;
    uint8_t *sdata;
    uint32_t slen;

    uint8_t buf[128];
    uint8_t header[] = {
        0x00, 0x09, 0x0f, 0xf0, 0x0f, 0xf0, 0x0f, 0xf0,
        0x0f, 0xf0, 0x00, 0x00, 0x01,
    };

    if (fread (buf, 1, sizeof (header), bit_file) != sizeof (header))
        return URJ_STATUS_FAIL;

    if (memcmp (buf, header, sizeof (header)) != 0)
        return URJ_STATUS_FAIL;

    urj_log (URJ_LOG_LEVEL_DEBUG,
             _("Valid xilinx bitfile header found.\n"));

    while (sid != 'e')
    {
        if (xlx_read_section (bit_file, &sid, &sdata, &slen) != URJ_STATUS_OK)
            return URJ_STATUS_FAIL;

        urj_log (URJ_LOG_LEVEL_DEBUG,
                 _("Read section id=%c len=%d.\n"), sid, slen);

        /* make sure that strings are terminated */
        if (sid != 'e')
            sdata[slen-1] = '\0';

        switch (sid)
        {
            case 'a': bs->design = (char *) sdata; break;
            case 'b': bs->part_name = (char *) sdata; break;
            case 'c': bs->date = (char *) sdata; break;
            case 'd': bs->time = (char *) sdata; break;
            case 'e': bs->data = sdata; bs->length = slen; break;
        }
    }

    return URJ_STATUS_OK;
}

xlx_bitstream_t *
xlx_bitstream_alloc (void)
{
    xlx_bitstream_t *bs = calloc (1, sizeof (xlx_bitstream_t));

    if (!bs)
    {
        urj_error_set (URJ_ERROR_OUT_OF_MEMORY, _("malloc(%zu) fails"),
                       sizeof (xlx_bitstream_t));
        return NULL;
    }

    return bs;
}

void
xlx_bitstream_free (xlx_bitstream_t *bs)
{
    free (bs->design);
    free (bs->part_name);
    free (bs->date);
    free (bs->time);
    free (bs->data);

    free (bs);
}
