/*
 * deftab.c
 *
 *  Created on: Mar 23, 2012
 *      Author: mujtaba
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "common.h"
#include "deftab.h"

deftab_t * deftab_alloc(void)
{
	char ** array;
	deftab_t * table = (deftab_t *) malloc(sizeof(deftab_t));
	if(table)
	{
		// initialize to zero
		memset(table, 0, sizeof(table));

		// allocate memory for string array (start with capacity of 1)
		array = (char **) malloc(sizeof(char *));
		if(array)
		{
			table->size = 0;
			table->capacity = 1;
			table->array = array;
		}
	}

	//printf("%s: New table @ 0x%08x\n", __func__, table);
	return table;
}

void deftab_free(deftab_t * table)
{
	int i;

	if(table)
	{
		if(table->array)
		{
			for(i = 0; i < table->size; i++)
			{
				//printf("%s: Free item %d @ 0x%08x\n", __func__, i, table->array[i]);
				free(table->array[i]);
			}

			//printf("%s: Free array @ 0x%08x\n", __func__, table->array);
			free(table->array);
		}

		//printf("%s: Free table @ 0x%08x\n", __func__, table);
		free(table);
	}
}

int deftab_add(deftab_t * table, const char * data)
{
	int		result = -1;
	int		bufsize;
	char **	tmpArray;
	char *	tmpData;

	if(table && table->array && data)
	{
		// check if array is full, if so, then grow capacity
		if(table->size >= table->capacity)
		{
			// allocate a new array with twice the capacity as this one
			tmpArray = (char **) malloc(2 * table->capacity * sizeof(char *));

			// copy contents to new array
			memcpy(tmpArray, table->array, table->capacity * sizeof(char *));

			// free the old array
			free(table->array);

			// point to new array
			table->array = tmpArray;

			// capacity has doubled
			table->capacity *= 2;

			//printf("%s: Increased array capacity to %d\n", __func__, table->capacity);
		}

		// allocate memory for string
		bufsize = strlen(data) + 1;
		tmpData = (char *) malloc(bufsize);

		// copy string to new location
		strcpy_s(tmpData, bufsize, data);

		// add new string to array
		result = table->size++;
		table->array[result] = tmpData;

		//printf("%s: Added item %d @ 0x%08x = '%s'\n", __func__, result, table->array[result], table->array[result]);
	}

	return result;
}

char * deftab_get(deftab_t * table, int index)
{
	char * result = NULL;

	if(table && index >= 0)
	{
		result = table->array[index];
	}

	return result;
}
