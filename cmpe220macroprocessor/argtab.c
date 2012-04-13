/*
 * argtab.c - Contains functions for the ARGTAB data structure.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "definitions.h"
#include "argtab.h"

/**
 * Function: argtab_alloc
 * Description:
 *  - Allocates memory for an ARGTAB data structure.
 * Parameters:
 *  - none
 * Returns:
 *  - If successful, returns pointer to new ARGTAB structure. Otherwise,
 *    returns NULL.
 */
argtab_t * argtab_alloc(void)
{
    argtab_t * table = (argtab_t *) malloc(sizeof(argtab_t));
    int arrayBufSize = ARGTAB_MAX_ARRAY_SIZE * sizeof(char *);
    if(table != NULL)
    {
        // initialize to zero
        memset(table, 0, sizeof(table));
        table->array = (char **) malloc(arrayBufSize);
        if(table->array)
        {
            memset(table->array, 0, arrayBufSize);
        }
        else
        {
            // oops, no memory for the array, so bail
            free(table);
            table = NULL;
        }
    }

    //printf("%s: New table @ 0x%08x\n", __func__, table);
    return table;
}

/**
 * Function: argtab_free
 * Description:
 *  - Frees the memory associated with the given ARGTAB data structure.
 * Parameters:
 *  - table: Pointer to ARGTAB data structure.
 * Returns:
 *  - none
 */
void argtab_free(argtab_t * table)
{
    if(table)
    {
        if(table->array)
        {
            argtab_clear(table); // remove all elements
            //printf("%s: Free array @ 0x%08x\n", __func__, table->array);
            free(table->array);
        }
        //printf("%s: Free table @ 0x%08x\n", __func__, table);
        free(table);
    }
}

/**
 * Function: argtab_add
 * Description:
 *  - Adds the given symbol to the ARGTAB, as the specified argument number.
 * Parameters:
 *  - table: Pointer to ARGTAB.
 *  - argnum: Argument number to add (1-based).
 *  - symbol: Symbol to add.
 * Returns:
 *  - If successful, returns array index (0-based) of the new entry (NOTE: not
 *    argument number!). Otherwise, returns -1.
 */
int argtab_add(argtab_t * table, int argnum, const char * symbol)
{
    int		result = -1;
    int		bufsize;
    char *	tmpData;

    if( (table != NULL) && (table->array != NULL) &&
        (argnum > 0) && (argnum <= ARGTAB_MAX_ARRAY_SIZE) &&
        (symbol != NULL) )
    {
        // argnum is 1-based, while array index is 0-based
        result = argnum - 1;

        // allocate memory for string
        bufsize = strlen(symbol) + 1;
        tmpData = (char *) malloc(bufsize);

        // copy string to new location
        strcpy_s(tmpData, bufsize, symbol);

        // add new string to array
        // if overwriting, make sure to free the existing string
        if(table->array[result] != NULL)
        {
            free(table->array[result]);
        }
        table->array[result] = tmpData;

        //printf("%s: Added item %d @ 0x%08x = '%s'\n", __func__, result, table->array[result], table->array[result]);
    }

    return result;
}

/**
 * Function: argtab_get
 * Description:
 *  - Retrieves the symbol associated with the given argument number.
 * Parameters:
 *  - table: Pointer to ARGTAB.
 *  - argnum: Argument number in ARGTAB (1-based).
 * Returns:
 *  - Pointer to string representing the symbol.
 */
char * argtab_get(argtab_t * table, int argnum)
{
    char *	result = NULL;
    int		index = argnum -1;

    if( (table != NULL) && (argnum > 0) && (argnum <= ARGTAB_MAX_ARRAY_SIZE) )
    {

        result = table->array[index];
    }

    return result;
}

/**
 * Function: argtab_clear
 * Description:
 *  - Clears the data in the argument table. De-allocates memory associated
 *    with the data in the argument table, but not the argument table itself.
 * Parameters:
 *  - table: Pointr to ARGTAB.
 * Returns:
 *  - none
 */
void argtab_clear(argtab_t * table)
{
    int i;

    if(table && table->array)
    {
        for(i = 0; i < ARGTAB_MAX_ARRAY_SIZE; i++)
        {
            if(table->array[i])
            {
                //printf("%s: Free item %d @ 0x%08x\n", __func__, i, table->array[i]);
                free(table->array[i]);
                table->array[i] = NULL;
            }
        }
    }
}