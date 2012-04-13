/*
 * deftab.h - Contains functions and definitions for the DEFTAB data structure.
 */

#ifndef DEFTAB_H_
#define DEFTAB_H_

typedef struct
{
    int     size;
    int     capacity;
    char **	array;
} deftab_t;

deftab_t *  deftab_alloc(void);
void        deftab_free(deftab_t *);
int         deftab_add(deftab_t * table, const char * data);
char *      deftab_get(deftab_t * table, int index);

#endif /* DEFTAB_H_ */
