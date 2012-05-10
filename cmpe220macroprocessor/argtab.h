/*
 * argtab.h - Contains functions and definitions for the ARGTAB data structure.s
 */

#ifndef ARGTAB_H_
#define ARGTAB_H_

#include "uthash\uthash.h"
#include <stddef.h>

#define ARGTAB_STRING_SIZE (64)
#define bool unsigned char
#define true 1
#define false 0

// Value is an array if it starts with ( and ends with )
// NOTE: Parens are removed during insertion and bool is set
struct argtab_data
{
    char            key[ARGTAB_STRING_SIZE];
    char            value[ARGTAB_STRING_SIZE];
	bool			valIsArray;
    UT_hash_handle  hh;
};

// The argtab_t structure contains an array of string pointers.
typedef struct
{
    int size;
    struct argtab_data *    data;
}argtab_t;

argtab_t *	argtab_alloc(void);
void        argtab_free(argtab_t * table);
int         argtab_add(argtab_t * table, const char * symbol, const char * value);
char *      argtab_get(argtab_t * table, const char * symbol);
int         argtab_set(argtab_t * table, const char * symbol, const char * value);
int         argtab_addOrSet(argtab_t * table, const char * symbol, const char * value);
void        argtab_clear(argtab_t * table);
void        argtab_substituteValues(argtab_t * table, char * buffer, size_t bufsize);


#endif /* ARGTAB_H_ */
