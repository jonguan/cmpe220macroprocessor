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
	char opCode[8];
	int result = FAILURE;
	parse_info_t *parseInfo = parse_info_alloc();

	// Get OPCODE (strtok)
	if(parse_line(parseInfo, macroLine) == FAILURE)
	{
		printf("Error in parse_line.\n");
		return FAILURE;
	}else
	{
		//set OPCODE
		OPCODE = parseInfo->opcode;
	}
	
	/* Search NAMTAB for OPCODE*/

	if (opCode != NULL)
	{
		//Call expand
		result = expand(inputFile, outputFile, opCode);
	}
	else if (strcmp(opCode, "MACRO"))
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
			return FAILURE;
		}

		// write line out
		fprintf(outputFile, "inputLine");

	}

	// Memory cleanup
	parse_info_free(parseInfo);

	return result;
}

