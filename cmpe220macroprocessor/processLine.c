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
#include "common.h"
#include "definitions.h"
#include "argtab.h"
#include "deftab.h"
#include "namtab.h"
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
int processLine(FILE * inputFile, FILE* outputFile, const char *macroLine);
{
	char opCode[8];
	int result = FAILURE;
	FILE *outputFile;
	errno_t rc;

	// Error Check
	if(strlen(inputLine) < kOpFlagSymStart)
	{
		return result;
	}

	// Get OPCODE (strtok)
	strncpy_s(opCode, (kOpFlagSymStart - kOpCodeStart), inputLine + kOpCodeStart, _TRUNCATE);

	/* Search NAMTAB for OPCODE*/

	if (opCode != NULL)
	{
		//TODO call expand
		//result = expand(inputFileDes, outputFileDes, opCode);
	}
	else if (strcmp(opCode, "MACRO"))
	{
		//TODO call define
		//result = define(inputFileName);
	}
	else
	{
		//write source line to expanded file
		////////////////////////////////////////////

		// Open output file
		//Output is the last operand (argc-1)
		rc = fopen_s(&outputFile, outputFileName, "w");

		// Error check
		if (outputFile == NULL) {
			fprintf(stderr, "Can't open output file in.list!\n");
			return FAILURE;
		}

		// write line out
		fprintf(outputFile, "inputLine");

		// close file
		fclose(outputFile);
	}

	return result;
}

