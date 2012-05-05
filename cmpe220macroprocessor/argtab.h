/*
 * argtab.h - Contains functions and definitions for the ARGTAB data structure.s
 */

#ifndef ARGTAB_H_
#define ARGTAB_H_

#include "uthash\uthash.h"

#define ARGTAB_STRING_SIZE (64)

struct argtab_data
{
    int             id;
    char            key[ARGTAB_STRING_SIZE];
    char            value[ARGTAB_STRING_SIZE];
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
int         argtab_add(argtab_t * table, int id, const char * symbol, const char * value);
char *      argtab_get(argtab_t * table, const char * symbol);
void        argtab_clear(argtab_t * table);
void        argtab_substituteValues(argtab_t * table, char * buffer, size_t bufsize);

#endif /* ARGTAB_H_ */
