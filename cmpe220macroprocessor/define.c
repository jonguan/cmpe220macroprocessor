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
	argtab_t * tmp_argtab = NULL;
	int argidx = 1;
	int index = 0;
	int level = 1;
	int i = 0;
	const char argDelim[] = ", ";
	char * params = NULL;
	char * token = NULL;
	char * nextToken = NULL;
	char * argReplace;
	char argWith[16];
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

	// create a temporary local argtab for keeping track of parameters
	tmp_argtab = argtab_alloc();
	if(tmp_argtab == NULL)
	{
		printf("ERROR: Couldn't allocate memory for a temporary local argtab!\n");
		parse_info_free(parse_info);
		return FAILURE;
	}
	/*
		Enter params into temp argtab
		NOTE: argidx starts at 1, token is in format of &token
	*/
	if(parse_info->operators)
	{
		params = _strdup(parse_info->operators);
		token = strtok_s(params, argDelim, &nextToken);
		while(token != NULL)
		{
			argtab_add(tmp_argtab, argidx++, token);
			token = strtok_s(NULL, argDelim, &nextToken);
		}
		free(params);
		params = NULL;
	}

	while(level > 0)
	{
		//  GET Next LINE
		currLine = getline(inputFile);
		parse_info_clear(parse_info);
		if(parse_line(parse_info, currentLine) != SUCCESS)
		{
			argtab_free(tmp_argtab);
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
					argtab_add(tmp_argtab, argidx++, parse_info->label);
				}
			}

			// Substitute positional notation for parameters
			tmpString = _strdup(currentLine);
			for(i = 1; i < argidx; i++)
			{
				argReplace = argtab_get(tmp_argtab, i);
				sprintf_s(argWith, sizeof(argWith), "?%d", i);
				strReplace(tmpString, strlen(tmpString) + 1, argReplace, argWith);
			}
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
	argtab_free(tmp_argtab);
	parse_info_free(parse_info);
	return SUCCESS;
}
