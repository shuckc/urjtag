/*
 * Functions to parse Lattice bitstream files
 *
 * Copyright (C) 2012, Chris Shucksmith
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
 * Written by Chris Shucksmith <chris@shucksmith.co.uk>, 2012
 *
 */

#include <sysdep.h>
#include <string.h>
#include <stdlib.h>

#include <urjtag/log.h>
#include <urjtag/error.h>

#include "lattice.h"

int
lat_bitstream_load_bit (FILE *bit_file, lat_bitstream_t *bs)
{

    // typedef struct {
    //     char *text;
    //     lat_header_t *next;
    // } lat_header_t;

    // typedef struct {
    //     char *filename;
    //     lat_header_t *header;
    //     uint32_t   length;
    //     uint8_t    *data;
    // } lat_bitstream_t;

    int status = URJ_STATUS_FAIL;

    /* Get file size */
    fseek(bit_file, 0L, SEEK_END);
    uint32_t file_size = ftell(bit_file);
    fseek(bit_file, 0L, SEEK_SET);

    /* header: 0xff 0x00 */
    if (fgetc(bitfile) != 0xff)
        return URJ_STATUS_FAIL;
    if (fgetc(bitfile) != 0x00)
        return URJ_STATUS_FAIL;
    urj_log (URJ_LOG_LEVEL_DEBUG, _("Valid lattice header found.\n"));

    // read null-strings, until the value starting 0xff, which is the bitstream
    // lat_bitstream_t *bs = malloc(sizeof(lat_bitstream_t));
    lat_header_t *prev = NULL;

    char buf[500];
    int nextchar = 0;
    while (r != 0xff)
    {
        buf[nextchar++] = fgetc(bit_file);
        if (r == '\0')
        {
            headerEntry->text = strdup(buf);
            nextchar = 0;
            lat_header_t *headerEntry = malloc(sizeof(lat_header_t));

            // preserve order, insert at tail
            if (prev != NULL) prev->next = headerEntry;
            prev = headerEntry;
            if (bs->header == NULL) bs->header = headerEntry;
        }        
    }

    /* positioned at the start of the raw bitsream data, calculate size */
    uint32_t pos = ftell(bit_file);
    bs->header = bitstreamsz - pos + 1;

    /* allocate memory for bitstream */
    bs->data = malloc(bs->header);

    if (fread (bs->data, 1, bs->header, bit_file) != bs->header)
        goto fail_free;


    return URJ_STATUS_OK;

  fail_free:
    free(bs->data);
    free(bs->header);
    // TODO: free chain elements
    return status;
}

lat_bitstream_t *
lat_bitstream_alloc (void)
{
    lat_bitstream_t *bs = calloc (1, sizeof (lat_bitstream_t));

    if (!bs)
    {
        urj_error_set (URJ_ERROR_OUT_OF_MEMORY, _("malloc(%zu) fails"),
                       sizeof (lat_bitstream_t));
        return NULL;
    }

    return bs;
}

void
lat_bitstream_free (lat_bitstream_t *bs)
{
    free(bs->data);

    lat_header_t *prev = bs->header;
    lat_header_t *tmp;
    while (prev != NULL)
    {
        tmp = prev;
        prev = prev->next;
        free(tmp->text);
        free(tmp);
    }
    
    free (bs);
}
