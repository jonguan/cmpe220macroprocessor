/*
 * namtab.h
 *
 *  Created on: Mar 24, 2012
 *      Author: mujtaba
 */

#ifndef NAMTAB_H_
#define NAMTAB_H_

typedef struct
{
	char *	symbol;
	int		deftabStart;
	int		deftabEnd;
} namtab_entry_t;

typedef struct
{
	int					size;
	int					capacity;
	namtab_entry_t **	array;
} namtab_t;

namtab_t *			namtab_alloc(void);
void				namtab_free(namtab_t * table);
int					namtab_add(namtab_t * table, const char * symbol, int start, int end);
namtab_entry_t *	namtab_get(namtab_t * table, const char * symbol);

#endif /* NAMTAB_H_ */
