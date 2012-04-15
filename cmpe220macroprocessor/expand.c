/*************************************************************************
 * CMPE220 - Project: Macroprocessor for Assembly programs
 *
 * expand.c:
 *
 * This is an implementation of the macroprocessor EXPAND function
 * that expand macros based on their definition in DEFTAB and writes
 * the expanded assembly program to an outputfile.
 * 
 * Author: Rajesh Somasundaran <rsomasundaran@gmail.com>
 * Date: Apr 14 2012
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


/*
 * expand:
 * Main function to expand MACRO with its definition as found in DEFTAB
 *
 * Parameters:
 *  - inputFileDes - File descriptor for the input assembly program file
 *  - outputFileDes - File descriptor for the output (expanded) assembly program file
 *  - macroName - Name of MACRO to be expanded
 * Returns:
 * SUCCESS (0) or FAILURE (-1)
 */
int expand(FILE *inputFileDes, FILE *outputFileDes, const char *macroName)  
{
	char *line;
	int argCount;
	int endOfMacroDef;
	namtab_entry_t *nameEntry;
	
	EXPANDING = TRUE;
	
	printf("Expanding Macro: %s ...\n", macroName);
	
	/* Get first line of macro definition from DEFTAB */
	line = getline(inputFileDes);
	//printf("First line of definition for macro %s:\n", macroName);
	//printf("%s\n", line);

	/* Create ARGTAB with arguments from macro invocation */
	argCount = setUpArguments(line, macroName);
	if (argCount < 0) {
		return FAILURE;
	}
	//printf("Number of arguments for macro %s: %d\n", macroName, argCount);

	/* Write macro invocation line to the output file as a comment */
	if (commentOutMacroCall(line, outputFileDes) == FAILURE) {
		return FAILURE;
	}

	/* 
	 * Read from NAMTAB, the starting and ending index of macro definition in DEFTAB
	 * and process each line from DEFTAB
	 */
	nameEntry = namtab_get(namtab, macroName);
	if (nameEntry == NULL) {
		return FAILURE;
	}
	deftabIndex = nameEntry->deftabStart;
	endOfMacroDef = nameEntry->deftabEnd;
		
	while (deftabIndex < endOfMacroDef) {	// Assumes the MACRO definition ends with MEND in DEFTAB!
		line = getline(inputFileDes);
		processline(line, inputFileDes, outputFileDes);
		deftabIndex++;
	}

	EXPANDING = FALSE;

	return SUCCESS;
}


/*
 * setUpArguments:
 * Create ARGTAB with arguments from macro invocation.
 *
 * Parameters:
 *  - inputLine - input assembly program line to comment out
 *  - macroName - Name of MACRO being expanded
 * Returns:
 *  - >0, Argument count OR
 *  - 0, if inputLine is a comment or if no arguments found in macro invocation OR
 *  - -1, for all FAILURE cases
 */
int setUpArguments (char *line, const char *macroName)
{
	int argCount = 0;
	char *operand;
	parse_info_t *splitLine;
		
	
	argtab = argtab_alloc();	// Create empty ARGTAB

	/* 
	 * If ARGTAB creation succeeded, proceed to parse the input line
	 * into tokens - label, opcode, operands string.
	 */
	if ((argtab == NULL) || (parse_line(splitLine, line) < 0) {
		return FAILURE;
	}

	if (splitLine->isComment == TRUE) {		// Input line is a comment
		return 0;
	}

	/*
	 * Fill ARGTAB with arguments from macro invocation.
	 * Format of arguments in operands field: &op1,&op2,&op3,...
	 * ARGTAB indexing starts at 1.
	 */
	operand = strtok(splitLine->operators, ",&");
	while (operand != NULL){
		argCount++;
		if (argtab_add(argtab, argCount, operand) < 0) {
			argtab_free(argtab);
			return FAILURE;
		}
		operand = strtok(NULL, ",&");
	}
		
	return argCount;
}

/*
 * commentOutMacroCall:
 * Writes the macro invocation line to the output file as a comment.
 *
 * Parameters:
 *  - inputLine - input assembly program line to comment out
 *  - outputfd - File descriptor for the output file
 * Returns:
 * SUCCESS (0) or FAILURE (-1)
 */

int commentOutMacroCall(char *inputLine, FILE *outputfd)
{
	char *commentedLine = (char *) malloc((sizeof(inputLine) + (2 * sizeof(char)));

	/*
	 * Add a "." at the beginning of the input line to make it a comment
	 * and write it at the end of the output file.
	 */
	if(commentedLine != NULL) {
		memset(commentedLine, '\0', sizeof(commentedLine);
		strncpy(commentedLine,".", sizeof(char));
		strncat(commentedLine, inputLine, sizeof(inputLine);

		fseek(outputfd, 0, SEEK_END);
		fwrite(commentedLine, sizeof(commentedLine), 1, outputfd);

		return SUCCESS;
	}

	return FAILURE;
}

