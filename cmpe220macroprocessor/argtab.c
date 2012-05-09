/*
 * argtab.c - Contains functions for the ARGTAB data structure.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "definitions.h"
#include "argtab.h"
#include "uthash\uthash.h"

// Private functions
int			nameLengthSort(struct argtab_data *a, struct argtab_data *b);

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
    if(table != NULL)
    {
        // initialize values
        table->size = 0;
        table->data = NULL;
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
        argtab_clear(table);
        //printf("%s: Free table @ 0x%08x\n", __func__, table);
        free(table);
    }
}

/**
 * Function: argtab_add
 * Description:
 *  - Adds the given symbol to the ARGTAB, with the specified value.
 * Parameters:
 *  - table: Pointer to ARGTAB.
 *  - symbol: Symbol to add.
 *  - value: Value associated with the symbol.
 * Returns:
 *  - If successful, returns SUCCESS, otherwise returns FAILURE.
 */
int argtab_add(argtab_t * table, const char * symbol, const char * value)
{
    int	result = FAILURE;
    struct argtab_data * element = NULL;
    struct argtab_data ** ht = NULL;

    if(table != NULL && symbol != NULL && value != NULL)
    {
        ht = &(table->data);
        HASH_FIND_STR(*ht, symbol, element);
        if(element)
        {
            // hash key already exists
            return FAILURE;
        }

        // hash key is unique
        element = (struct argtab_data *) malloc(sizeof(struct argtab_data));
        if(element != NULL)
        {
            strcpy_s(element->key, ARGTAB_STRING_SIZE, symbol);
            strcpy_s(element->value, ARGTAB_STRING_SIZE, value);
            HASH_ADD_STR(*ht, key, element);
            table->size++;
            result = SUCCESS;
        }
    }

    return result;
}

/**
 * Function: argtab_set
 * Description:
 *  - Sets the value of the given symbol already in ARGTAB.
 * Parameters:
 *  - table: Pointer to ARGTAB.
 *  - symbol: Symbol to modify.
 *  - value: Value associated with the symbol.
 * Returns:
 *  - If successful, returns SUCCESS, otherwise returns FAILURE.
 */
int argtab_set(argtab_t * table, const char * symbol, const char * value)
{
    int	result = FAILURE;
    struct argtab_data * element = NULL;
    struct argtab_data ** ht = NULL;

    if(table != NULL && symbol != NULL && value != NULL)
    {
        ht = &(table->data);
        HASH_FIND_STR(*ht, symbol, element);
        if(element)
        {
            // hash key found
            strcpy_s(element->value, ARGTAB_STRING_SIZE, value);
            result = SUCCESS;
        }
    }

    return result;
}

/**
 * Function: argtab_addOrSet
 * Description:
 *  - Adds the entry to ARGTAB, if it doesn't exist. Otherwise, replaces the value.
 * Parameters:
 *  - table: Pointer to ARGTAB.
 *  - symbol: Symbol string (key).
 *  - value: Value associated with the symbol.
 * Returns:
 *  - If successful, returns SUCCESS. Otherwise, returns FAILURE.
 */
int argtab_addOrSet(argtab_t * table, const char * symbol, const char * value)
{
    int result = argtab_add(table, symbol, value);
    if(result != SUCCESS)
    {
        result = argtab_set(table, symbol, value);
    }

    return result;
}

/**
 * Function: argtab_get
 * Description:
 *  - Retrieves the value associated with the given symbol.
 * Parameters:
 *  - table: Pointer to ARGTAB.
 *  - symbol: Symbol to look up.
 * Returns:
 *  - Pointer to string representing the value associated with the symbol.
 */
char * argtab_get(argtab_t * table, const char * symbol)
{
    char * result = NULL;
    struct argtab_data ** ht = NULL;
    struct argtab_data * found = NULL;

    if(table && symbol)
    {
        ht = &(table->data);
        HASH_FIND_STR(*ht, symbol, found);
        if(found)
        {
            result = found->value;
        }
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
    struct argtab_data **ht, *i, *tmp;
    if(table)
    {
        ht = &(table->data);
        if(ht)
        {
            HASH_ITER(hh, *ht, i, tmp)
            {
                HASH_DEL(*ht, i);
                free(i);
            }
        }
    }
}

/**
 * Function: argtab_substituteValues
 * Description:
 *  - Given an ARGTAB and a string buffer, replaces all keys in the
 *    string with their corresponding values.
 * Parameters:
 *  - table: Pointer to ARGTAB.
 *  - buffer: Pointer to string buffer.
 *  - bufsize: Size fo the string buffer, in bytes.
 * Returns:
 *  - none
 */
void argtab_substituteValues(argtab_t * table, char * buffer, size_t bufsize)
{
    struct argtab_data *i, *tmp;
    struct argtab_data **ht = NULL;
    if(table && buffer)
    {
        ht = &(table->data);
        if(ht)
        {
			//sort by length of the key descending order
			HASH_SORT(*ht, nameLengthSort);

            HASH_ITER(hh, *ht, i, tmp)
            {
                strReplace(buffer, bufsize, i->key, i->value);
            }
        }
    }
}


/**
 * Function: nameLengthSort
 * Description:
 *  - A comparision function for HASH_SORT
 *    The second argument is a pointer to a comparison function. 
 *
 * Parameters:
 *  - pointers to 2 nodes to compare
 * Returns:
 *  - < 0 ; b < a
 *  - 0; a == b
 *  - > 0; b > a
 */
int nameLengthSort(struct argtab_data *a, struct argtab_data *b)
{
	return strlen(b->key) - strlen(a->key);
}