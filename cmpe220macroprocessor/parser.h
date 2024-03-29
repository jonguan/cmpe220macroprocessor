/*
 * parser.h - Contains definitions for parsing functions.
 */

#ifndef PARSER_H_
#define PARSER_H_

#include "definitions.h"

typedef struct
{
   BOOL     isComment;
   BOOL     hasKeywordMacroParameters;
   char *   label;
   char *   opcode;
   char *   operators;
} parse_info_t;

parse_info_t *  parse_info_alloc(void);
void            parse_info_free(parse_info_t * parse_info);
void            parse_info_clear(parse_info_t * parse_info);
void            parse_info_print(parse_info_t * parse_info);
int             parse_line(parse_info_t * parse_info, const char * line);
int				parse_reconstruct_string(parse_info_t * parse_info, char *returnString);
#endif // PARSER_H_
