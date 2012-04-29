/*************************************************************************
 * CMPE220 - Project: Macroprocessor for Assembly programs
 *
 * processLine.c:
 *
 * This is an implementation of the macroprocessor PROCESSLINE function
 * that expand macros based on their definition in DEFTAB and writes
 * the expanded assembly program to an outputfile.
 * 
 * Author: Jon Guan <jonguan@gmail.com>
 * Date: Apr 21 2012
 *
 *************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include "definitions.h"
#include "parser.h"

/**
* Function: processLine
* Description:
*  - If macroLine begins with MACRO, delegates work to function Define
*  - If macroLine begins with macro defined in NAMTAB, then delegates work to function EXPAND
*	 Otherwise, writes out line to outputFile
*
* Parameters:
* inputFile - File pointer from file already open for reading
* outputFile - File pointer from file already open for writing
* macroLine from getline
*
* Returns:
* SUCCESS (0) or FAILURE (-1)
*/
int processLine(FILE * inputFile, FILE* outputFile, const char *macroLine)
{
	int result = FAILURE;
	parse_info_t *parseInfo = parse_info_alloc();

	// Get OPCODE (strtok)
	if(parse_line(parseInfo, macroLine) == FAILURE)
	{
		printf("Error in parse_line.\n");
        parse_info_free(parseInfo);
		return FAILURE;
	}
    else
	{
        //set OPCODE
        if(parseInfo->opcode)
        {
		    strcpy_s(OPCODE, sizeof(OPCODE), parseInfo->opcode);
        }
        else
        {
            sprintf_s(OPCODE, sizeof(OPCODE), "");
        }
	}
	
	/* Search NAMTAB for OPCODE*/

	if (namtab_get(namtab, parseInfo->opcode) != NULL)
	{
		//Call expand
		result = expand(inputFile, outputFile, parseInfo->opcode);
	}
	else if (parseInfo->opcode != NULL && strncmp("MACRO", parseInfo->opcode, strlen("MACRO")) == 0)
	{
		//Call define
		result = define(inputFile, outputFile, macroLine);
	}
	else
	{
		//write source line to expanded file
		////////////////////////////////////////////


		// Error check
		if (outputFile == NULL) {
			fprintf(stderr, "Output file passed to processLine is null!\n");
            parse_info_free(parseInfo);
			return FAILURE;
		}

		// write line out
		fprintf(outputFile, currentLine);

		result = SUCCESS;
	}

	// Memory cleanup
	parse_info_free(parseInfo);

	return result;
}

