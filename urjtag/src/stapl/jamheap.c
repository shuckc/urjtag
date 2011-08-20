/****************************************************************************/
/*                                                                          */
/*  Module:         jamheap.c                                               */
/*                                                                          */
/*                  Copyright (C) Altera Corporation 1997                   */
/*                                                                          */
/*  Description:    Heap management functions.  The heap is implemented as  */
/*                  a linked list of blocks of variable size.               */
/*                                                                          */
/*  Revisions:      1.1 added support for dynamic memory allocation         */
/*                                                                          */
/****************************************************************************/

#include <stdint.h>
#include "jamexprt.h"
#include "jamdefs.h"
#include "jamsym.h"
#include "jamstack.h"
#include "jamheap.h"
#include "jamjtag.h"
#include "jamutil.h"

/****************************************************************************/
/*                                                                          */
/*  Global variables                                                        */
/*                                                                          */
/****************************************************************************/

JAMS_HEAP_RECORD *urj_jam_heap = NULL;

void *urj_jam_heap_top = NULL;

int32_t urj_jam_heap_records = 0L;

/****************************************************************************/
/*                                                                          */

JAM_RETURN_TYPE
urj_jam_init_heap (void)
/*                                                                          */
/*  Description:    Initializes the heap area.  This is where all array     */
/*                  data is stored.                                         */
/*                                                                          */
/*  Returns:        JAMC_SUCCESS for success, or JAMC_OUT_OF_MEMORY if no   */
/*                  memory was available for the heap.                      */
/*                                                                          */
/****************************************************************************/
{
    JAM_RETURN_TYPE status = JAMC_SUCCESS;
    void **symbol_table = NULL;
    JAMS_STACK_RECORD *stack = NULL;
    int32_t *jtag_buffer = NULL;

    urj_jam_heap_records = 0L;

    if (urj_jam_workspace != NULL)
    {
        symbol_table = (void **) urj_jam_workspace;
        stack = (JAMS_STACK_RECORD *) & symbol_table[JAMC_MAX_SYMBOL_COUNT];
        jtag_buffer = (int32_t *) & stack[JAMC_MAX_NESTING_DEPTH];
        urj_jam_heap = (JAMS_HEAP_RECORD *)
            (((char *) jtag_buffer) + JAMC_JTAG_BUFFER_SIZE);
        urj_jam_heap_top = (void *) urj_jam_heap;

        /*
         *      Check that there is some memory available for the heap
         */
        if (((int32_t) urj_jam_heap) > (((int32_t) urj_jam_workspace_size) +
                                    ((int32_t) urj_jam_workspace)))
        {
            status = JAMC_OUT_OF_MEMORY;
        }
    }
    else
    {
        /* initialize heap to empty list */
        urj_jam_heap = NULL;
    }

    return status;
}

void
urj_jam_free_heap (void)
{
    int record = 0;
    JAMS_HEAP_RECORD *heap_ptr = NULL;
    JAMS_HEAP_RECORD *tmp_heap_ptr = NULL;

    if ((urj_jam_heap != NULL) && (urj_jam_workspace == NULL))
    {
        heap_ptr = urj_jam_heap;
        for (record = 0; record < urj_jam_heap_records; ++record)
        {
            if (heap_ptr != NULL)
            {
                tmp_heap_ptr = heap_ptr;
                heap_ptr = heap_ptr->next;
                free (tmp_heap_ptr);
            }
        }
    }
}

/****************************************************************************/
/*                                                                          */

JAM_RETURN_TYPE urj_jam_add_heap_record
    (JAMS_SYMBOL_RECORD *symbol_record,
     JAMS_HEAP_RECORD **heap_record, int32_t dimension)
/*                                                                          */
/*  Description:    Adds a heap record of the specified size to the heap.   */
/*                                                                          */
/*  Returns:        JAMC_SUCCESS for success, or JAMC_OUT_OF_MEMORY if not  */
/*                  enough memory was available.                            */
/*                                                                          */
/****************************************************************************/
{
    int count = 0;
    int element = 0;
    int32_t space_needed = 0L;
    BOOL cached = false;
    JAMS_HEAP_RECORD *heap_ptr = NULL;
    JAM_RETURN_TYPE status = JAMC_SUCCESS;

    /*
     *      Compute space needed for array or cache buffer.  Initialized arrays
     *      will not be cached if their size is less than the cache buffer size.
     */
    switch (symbol_record->type)
    {
    case JAM_INTEGER_ARRAY_WRITABLE:
        space_needed = dimension * sizeof (int32_t);
        break;

    case JAM_BOOLEAN_ARRAY_WRITABLE:
        space_needed = ((dimension >> 5) + ((dimension & 0x1f) ? 1 : 0)) *
            sizeof (int32_t);
        break;

    case JAM_INTEGER_ARRAY_INITIALIZED:
        space_needed = dimension * sizeof (int32_t);
/*      if (space_needed > JAMC_ARRAY_CACHE_SIZE)   */
/*      {                                           */
/*          space_needed = JAMC_ARRAY_CACHE_SIZE;   */
/*          cached = true;                          */
/*      }                                           */
        break;

    case JAM_BOOLEAN_ARRAY_INITIALIZED:
        space_needed = ((dimension >> 5) + ((dimension & 0x1f) ? 1 : 0)) *
            sizeof (int32_t);
/*      if (space_needed > JAMC_ARRAY_CACHE_SIZE)   */
/*      {                                           */
/*          space_needed = JAMC_ARRAY_CACHE_SIZE;   */
/*          cached = true;                          */
/*      }                                           */
        break;

    case JAM_PROCEDURE_BLOCK:
        space_needed = ((dimension >> 2) + 1) * sizeof (int32_t);
        break;

    default:
        status = JAMC_INTERNAL_ERROR;
        break;
    }

    /*
     *      Check if there is enough space
     */
    if (status == JAMC_SUCCESS)
    {
        if (urj_jam_workspace != NULL)
        {
            heap_ptr = (JAMS_HEAP_RECORD *) urj_jam_heap_top;

            urj_jam_heap_top = (void *) ((int32_t) heap_ptr +
                                     (int32_t) sizeof (JAMS_HEAP_RECORD) +
                                     space_needed);

            if ((int32_t) urj_jam_heap_top > (int32_t) urj_jam_symbol_bottom)
            {
                status = JAMC_OUT_OF_MEMORY;
            }
        }
        else
        {
            heap_ptr = (JAMS_HEAP_RECORD *) malloc ((unsigned int)
                                                        (sizeof
                                                         (JAMS_HEAP_RECORD) +
                                                         space_needed));

            if (heap_ptr == NULL)
            {
                status = JAMC_OUT_OF_MEMORY;
            }
            else if (urj_jam_heap == NULL)
            {
                urj_jam_heap = heap_ptr;
            }
        }
    }

    /*
     *      Add the new record to the heap
     */
    if (status == JAMC_SUCCESS)
    {
        heap_ptr->symbol_record = symbol_record;
        heap_ptr->dimension = dimension;
        heap_ptr->cached = cached;
        heap_ptr->position = 0L;

        if (urj_jam_workspace != NULL)
        {
            /* point next pointer to position of next block */
            heap_ptr->next = (JAMS_HEAP_RECORD *) urj_jam_heap_top;
        }
        else
        {
            /* add new heap block to beginning of list */
            heap_ptr->next = urj_jam_heap;
            urj_jam_heap = heap_ptr;
        }

        /* initialize data area to zero */
        count = (int) (space_needed / sizeof (int32_t));
        for (element = 0; element < count; ++element)
        {
            heap_ptr->data[element] = 0L;
        }

        ++urj_jam_heap_records;

        *heap_record = heap_ptr;
    }

    return status;
}

/****************************************************************************/
/*                                                                          */

void *
urj_jam_get_temp_workspace (int32_t size)
/*                                                                          */
/*  Description:    Gets a pointer to the unused area of the heap for       */
/*                  temporary use.  This area will be used for heap records */
/*                  if urj_jam_add_heap_record() is called.                     */
/*                                                                          */
/*  Returns:        pointer to memory, or NULL if memory not available      */
/*                                                                          */
/****************************************************************************/
{
    void *temp_workspace = NULL;

    if (urj_jam_workspace != NULL)
    {
        if (((int32_t) urj_jam_heap_top) + size <= (int32_t) urj_jam_symbol_bottom)
        {
            temp_workspace = urj_jam_heap_top;
        }
    }
    else
    {
        temp_workspace = malloc ((unsigned int) size);
    }

    return temp_workspace;
}

/****************************************************************************/
/*                                                                          */

void
urj_jam_free_temp_workspace (void *ptr)
/*                                                                          */
/*  Description:    Frees memory buffer allocated by urj_jam_get_temp_workspace */
/*                                                                          */
/*  Returns:        Nothing                                                 */
/*                                                                          */
/****************************************************************************/
{
    if (urj_jam_workspace == NULL)
        free (ptr);
}
