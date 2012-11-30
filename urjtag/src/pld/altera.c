/*
 * Driver for Altera FPGAs
 *
 * Copyright (C) 2010, Chris Shucksmith
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
#include <dirent.h>

#include <urjtag/tap.h>
#include <urjtag/part.h>
#include <urjtag/chain.h>
#include <urjtag/tap_state.h>
#include <urjtag/tap_register.h>
#include <urjtag/data_register.h>
#include <urjtag/part_instruction.h>
#include <urjtag/pld.h>
#include <urjtag/bitops.h>
#include "altera.h"

static alt_device_config_t* alt_lookup_device_parameters(urj_pld_t *pld, uint32_t idcode);
static char* alt_bsdl_scan_for_filename (urj_chain_t *chain, const char *filename);
static void alt_free_device_parameters( alt_device_config_t* dev);
static int alt_instruction_resize_dr (urj_part_t *part, const char *ir_name, const char *dr_name, int dr_len);

static int
alt_set_ir_and_shift (urj_chain_t *chain, urj_part_t *part, char *iname)
{
    urj_part_set_instruction (part, iname);
    if (part->active_instruction == NULL)
    {
        urj_error_set (URJ_ERROR_PLD, "unknown instruction '%s'", iname);
        return URJ_STATUS_FAIL;
    }
    urj_tap_chain_shift_instructions (chain);

    return URJ_STATUS_OK;
}

static int
alt_write_register (urj_pld_t *pld, uint32_t reg, uint32_t value)
{
    urj_log (URJ_LOG_LEVEL_ERROR, _("altera: write register unsupported.\n"));
    return URJ_STATUS_FAIL;
}

static int
alt_read_register (urj_pld_t *pld, uint32_t reg, uint32_t *value)
{
    urj_log (URJ_LOG_LEVEL_ERROR, _("altera: read register unsupported.\n"));
    return URJ_STATUS_FAIL;
}

static int
alt_print_status (urj_pld_t *pld)
{
    urj_chain_t *chain = pld->chain;
    urj_part_t *part = pld->part;
    urj_tap_register_t *r;

    /* set all devices in bypass mode */
    urj_tap_reset_bypass (chain);

    /* CHECK_STATUS instruction emulates a SAMPLE operation - status pins
      are not part of the usual boundary scan register. The register has three bits of data for
      each boundary scan element */

    if (alt_set_ir_and_shift (chain, part, "CHECK_STATUS") != URJ_STATUS_OK)
    {
         urj_log (URJ_LOG_LEVEL_ERROR, _("altera: unable to shift CHECK_STATUS instruction\n"));
         return URJ_STATUS_FAIL;
    }

    urj_tap_chain_shift_data_registers (chain, 1);

    r = part->active_instruction->data_register->out;

    uint32_t idcode = urj_tap_register_get_value (part->id);
    alt_device_config_t* dev = alt_lookup_device_parameters(pld, idcode);
    
    /** scan chain register is 3 bits per position, reversed. We need position at jseq_conf_done 
     * and it's the second bit of each three with the status
     *
     */
    uint32_t pinIdx = (dev->jseq_max - dev->jseq_conf_done) * 3 + 1;
    
    urj_log (URJ_LOG_LEVEL_NORMAL, _("Device Configuration Bits\n"));
    urj_log (URJ_LOG_LEVEL_NORMAL, _("\tCONF_DONE: %d\n"), r->data[pinIdx]);
    return URJ_STATUS_OK;
}

static int
alt_configure (urj_pld_t *pld, FILE *bit_file)
{
    urj_chain_t *chain = pld->chain;
    urj_part_t *part = pld->part;
    urj_part_instruction_t *i;
    uint32_t u;
    int dr_len;
    char *dr_data;
    int status = URJ_STATUS_OK;

    /* set all devices in bypass mode */
    urj_tap_reset_bypass (chain);

    /* Get file size */
    fseek(bit_file, 0L, SEEK_END);
    uint32_t file_size = ftell(bit_file);
    fseek(bit_file, 0L, SEEK_SET);

    uint32_t idcode = urj_tap_register_get_value (part->id);
 
    /* TODO: certain parts need some header bytes skipping
     * Cyclone / Cyclone II   */
    if ((idcode>0x2080000 && idcode<0x2086000) || (idcode>=0x20B10DD && idcode<=0x20B60DD))
    {
      fseek(bit_file, 44L, SEEK_SET);
      file_size -= 44;
    }

    if(file_size == 0)
    {
       urj_log (URJ_LOG_LEVEL_ERROR, _("Bitstream length could not be determined or was empty.\n"));
       return URJ_STATUS_FAIL;
    }

    urj_log (URJ_LOG_LEVEL_NORMAL, _("Bitstream information:\n"));
    urj_log (URJ_LOG_LEVEL_NORMAL, _("\tLength: %d bytes\n"), file_size);

    /* allocate memory for data */
    char *data = malloc(file_size);

    if (fread (data, 1, file_size, bit_file) != file_size)
        goto fail_free;

    dr_len = file_size * 8;

    if (alt_instruction_resize_dr (part, "PROGRAM", "BITST", dr_len)
            != URJ_STATUS_OK)
    {
        urj_log (URJ_LOG_LEVEL_ERROR, _("Bitstream instruction resize failed:\n"));
        goto fail_free;
    }
    i = urj_part_find_instruction (part, "PROGRAM");

    /* copy data into shift register */
    dr_data = i->data_register->in->data;
    for (u = 0; u < file_size; u++)
    {
        /* flip bits */
        dr_data[8*u+7] = (data[u] & 0x80) ? 1 : 0;
        dr_data[8*u+6] = (data[u] & 0x40) ? 1 : 0;
        dr_data[8*u+5] = (data[u] & 0x20) ? 1 : 0;
        dr_data[8*u+4] = (data[u] & 0x10) ? 1 : 0;
        dr_data[8*u+3] = (data[u] & 0x08) ? 1 : 0;
        dr_data[8*u+2] = (data[u] & 0x04) ? 1 : 0;
        dr_data[8*u+1] = (data[u] & 0x02) ? 1 : 0;
        dr_data[8*u+0] = (data[u] & 0x01) ? 1 : 0;
    }

    if (alt_set_ir_and_shift (chain, part, "PROGRAM") != URJ_STATUS_OK)
    {
        status = URJ_STATUS_FAIL;
        goto fail_free;
    }

    /* push entire bitstream out through dr */    
    urj_tap_chain_shift_data_registers (chain, 0);

    alt_print_status(pld);

    /* resume USER operation from program mode */
    if (alt_set_ir_and_shift (chain, part, "STARTUP") != URJ_STATUS_OK)
    {
        status = URJ_STATUS_FAIL;
        goto fail_free;
    }

    urj_tap_reset_bypass (chain);

    urj_tap_chain_flush (chain);

 fail_free:
    free(data);
    return status;
}

static int
alt_reconfigure (urj_pld_t *pld)
{
    urj_chain_t *chain = pld->chain;
    urj_part_t *part = pld->part;

    urj_tap_reset_bypass (chain);

    /* instruction acts the same as pulse to nCONFIG pin */
    if (alt_set_ir_and_shift (chain, part, "PULSE_NCONFIG") != URJ_STATUS_OK)
        return URJ_STATUS_FAIL;

    urj_tap_reset (chain);
    urj_tap_chain_flush (chain);

    return URJ_STATUS_OK;
}

static int
alt_instruction_resize_dr (urj_part_t *part, const char *ir_name, const char *dr_name, int dr_len)
{
    urj_data_register_t *d;
    urj_part_instruction_t *i;

    i = urj_part_find_instruction (part, ir_name);

    if (i == NULL)
    {
        urj_error_set (URJ_ERROR_PLD, "unknown instruction '%s'", ir_name);
        return URJ_STATUS_FAIL;
    }

    d = urj_part_find_data_register (part, dr_name);

    if (d == NULL)
    {
        d = urj_part_data_register_alloc (dr_name, dr_len);
        d->next = part->data_registers;
        part->data_registers = d;
    }
    else if (d->in->len != dr_len)
    {
        /* data register length does not match */
        urj_part_data_register_realloc (d, dr_len);
    }

    i->data_register = d;

    return URJ_STATUS_OK;
}

static int
alt_detect (urj_pld_t *pld)
{
    urj_part_t *part = pld->part;
    uint32_t idcode;
    uint32_t manufacturer;

    /* there's no obvious grouping of family within part range. For now accept all Altera devices */

    idcode = urj_tap_register_get_value (part->id);
    manufacturer = ((idcode >> 1) & 0x7FF);
    urj_log (URJ_LOG_LEVEL_NORMAL, _("Altera detect for manuf %08x\n"), manufacturer);

    switch (manufacturer)
    {
        case ALTERA_IDCODE_MANUF:
            break;
        default:
            return URJ_STATUS_FAIL;
    }

    /* consult the Altera device configuration data for specific device compatibility */
    alt_device_config_t* dev = alt_lookup_device_parameters(pld, idcode);
    if (!dev) return URJ_STATUS_FAIL;
    
    urj_log (URJ_LOG_LEVEL_NORMAL, _("Detected part:\n"));
    urj_log (URJ_LOG_LEVEL_NORMAL, _("\tFamily: %s\n"), dev->family);
    urj_log (URJ_LOG_LEVEL_NORMAL, _("\tDevice: %s\n"), dev->device);
    urj_log (URJ_LOG_LEVEL_NORMAL, _("\tIDCODE: %08x\n"), dev->idcode);
    urj_log (URJ_LOG_LEVEL_NORMAL, _("\tSTATUS pins: %d (STATUS dr-length %d)\n"), dev->jseq_max,  dev->jseq_max*3);
    urj_log (URJ_LOG_LEVEL_NORMAL, _("\tCONF_DONE pin: %d\n"), dev->jseq_conf_done);
    
    /* add instructions that Altera do not include in bsdl files. Test first if they exist
     * to allow bsdl files to overwrite them.
     *   STARTUP       - bring device of program mode into 'USER' design operating
     *   CHECK_STATUS  - read back status pins including CONF_DONE
     *   PULSE_NCONFIG - start configuration cycle
     */
    if (!urj_part_find_instruction(part, "STARTUP"))
       urj_part_instruction_define (part, "STARTUP", "0000000011", "BYPASS");

    if (!urj_part_find_instruction(part, "PROGRAM"))
       urj_part_instruction_define (part, "PROGRAM", "0000000010", "BYPASS");

     if (!urj_part_find_instruction(part, "PULSE_NCONFIG")) 
       urj_part_instruction_define (part, "PULSE_NCONFIG", "0000000001", "BYPASS");

     if (!urj_part_find_instruction(part, "CHECK_STATUS")) 
     {
        /* length of the status register is part specific. Three bits required for each item 
         * in the register, much like a boundary scan
         */
	urj_part_data_register_define(part, "STATUS", dev->jseq_max*3 );
        urj_part_instruction_define (part, "CHECK_STATUS", "0000001000", "STATUS");
    }
 
    alt_free_device_parameters(dev);
 
    return URJ_STATUS_OK;
}

static alt_device_config_t* alt_lookup_device_parameters(urj_pld_t *pld, uint32_t idcode) {

    const char* datafile = "altera_pld_config.csv";
    char family [80];
    char device [80];
    uint32_t jidcode, jseq_max, jseq_conf_done;
    char buffer[1024];
    uint32_t i;
    
    char* filename = alt_bsdl_scan_for_filename(pld->chain, datafile);
    alt_device_config_t* retval = NULL;

    if (filename == NULL) 
    {
        urj_log (URJ_LOG_LEVEL_ERROR, _("Altera PLD part configuration empty; cannot locate file %s in bsdl search path\n"), datafile);
        return NULL;
    }

   FILE *file = fopen(filename, "r");

   urj_log (URJ_LOG_LEVEL_NORMAL, _("Altera PLD part configuration loading from %s\n"), filename);

   free(filename);

   if ( file )
   {
      for ( i = 0; fgets(buffer, sizeof buffer, file); ++i ) 
      {
         /*
          * Parse the comma-separated values from each line into 'array'.
	  * skip blank lines and those that begin with #
	  * format:  Family,Part,IDCODE,STATUS_pins, CONF_DONE_pinpos
          */
          if (strncmp(buffer, "#", 1) == 0) continue;
          if (strncmp(buffer, "\n", 1) == 0) continue;
          if (strncmp(buffer, " \n", 1) == 0) continue;

          sscanf(buffer, "%[^,\n]%*c%[^,\n]%*c%x,%d,%d,%d", family, device, &jidcode, &jseq_max, &jseq_conf_done);
          urj_log(URJ_LOG_LEVEL_DEBUG, _("   part %20s %20s %08x  %d %d\n"), family, device, jidcode, jseq_max, jseq_conf_done);

          if (jidcode == idcode && !retval)
          {

            urj_log(URJ_LOG_LEVEL_NORMAL, _("   part %20s %20s %08x  %d %d\n"), family, device, jidcode, jseq_max, jseq_conf_done);

            /* populate alt_device_config_t */
            retval = malloc(sizeof(alt_device_config_t));

            retval->family = strdup(family);
            retval->device = strdup(device);
            retval->idcode = idcode;
            retval->jseq_max = jseq_max;
            retval->jseq_conf_done = jseq_conf_done;

          }
      }
      fclose(file);

   }
   else /* fopen() returned NULL */
   {
      perror(filename);
   }
   return retval;
}

static void alt_free_device_parameters( alt_device_config_t* dev)
{
    if (dev)
    {
           free(dev->family);
           free(dev->device);
           free(dev);    
    }
}

/*****************************************************************************
 * alt_bsdl_scan_for_filename( chain, filename, path )
 *
 * Scans through all files found via the elements in bsdl_path_list looking
 * for a specific filename. If found, return the full path to the file suitable
 * for passing to fopen(...)
 *
 * Parameters
 *   chain     : pointer to active chain structure
 *   filename  : file to locate within the bsdl scan tree
 *   
 * Returns
 *   = 0   : file could not be located
 *  char*  : newly allocated memory containing the full file path, caller must free
 *
 ****************************************************************************/
static char* alt_bsdl_scan_for_filename (urj_chain_t *chain, const char *filename)
{
    urj_bsdl_globs_t *globs = &(chain->bsdl);
    int idx = 0;
    
    /* abort if no path list was specified */
    if (globs->path_list == NULL)
        return NULL;

    while (globs->path_list[idx] )
    {
        DIR *dir;

        if ((dir = opendir (globs->path_list[idx])))
        {
            struct dirent *elem;

            /* run through all elements in the current directory */
            while ((elem = readdir (dir)))
            {
                char *name;

                if (strcmp(filename, elem->d_name) == 0)
                {

                    /* if alloc fails, act like we didn't find the file */
                    name = malloc (strlen (globs->path_list[idx])
                                   + strlen (elem->d_name) + 1 + 1);
                    if (name)
                    {
                        strcpy (name, globs->path_list[idx]);
                        strcat (name, "/");
                        strcat (name, elem->d_name);

                        return name;

                    }
                }
            }

            closedir (dir);
        }
        else
	    urj_log (URJ_LOG_LEVEL_ERROR, _("Cannot open directory %s\n"), globs->path_list[idx]);

        idx++;
    }

    return NULL;
}

const urj_pld_driver_t urj_pld_alt_driver = {
    .name = N_("Altera Stratix/Cyclone/Arria"),
    .detect = alt_detect,
    .print_status = alt_print_status,
    .configure = alt_configure,
    .reconfigure = alt_reconfigure,
    .read_register = alt_read_register,
    .write_register = alt_write_register,
    .register_width = 2,
};

