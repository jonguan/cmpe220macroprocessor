/*
 * argtab.h - Contains functions and definitions for the ARGTAB data structure.s
 */

#ifndef ARGTAB_H_
#define ARGTAB_H_

// this is the maximum number of arguments we can handle in macros
#define ARGTAB_MAX_ARRAY_SIZE	(32)

// The argtab_t structure contains an array of string pointers.
typedef struct
{
    char **	array;
} argtab_t;

argtab_t *	argtab_alloc(void);
void        argtab_free(argtab_t * table);
int         argtab_add(argtab_t * table, int argnum, const char * symbol);
char *      argtab_get(argtab_t * table, int argnum);
void        argtab_clear(argtab_t * table);

#endif /* ARGTAB_H_ */
