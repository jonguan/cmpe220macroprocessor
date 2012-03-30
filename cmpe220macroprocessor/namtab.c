/*
 * namtab.c
 *
 *  Created on: Mar 24, 2012
 *      Author: mujtaba
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "common.h"
#include "namtab.h"

namtab_t * namtab_alloc(void)
{
	namtab_entry_t ** array;
	namtab_t * table = (namtab_t *) malloc(sizeof(namtab_t));
	if(table)
	{
		// initialize to zero
		memset(table, 0, sizeof(table));

		// allocate memory for array (start with capacity of 1)
		array = (namtab_entry_t **) malloc(sizeof(namtab_entry_t *));
		if(array)
		{
			table->size = 0;
			table->capacity = 1;
			table->array = array;
		}
	}

	printf("%s: New table @ 0x%08x\n", __func__, table);
	return table;
}

void namtab_free(namtab_t * table)
{
	int i;

	if(table)
	{
		if(table->array)
		{
			for(i = 0; i < table->size; i++)
			{
				printf("%s: Free item %d @ 0x%08x\n", __func__, i, table->array[i]);
				free(table->array[i]);
			}

			printf("%s: Free array @ 0x%08x\n", __func__, table->array);
			free(table->array);
		}

		printf("%s: Free table @ 0x%08x\n", __func__, table);
		free(table);
	}
}

int namtab_add(namtab_t * table, const char * symbol, int start, int end)
{
	int					result = -1;
	int					bufsize;
	namtab_entry_t **	tmpArray;
	namtab_entry_t *	tmpData;

	if(table && table->array && symbol)
	{
		// check if array is full, if so, then grow capacity
		if(table->size >= table->capacity)
		{
			// allocate a new array with twice the capacity as this one
			tmpArray = (namtab_entry_t **) malloc(2 * table->capacity * sizeof(namtab_entry_t *));

			// copy contents to new array
			memcpy(tmpArray, table->array, table->capacity * sizeof(namtab_entry_t *));

			// free the old array
			free(table->array);

			// point to new array
			table->array = tmpArray;

			// capacity has doubled
			table->capacity *= 2;

			printf("%s: Increased array capacity to %d\n", __func__, table->capacity);
		}

		// allocate memory for data
		tmpData = (namtab_entry_t *) malloc(sizeof(namtab_entry_t));
		if(tmpData)
		{
			bufsize = strlen(symbol) + 1;
			tmpData->symbol = (char *) malloc(bufsize);
			if(tmpData->symbol)
			{
				// fill in data
				strcpy_s(tmpData->symbol, bufsize, symbol);
				tmpData->deftabStart = start;
				tmpData->deftabEnd = end;

				// add new string to array
				result = table->size++;
				table->array[result] = tmpData;

				// debug
				printf("%s: Added item %d @ 0x%08x, (%d, %d) = '%s'\n",
						__func__, result,
						table->array[result],
						table->array[result]->deftabStart,
						table->array[result]->deftabEnd,
						table->array[result]->symbol);
			}
		}
	}

	return result;
}

namtab_entry_t * namtab_get(namtab_t * table, const char * symbol)
{
	namtab_entry_t * result = NULL;
	int i = 0;

	if(table && symbol)
	{
		for(i = 0; i < table->size; i++)
		{
			if(strcmp(table->array[i]->symbol, symbol) == 0)
			{
				result = table->array[i];
				break;
			}
		}
	}

	return result;
}