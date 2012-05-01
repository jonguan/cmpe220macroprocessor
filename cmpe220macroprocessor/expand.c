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
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include "definitions.h"
#include "parser.h"

// local function definitions
int setUpArguments (char *line, const char *macroName, int maxArgs);
int commentOutMacroCall(char *inputLine, FILE *outputfd);
int getNumParameters(char *line, const char *macroName);
char *currentLabel = NULL;

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
	char *labelledLine;
	char *operand;
	const char opDelim[] = "& ";
	int argCount;
	int endOfMacroDef;
	int bufferLen;
	int sizeOfTAB; 
	namtab_entry_t *nameEntry;
	parse_info_t *parsedLine = parse_info_alloc();

	EXPANDING = TRUE;
	
	if(VERBOSE) {
		printf("EXPAND: Expanding Macro: %s ...\n", macroName);
	}

    // check for null pointers
    if(inputFileDes == NULL || outputFileDes == NULL || macroName == NULL)
    {
        return FAILURE;
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

	/*
		Set up arguments to get from deftab
	*/
	deftabIndex = (nameEntry->deftabStart);  // First line is macro prototype!
	endOfMacroDef = nameEntry->deftabEnd;

	/*
		Get number of parameters from macro definitino
	*/
	line = getline(inputFileDes);
	argCount = getNumParameters(line, macroName);

	/* Create ARGTAB with arguments from macro invocation */
	if (setUpArguments(currentLine, macroName, argCount) == FAILURE) {
		return FAILURE;
	}
	
	// Increment deftabIndex to point to first line of definition
	deftabIndex++;

	while (deftabIndex < endOfMacroDef) {	// Assumes the MACRO definition ends with MEND in DEFTAB!
		line = getline(inputFileDes);

		/* If macro invocation came with a label, copy the label down to next line */
		if (currentLabel != NULL) {
			bufferLen = strlen(currentLabel) + strlen(line) + (2 * sizeof(char));
			labelledLine = (char *) malloc(bufferLen);
			memset(labelledLine, '\0', bufferLen);

			sizeOfTAB = sizeof('\t');
			strcpy_s(labelledLine, bufferLen, currentLabel);
			strcat_s(labelledLine, bufferLen, &line[sizeOfTAB]+1);
			strcpy_s(line, bufferLen, labelledLine);

			currentLabel = NULL;
			free(labelledLine);
		}
		
		/* Parse for conditional expansion*/
		if(parse_line(parsedLine, line) == FAILURE)
		{
			return FAILURE;
		}

		// Check first character for argument
		if(parsedLine->label != NULL &&
			*(parsedLine->label) == '&' && 
			!isspace(*((parsedLine->label) + 1)) && 
			strcmp("SET", parsedLine->opcode) == SUCCESS)
		{
			//Add to argtab
			operand = strtok(parsedLine->label, opDelim);
			if (argtab_add(argtab, argCount, operand) < 0) {
				parse_info_free(parsedLine);
				return FAILURE;
			}

			//Increment argCount for next cond. variable, if exists
			argCount++;
		}
		processLine(inputFileDes, outputFileDes, line);
		deftabIndex++;
	}
	
	EXPANDING = FALSE;
	UNIQUE_ID++;        // increment invocation ID

	// free memory
	parse_info_free(parsedLine);

	return SUCCESS;
}


/*
 * setUpArguments:
 * Set up ARGTAB with arguments from macro invocation.
 *
 * Parameters:
 *  - inputLine - input assembly program line to comment out
 *  - macroName - Name of MACRO being expanded
 *  - maxArgs - number of Parameters for macro macroName
 * Returns:
 *  - >0, Argument count OR
 *  - 0, if inputLine is a comment or if no arguments found in macro invocation OR
 *  - -1, for all FAILURE cases
 */
int setUpArguments (char *line, const char *macroName, int maxArgs)
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
	 * If there is a label for the macro, use it as a label
	 * for the first instruction in the macro definition, while expanding.
	 */
	if (splitLine->label != NULL) {
		currentLabel = _strdup(splitLine->label);
	}

	/*
	 * Fill ARGTAB with arguments from macro invocation.
	 * Format of arguments in operands field: &op1,&op2,&op3,...
	 * ARGTAB indexing starts at 1.
	 */
	operand = strtok_s(splitLine->operators, ",& ", &nextToken);

	// Null operands are now allowed for cases of cond. expansion
	for (argCount = 0; argCount < maxArgs; argCount ++){
		
		operand = strtok(operand, " ");
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
 * getNumParameters:
 * Returns number of parameters from macro definition.
 *
 * Parameters:
 *  - inputLine - input macro definition line
 *  - macroName - Name of MACRO being expanded
 * Returns:
 *  - >0, Argument count OR
 *  - 0, if inputLine is a comment or if no arguments found in macro invocation OR
 *  - -1, for all FAILURE cases
 */
int getNumParameters(char *line, const char *macroName)
{
	int argCount = 0;
	char *operand;
	parse_info_t *defLine = NULL;
	char *nextToken = NULL;
	const char argDelim[] = ", ";
	
    defLine = parse_info_alloc(); // create empty parse_info_t

	/* 
	 * If ARGTAB creation succeeded, proceed to parse the input line
	 * into tokens - label, opcode, operands string.
	 */
	if ((defLine == NULL) || (parse_line(defLine, line) < 0) || strcmp(defLine->opcode, macroName) != SUCCESS ) {
        parse_info_free(defLine);
		return FAILURE;
	}
	

	/*
	 * Count number of arguments from macro invocation.
	 * Format of arguments in operands field: &op1,&op2,&op3,...
	 * Parameters must have & in front, with no space after.
	 */
	operand = strtok_s(defLine->operators, argDelim, &nextToken);
	while (operand != NULL){
		// Parameter must start with & and have non-whitespace follow
		if(*operand == '&' && !isspace(*(operand+1)))
			argCount++;	
		operand = strtok_s(NULL, argDelim, &nextToken);
	}
	
	parse_info_free(defLine);
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
    
    bufferLen = strlen(inputLine) + (2 * sizeof(char));
    commentedLine = (char *) malloc(bufferLen);

	/*
	 * Add a "." at the beginning of the input line to make it a comment
	 * and write it at the end of the output file.
	 */
	if(commentedLine != NULL) {
		memset(commentedLine, '\0', bufferLen);
		strcpy_s(commentedLine, bufferLen, ".");
		strcat_s(commentedLine, bufferLen, inputLine);

		fprintf(outputfd, commentedLine);

        free(commentedLine);
		return SUCCESS;
	}

	return FAILURE;
}

