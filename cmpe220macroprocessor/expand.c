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
#include "definitions.h"
#include "parser.h"

// local function definitions
int setUpArguments (char *line, const char *macroName);
int commentOutMacroCall(char *inputLine, FILE *outputfd);

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

    // check for null pointers
    if(inputFileDes == NULL || outputFileDes == NULL || macroName == NULL)
    {
        return FAILURE;
    }

	/* Create ARGTAB with arguments from macro invocation */
	argCount = setUpArguments(currentLine, macroName);
	if (argCount < 0) {
		return FAILURE;
	}
	
	if(VERBOSE){
		printf("Number of arguments for macro %s: %d\n", macroName, argCount);
	}

	/* Write macro invocation line to the output file as a comment */
	if (commentOutMacroCall(currentLine, outputFileDes) == FAILURE) {
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
	deftabIndex = 1 + (nameEntry->deftabStart);  // First line is macro prototype!
	endOfMacroDef = nameEntry->deftabEnd;
		
	while (deftabIndex < endOfMacroDef) {	// Assumes the MACRO definition ends with MEND in DEFTAB!
		line = getline(inputFileDes);
		processLine(inputFileDes, outputFileDes, line);
		deftabIndex++;
	}

	EXPANDING = FALSE;

	return SUCCESS;
}


/*
 * setUpArguments:
 * Set up ARGTAB with arguments from macro invocation.
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
	parse_info_t *splitLine = NULL;
	char *nextToken = NULL;
	
    splitLine = parse_info_alloc(); // create empty parse_info_t

	/* 
	 * If ARGTAB creation succeeded, proceed to parse the input line
	 * into tokens - label, opcode, operands string.
	 */
	if ((argtab == NULL) || (splitLine == NULL) || (parse_line(splitLine, line) < 0)) {
        parse_info_free(splitLine);
		return FAILURE;
	}

	if (splitLine->isComment == TRUE) {		// Input line is a comment
        parse_info_free(splitLine);
		return SUCCESS;
	}

	/*
	 * Fill ARGTAB with arguments from macro invocation.
	 * Format of arguments in operands field: &op1,&op2,&op3,...
	 * ARGTAB indexing starts at 1.
	 */
	operand = strtok_s(splitLine->operators, ",& ", &nextToken);
	while (operand != NULL){
		argCount++;
		if (argtab_add(argtab, argCount, operand) < 0) {
            parse_info_free(splitLine);
			return FAILURE;
		}
		operand = strtok_s(NULL, ",& ", &nextToken);
	}
	
    parse_info_free(splitLine);
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
    int bufferLen;
	char *commentedLine;
    
    if(inputLine == NULL || outputfd == NULL)
    {
        return FAILURE;
    }
    
    bufferLen = strlen(inputLine) + (2 * sizeof(char)) + 1;
    commentedLine = (char *) malloc(bufferLen);

	/*
	 * Add a "." at the beginning of the input line to make it a comment
	 * and write it at the end of the output file.
	 */
	if(commentedLine != NULL) {
		memset(commentedLine, '\0', bufferLen);
		strcpy_s(commentedLine, bufferLen, ".");
		strcat_s(commentedLine, bufferLen - strlen(commentedLine), inputLine);

	/*	fseek(outputfd, 0, SEEK_END);
		fwrite(commentedLine, sizeof(commentedLine), 1, outputfd);*/

		fprintf(outputfd, commentedLine);

        free(commentedLine);
		return SUCCESS;
	}

	return FAILURE;
}

