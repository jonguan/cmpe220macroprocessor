/*
 * parser.c - Contains functions for parsing SIC assembly code.
 */

#include <stdlib.h>
#include <string.h>
#include "definitions.h"
#include "parser.h"

/**
 * Function: parse_info_alloc
 * Description:
 *  - Allocates memory for a parse_info_t struct.
 * Parameters:
 *  - none
 * Returns:
 *  - If successful, returns pointer to parse_info_t struct. Otherwise, returns
 *    NULL.
 */
parse_info_t * parse_info_alloc(void)
{
    //TODO stubbed
    return NULL;
}

/**
 * Function: parse_info_free
 * Description:
 *  - Frees the memory associated with the given parse_info_t struct.
 * Parameters:
 *  - parse_info: Pointer to a valid parse_info_t struct.
 * Returns:
 *  - none
 */
void parse_info_free(parse_info_t * parse_info)
{
    //TODO stubbed
}

/**
 * Function: parse_info_line
 * Description:
 *  - Parses the given line and fills in the specified parse_info_t struct.
 * Parameters:
 *  - parse_info: Pointer to a valid parse_info_t struct.
 *  - line: Line of SIC assembly code to be parsed.
 * Returns:
 *  - If successful, returns 0. Otherwise, returns -1.
 */
int parse_info_line(parse_info_t * parse_info, const char * line)
{
    //TODO stubbed
    return -1;
}
