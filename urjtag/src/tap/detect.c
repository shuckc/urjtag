/*
 * $Id$
 *
 * Copyright (C) 2002 ETC s.r.o.
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
 * Written by Marcel Telka <marcel@telka.sk>, 2002.
 *
 */

#include <urjtag/sysdep.h>

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include <urjtag/cmd.h>

#include <urjtag/bsdl.h>

#include <urjtag/tap_register.h>
#include <urjtag/tap.h>
#include <urjtag/cable.h>
#include <urjtag/part.h>
#include <urjtag/chain.h>
#include <urjtag/part.h>
#include <urjtag/bus.h>
#include <urjtag/data_register.h>
#include <urjtag/jtag.h>

struct id_record
{
    char name[20];
    char fullname[100];
};

static int
find_record (char *filename, urj_tap_register_t *key, struct id_record *idr)
{
    FILE *file;
    urj_tap_register_t *tr;
    int r = 0;

    file = fopen (filename, "r");
    if (!file)
    {
        printf (_("Cannot open %s\n"), filename);
        return 0;
    }

    tr = urj_tap_register_alloc (key->len);

    for (;;)
    {
        char *p;
        char *s;
        char line[1024];

        if (fgets (line, 1024, file) == NULL)
            break;

        /* remove comment and nl from the line */
        p = strpbrk (line, "#\n");
        if (p)
            *p = '\0';

        p = line;

        /* skip whitespace */
        while (*p && isspace (*p))
            p++;

        /* remove ending whitespace */
        s = strchr (p, '\0');
        while (s != p)
        {
            if (!isspace (*--s))
                break;
            *s = '\0';
        }

        /* line is empty? */
        if (!*p)
            continue;

        /* find end of field */
        s = p;
        while (*s && !isspace (*s))
            s++;
        if (*s)
            *s++ = '\0';

        /* test field length */
        if (strlen (p) != key->len)
            continue;

        /* match */
        urj_tap_register_init (tr, p);
        if (urj_tap_register_compare (tr, key))
            continue;

        /* next field */
        p = s;

        /* skip whitespace */
        while (*p && isspace (*p))
            p++;

        /* line is empty? */
        if (!*p)
            continue;

        /* find end of field */
        s = p;
        while (*s && !isspace (*s))
            s++;
        if (*s)
            *s++ = '\0';

        /* test field length */
        if (strlen (p) >= sizeof idr->name)
            continue;

        /* copy name */
        strcpy (idr->name, p);

        /* next field */
        p = s;

        /* skip whitespace */
        while (*p && isspace (*p))
            p++;

        /* line is empty? */
        if (!*p)
            continue;

        /* test field length */
        if (strlen (p) >= sizeof idr->fullname)
            continue;

        /* copy fullname */
        strcpy (idr->fullname, p);

        r = 1;
        break;
    }

    fclose (file);

    urj_tap_register_free (tr);

    return r;
}

static uint64_t
bits_to_uint64 (urj_tap_register_t *t)
{
    int i;
    uint64_t l, b;

    l = 0;
    b = 1;
    for (i = 0; i < t->len; i++)
    {
        if (t->data[i] & 1)
            l |= b;
        b <<= 1;
    }
    return l;
}


int
urj_tap_detect_parts (urj_chain_t *chain, const char *db_path)
{
    int irlen;
    urj_tap_register_t *ir;
    int chlen;
    urj_tap_register_t *one;
    urj_tap_register_t *ones;
    urj_tap_register_t *br;
    urj_tap_register_t *id;
    urj_parts_t *ps;
    int i;

    char data_path[1024];
    char *cmd[3] = { "include", data_path, NULL };
    char manufacturer[URJ_PART_MANUFACTURER_MAXLEN + 1];
    char partname[URJ_PART_PART_MAXLEN + 1];
    char stepping[URJ_PART_STEPPING_MAXLEN + 1];

    /* Detect IR length */
    urj_tap_reset (chain);
    urj_tap_capture_ir (chain);
    irlen = urj_tap_detect_register_size (chain);
    if (irlen < 1)
        return 0;

    printf (_("IR length: %d\n"), irlen);
    chain->total_instr_len = irlen;

    /* Allocate IR */
    ir = urj_tap_register_fill (urj_tap_register_alloc (irlen), 1);
    if (ir == NULL)
    {
        printf (_("out of memory\n"));
        return 0;
    }

    urj_tap_shift_register (chain, ir, NULL, URJ_CHAIN_EXITMODE_IDLE);
    urj_tap_register_free (ir);

    /* Detect chain length */
    urj_tap_capture_dr (chain);
    chlen = urj_tap_detect_register_size (chain);
    if (chlen < 1)
    {
        printf (_("Unable to detect JTAG chain length\n"));
        return 0;
    }
    printf (_("Chain length: %d\n"), chlen);

    /* Allocate registers and parts */
    one = urj_tap_register_fill (urj_tap_register_alloc (1), 1);
    ones = urj_tap_register_fill (urj_tap_register_alloc (31), 1);
    br = urj_tap_register_alloc (1);
    id = urj_tap_register_alloc (32);
    ps = urj_part_parts_alloc ();
    if (!one || !ones || !br || !id || !ps)
    {
        printf (_("out of memory\n"));

        urj_tap_register_free (one);
        urj_tap_register_free (ones);
        urj_tap_register_free (br);
        urj_tap_register_free (id);
        urj_part_parts_free (ps);
        return 0;
    }
    chain->parts = ps;
    chain->active_part = 0;

    /* Detect parts */
    urj_tap_reset (chain);
    urj_tap_capture_dr (chain);

    for (i = 0; i < chlen; i++)
    {
        urj_part_t *part;
        urj_tap_register_t *did = br;   /* detected id (length is 1 or 32) */
        urj_tap_register_t *key;
        struct id_record idr;
        char *p;

        urj_tap_shift_register (chain, one, br, URJ_CHAIN_EXITMODE_SHIFT);
        if (urj_tap_register_compare (one, br) == 0)
        {
            /* part with id */
            urj_tap_shift_register (chain, ones, id,
                                    URJ_CHAIN_EXITMODE_SHIFT);
            urj_tap_register_shift_left (id, 1);
            id->data[0] = 1;
            did = id;
        }

        printf (_("Device Id: %s (0x%016" PRIX64 ")\n"),
                urj_tap_register_get_string (did), bits_to_uint64 (did));

        part = urj_part_alloc (did);
        if (part == NULL)
        {
            printf (_("Out of memory\n"));
            break;
        }
        urj_part_parts_add_part (ps, part);

        if (did == br)
            continue;

        chain->active_part = ps->len - 1;

#ifdef ENABLE_BSDL
        if (urj_bsdl_scan_files (chain, urj_tap_register_get_string (did),
                                 URJ_BSDL_MODE_DETECT) <= 0)
        {
#endif

            /* find JTAG declarations for a part with id */

            strcpy (data_path, db_path);        /* FIXME: Buffer overrun */

            /* manufacturers */
            strcat (data_path, "/MANUFACTURERS");

            key = urj_tap_register_alloc (11);
            memcpy (key->data, &id->data[1], key->len);
            if (!find_record (data_path, key, &idr))
            {
                printf (_("  Unknown manufacturer!\n"));
                urj_tap_register_free (key);
                continue;
            }
            urj_tap_register_free (key);

            printf (_("  Manufacturer: %s\n"), idr.fullname);
            if (strlen (idr.fullname) > URJ_PART_MANUFACTURER_MAXLEN)
                printf (_("Warning: Manufacturer too long\n"));
            strncpy (manufacturer, idr.fullname,
                     URJ_PART_MANUFACTURER_MAXLEN);
            manufacturer[URJ_PART_MANUFACTURER_MAXLEN] = '\0';

            /* parts */
            p = strrchr (data_path, '/');
            if (p)
                p[1] = '\0';
            else
                data_path[0] = '\0';
            strcat (data_path, idr.name);
            strcat (data_path, "/PARTS");

            key = urj_tap_register_alloc (16);
            memcpy (key->data, &id->data[12], key->len);
            if (!find_record (data_path, key, &idr))
            {
                printf (_("  Unknown part!\n"));
                urj_tap_register_free (key);
                continue;
            }
            urj_tap_register_free (key);

            printf (_("  Part(%d):         %s\n"), chain->active_part,
                    idr.fullname);
            if (strlen (idr.fullname) > URJ_PART_PART_MAXLEN)
                printf (_("Warning: Part too long\n"));
            strncpy (partname, idr.fullname, URJ_PART_PART_MAXLEN);
            partname[URJ_PART_PART_MAXLEN] = '\0';

            /* steppings */
            p = strrchr (data_path, '/');
            if (p)
                p[1] = '\0';
            else
                data_path[0] = '\0';
            strcat (data_path, idr.name);
            strcat (data_path, "/STEPPINGS");

            key = urj_tap_register_alloc (4);
            memcpy (key->data, &id->data[28], key->len);
            if (!find_record (data_path, key, &idr))
            {
                printf (_("  Unknown stepping!\n"));
                urj_tap_register_free (key);
                continue;
            }
            urj_tap_register_free (key);

            printf (_("  Stepping:     %s\n"), idr.fullname);
            if (strlen (idr.fullname) > URJ_PART_STEPPING_MAXLEN)
                printf (_("Warning: Stepping too long\n"));
            strncpy (stepping, idr.fullname, URJ_PART_STEPPING_MAXLEN);
            stepping[URJ_PART_STEPPING_MAXLEN] = '\0';

            /* part definition file */
            p = strrchr (data_path, '/');
            if (p)
                p[1] = '\0';
            else
                data_path[0] = '\0';
            strcat (data_path, idr.name);

            printf (_("  Filename:     %s\n"), data_path);

            /* run JTAG declarations */
            strcpy (part->manufacturer, manufacturer);
            strcpy (part->part, partname);
            strcpy (part->stepping, stepping);
            urj_cmd_run (chain, cmd);
#ifdef ENABLE_BSDL
        }
#endif

        if (part->active_instruction == NULL)
            part->active_instruction =
                urj_part_find_instruction (part, "IDCODE");
    }

    for (i = 0; i < 32; i++)
    {
        urj_tap_shift_register (chain, one, br, URJ_CHAIN_EXITMODE_SHIFT);
        if (urj_tap_register_compare (one, br) != 0)
        {
            printf (_("Error: Unable to detect JTAG chain end!\n"));
            break;
        }
    }
    urj_tap_shift_register (chain, one, NULL, URJ_CHAIN_EXITMODE_IDLE);

    urj_tap_register_free (one);
    urj_tap_register_free (ones);
    urj_tap_register_free (br);
    urj_tap_register_free (id);

    return ps->len;
}


/* In case we do not want to detect, we can add parts manually */

int
urj_tap_manual_add (urj_chain_t *chain, int instr_len)
{
    urj_tap_register_t *id;
    urj_part_t *part;
    char *cmd[] = { NULL, NULL, NULL, NULL, NULL };
    char *str;
    int result;

    id = urj_tap_register_alloc (1);
    if (id == NULL)
    {
        printf (_("Error: Unable to allocate a register!\n"));
        return 0;
    }

    /* if there are no parts, create the parts list */
    if (chain->parts == NULL)
    {
        chain->parts = urj_part_parts_alloc ();
        if (chain->parts == NULL)
        {
            printf (_("Error: Unable to allocate space for parts!\n"));
            return 0;
        }
    }

    part = urj_part_alloc (id);
    if (part == NULL)
    {
        printf (_("Error: Unable to allocate space for a part!\n"));
        return 0;
    }

    strncpy (part->part, "unknown", URJ_PART_PART_MAXLEN);
    part->instruction_length = instr_len;

    urj_part_parts_add_part (chain->parts, part);
    chain->active_part = chain->parts->len - 1;

    /* make the BR register available */
    if (urj_part_data_register_define (part, "BR", 1) != URJ_STATUS_OK)
    {
        printf (_("Error: could not set BR register"));
        return 0;
    }

    /* create a string of 1's for BYPASS instruction */
    cmd[0] = "instruction";
    cmd[1] = "BYPASS";
    cmd[3] = "BR";
    cmd[4] = NULL;
    str = calloc (instr_len + 1, sizeof (char));
    if (str == NULL)
    {
        printf (_("Out of memory!\n"));
        return 0;
    }

    memset (str, '1', instr_len);
    str[instr_len] = '\0';
    cmd[2] = str;
    result = urj_cmd_run (chain, cmd);
    free (str);

    if (result < 1)
    {
        printf (_("Error: could not set BYPASS instruction"));
        return 0;
    }

    /* update total instruction register length of chain */
    chain->total_instr_len += instr_len;

    return chain->parts->len;
}

int
urj_tap_detect (urj_chain_t *chain)
{
    int i;
    urj_bus_t *abus;

    urj_bus_buses_free ();
    urj_part_parts_free (chain->parts);
    chain->parts = NULL;
    urj_tap_detect_parts (chain, urj_get_data_dir ());
    if (!chain->parts)
    {
        urj_error_set (URJ_ERROR_INVALID, "chain has no parts");
        return URJ_STATUS_FAIL;
    }
    if (!chain->parts->len)
    {
        urj_part_parts_free (chain->parts);
        chain->parts = NULL;
        urj_error_set (URJ_ERROR_INVALID, "chain has empty parts list");
        return URJ_STATUS_FAIL;
    }
    urj_part_parts_set_instruction (chain->parts, "SAMPLE/PRELOAD");
    urj_tap_chain_shift_instructions (chain);
    urj_tap_chain_shift_data_registers (chain, 1);
    urj_part_parts_set_instruction (chain->parts, "BYPASS");
    urj_tap_chain_shift_instructions (chain);

    // Initialize all the buses
    for (i = 0; i < urj_buses.len; i++)
    {
        abus = urj_buses.buses[i];
        if (abus->driver->init)
        {
            if (abus->driver->init (abus) != URJ_STATUS_OK)
                // retain error state
                return URJ_STATUS_FAIL;
        }
    }

    return URJ_STATUS_OK;
}
