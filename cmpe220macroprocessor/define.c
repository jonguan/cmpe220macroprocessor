/*************************************************************************
 * CMPE220 - Project: Macroprocessor for Assembly programs
 *
 * define.c:
 *
 * This is an implementation of the macroprocessor DEFINE function
 * that Handles the definitions of macros. It enters information into
 * NAMTAB and DEFTAB. Substitutes positional notation for parameters.
 * Also, handles recursive MACRO declarations..
 * 
 * Author: Mujtaba Hassanpur <mhassanpur@gmail.com>
 * Date: Apr 14 2012
 *
 *************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#include "definitions.h"
#include "parser.h"

/**
* Function: define
* Description:
*  - Handles the definitions of macros. Enters information into NAMTAB and
*    DEFTAB. Substitutes positional notation for parameters. Also, handles
*    recursive MACRO declarations.
* Parameters:
*  - inputFile: File pointer to the already open input file.
*  - outputFile: File pointer to the already open outputfile.
*  - macroLine: The line of code that contains the MACRO directive (macro
*    declaration).
* Returns:
*  - If successful, returns SUCCESS. Otherwise, returns FAILURE.
*/
int define(FILE * inputFile, FILE * outputFile, const char * macroLine)
{
	parse_info_t * parse_info = NULL;
	namtab_entry_t * namtab_entry = NULL;
	int index = 0;
	int level = 1;
	int i = 0;
	const char argDelim[] = ", ";
	char * params = NULL;
	char * token = NULL;
	char * nextToken = NULL;
	char * tmpString;
    char * currLine;

	if(argtab == NULL || deftab == NULL || namtab == NULL)
	{
		// data structures not initialized
		printf("ERROR - %s: Data structures not initialized!\n", __func__);
		return FAILURE;
	}
	else if(inputFile == NULL || outputFile == NULL)
	{
		// bad file pointers
		printf("ERROR - %s: Bad file pointer!\n", __func__);
		return FAILURE;
	}
	else if(macroLine == NULL)
	{
		// null string for macro line
		printf("ERROR - %s: Null string for macro line!\n", __func__);
		return FAILURE;
	}

	// parse the macro line
	parse_info = parse_info_alloc();
	if(parse_line(parse_info, macroLine) != 0)
	{
		// something went wrong
		parse_info_free(parse_info);
		return FAILURE;
	}

	// make sure we're dealing with a macro definition line
	if(parse_info->opcode == NULL || parse_info->label == NULL || strncmp("MACRO", parse_info->opcode, strlen("MACRO")) != 0)
	{
		printf("ERROR: Invalid macro definition:\n%s\n\n", macroLine);
		parse_info_free(parse_info);
		return FAILURE;
	}

	// enter the macro name into NAMTAB
	index = namtab_add(namtab, parse_info->label, 0, 0); // use 0 indices for now
	namtab_entry = namtab_getIndex(namtab, index);

	// enter macro prototype into DEFTAB
	namtab_entry->deftabStart = deftab_add(deftab, macroLine);

	while(level > 0)
	{
		//  GET Next LINE
		currLine = getline(inputFile);
		parse_info_clear(parse_info);
		if(parse_line(parse_info, currentLine) != SUCCESS)
		{
			parse_info_free(parse_info);
			return FAILURE;
		}

		if(parse_info->isComment == FALSE)
		{
			//  Check for macro expansion variable
			if (strcmp(parse_info->opcode, "SET"))
			{
				//Check if macro varialbe is in valid format
				if(parse_info->label != NULL &&
					*(parse_info->label) == '&' && 
					!isspace(*((parse_info->label) + 1)) )
				{
					//Add to argTab
                    argtab_add(argtab, parse_info->label, parse_info->operators);
				}
			}

			// Substitute positional notation for parameters
			tmpString = _strdup(currentLine);
			index = deftab_add(deftab, tmpString);
			free(tmpString);
			if(parse_info->opcode != NULL && strncmp("MACRO", parse_info->opcode, strlen("MACRO")) == 0)
			{
				level++;
			}
			else if(parse_info->opcode != NULL && strncmp("MEND", parse_info->opcode, strlen("MEND")) == 0)
			{
				level--;
			}
		}
	}

	// store in NAMTAB pointers to beginning and end of definition
	namtab_entry->deftabEnd = index;

	// free allocated memory
	parse_info_free(parse_info);
	return SUCCESS;
}
