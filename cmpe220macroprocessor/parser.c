/*
 * parser.c - Contains functions for parsing SIC assembly code.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <ctype.h>
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
    parse_info_t * result = (parse_info_t *) malloc(sizeof(parse_info_t));
    if(result)
    {
        // initialize the structure
        memset(result, 0, sizeof(parse_info_t));
        parse_info_clear(result);
    }
    return result;
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
    if(parse_info == NULL)
    {
        return;
    }

    if(parse_info->label)
    {
        free(parse_info->label);
    }

    if(parse_info->opcode)
    {
        free(parse_info->opcode);
    }

    if(parse_info->operators)
    {
        free(parse_info->operators);
    }

    free(parse_info);
}

/**
 * Function: parse_info_clear
 * Description:
 *  - Clears the data in the given parse_info_t structure.
 * Parameters:
 *  - Pointer to valid parse_info_t struct.
 * Returns:
 *  - none
 */
void parse_info_clear(parse_info_t * parse_info)
{
    if(parse_info == NULL)
    {
        return;
    }

    parse_info->isComment = FALSE;
    parse_info->hasKeywordMacroParameters = FALSE;
    if(parse_info->label)
    {
        free(parse_info->label);
        parse_info->label = NULL;
    }
    if(parse_info->opcode)
    {
        free(parse_info->opcode);
        parse_info->opcode = NULL;
    }
    if(parse_info->operators)
    {
        free(parse_info->operators);
        parse_info->operators = NULL;
    }
}

/**
 * Function: parse_line
 * Description:
 *  - Parses the given line and fills in the specified parse_info_t struct.
 * Parameters:
 *  - parse_info: Pointer to a valid parse_info_t struct.
 *  - line: Line of SIC assembly code to be parsed.
 * Returns:
 *  - If successful, returns 0. Otherwise, returns -1.
 */
int parse_line(parse_info_t * parse_info, const char * line)
{
    char * tmp;
    char * token;
    char * nextToken;
    const char delimiters[] = " \t";    // spaces and/or tabs
    const char newlineDelimiter[] = "\n";

    if(parse_info == NULL || line == NULL)
    {
        // error
        return -1;
    }

    // clear the structure
    parse_info_clear(parse_info);

    // check if this line is a comment line (starts with a ".")
    if(strncmp(".", line, strlen(".")) == 0)
    {
        parse_info->isComment = TRUE;
        return 0;
    }

    // copy line to temp buffer because strtok modifies the string passed in
    tmp = _strdup(line);
    if(tmp == NULL)
    {
        // error - couldn't allocate memory for the new buffer, or other error
        return -1;
    }

    /*
     * NOTE: First token can be either a label or opcode!
     *  - When the token starts at the beginning of the line, it is a label.
     *    Otherwise, it is an opcode.
     */
    token = strtok_s(tmp, delimiters, &nextToken);
    if(strncmp(token, line, strlen(token)) == 0)
    {
        // The first token appears at the beginning of the line (on the first
        // column) so it is the label.
        parse_info->label = _strdup(token);

        // Now we get the second token, which should be the opcode
        token = strtok_s(NULL, delimiters, &nextToken);
        parse_info->opcode = _strdup(token);
    }
    else
    {
        // The first token is preceeded by whitespace, so consider it the opcode
        parse_info->opcode = _strdup(token);
    }

    // We want the rest of the line, so make the delimiter a newline and call
    // strtok again.
    token = strtok_s(NULL, newlineDelimiter, &nextToken);
    if(token)
    {
        // trim leading whitespace first
        while(isspace(*token))
        {
            token++;
        }
        parse_info->operators = _strdup(token);
    }

    // at this point we've done the parsing, check to see if keyword macro parameters are used
    if( parse_info->opcode != NULL &&
        strncmp("MACRO", parse_info->opcode, strlen("MACRO")) == 0 &&
        strstr(parse_info->operators, "=") != NULL )
    {
        parse_info->hasKeywordMacroParameters = TRUE;
    }

    free(tmp);
    return 0;
}

/**
 * Function: parse_info_print
 * Description:
 *  - Prints the contents of a parse_info_t structure.
 * Parameters:
 *  - Pointer to a parse_info_t structure.
 * Returns:
 *  - none
 */
void parse_info_print(parse_info_t * parse_info)
{
    printf("parse_info_t @ 0x%08x:\n", parse_info);
    if(parse_info)
    {
        printf("    isComment: ");
        if(parse_info->isComment)
        {
            printf("TRUE\n");
        }
        else
        {
            printf("FALSE\n");
        }

        printf("    label: ");
        if(parse_info->label)
        {
            printf("%s\n", parse_info->label);
        }
        else
        {
            printf("null\n");
        }

        printf("    opcode: ");
        if(parse_info->opcode)
        {
            printf("%s\n", parse_info->opcode);
        }
        else
        {
            printf("null\n");
        }

        printf("    operators: ");
        if(parse_info->operators)
        {
            printf("%s\n", parse_info->operators);
        }
        else
        {
            printf("null\n");
        }
    }
}