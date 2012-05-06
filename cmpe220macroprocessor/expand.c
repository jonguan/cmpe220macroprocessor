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
int setUpArguments (const char * macroDef, const char *line, const char *macroName, int maxArgs);
int commentOutMacroCall(char *inputLine, FILE *outputfd);
int getNumParameters(char *line, const char *macroName);
int evaluateOperands(char *operands);
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
	
	// If/WHILE Statement level - For conditional macro expansion
	int IFSTATEMENTLEVEL = 0;
	int WHILESTATEMENTLEVEL = 0;
	/* 
		For nested ifs, we keep track of result of the IF expression evaluation
		Global variable IFSTATEMENTLEVEL is a pointer to the current index in array
		
		For nested whiles, we keep track of the definition line number in an array,
		so that we can go back to that line when the while loops back. 
		WHILESTATEMENTLEVEL is a pointer to the level of indirection in nested while loops.
	*/
	BOOL shouldEvaluateSection = TRUE;
	int ifExpressionResult;
	BOOL isWhileExpression = FALSE;
	int nestedIFArray[MAX_NESTED_IF_SIZE];
	int nestedWhileArray[MAX_NESTED_WHILE_SIZE];
	char *line;
	char *labelledLine;
    char *macroInvocation;
	const char opDelim[] = "& ";
	int argCount;
	int endOfMacroDef;
	int bufferLen;
	int sizeOfTAB; 
	namtab_entry_t *nameEntry;
	parse_info_t *parsedLine = parse_info_alloc();
	int i = 0;

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
    macroInvocation = _strdup(currentLine);
	if (commentOutMacroCall(macroInvocation, outputFileDes) == FAILURE) {
        free(macroInvocation);
		return FAILURE;
	}

	/* 
	 * Read from NAMTAB, the starting and ending index of macro definition in DEFTAB
	 * and process each line from DEFTAB
	 */
	nameEntry = namtab_get(namtab, macroName);
	if (nameEntry == NULL) {
        free(macroInvocation);
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
	// Get macro definition line
	line = getline(inputFileDes);
	argCount = getNumParameters(line, macroName);

	/* Create ARGTAB with arguments from macro invocation */
	if (setUpArguments(macroInvocation, line, macroName, argCount) == FAILURE) {
        free(macroInvocation);
		return FAILURE;
	}
    free(macroInvocation);
	
	// Set up nested if array
	
	memset(nestedIFArray, '\0', MAX_NESTED_IF_SIZE);

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

		/* 
			Check for conditional expansion keywords - IF, ELSE, ENDIF, WHILE, ENDW
			Algorithm is as follows:
			1. When line hits IF or WHILE, increment the respective counter
			2. If is allowed to evaluate the entire section, then evaluate the expression
			3. else set the array value to FALSE.
			4. when endw or endif is hit, decrement the respective counter and restore the 
			5. if an ELSE is hit, and allowed to evaluate the entire section, evaluate what follows until endif.
			6. if else is hit and not allowed to evaluate the entire section, skip the section.
		*/
		if(strcmp(parsedLine->opcode, "IF") == SUCCESS || strcmp(parsedLine->opcode, "WHILE") == SUCCESS)
		{
			isWhileExpression = (strcmp(parsedLine->opcode, "WHILE") == SUCCESS);

			// Increment global variable
			if(isWhileExpression)
				WHILESTATEMENTLEVEL++;
			else
				IFSTATEMENTLEVEL++;
			
			// Evaluate operands only if allowed to evaluate entire section
			if(shouldEvaluateSection)
				ifExpressionResult = evaluateOperands(parsedLine->operators);
			else
				ifExpressionResult = SKIP;

			if(ifExpressionResult == FAILURE)
			{
				printf("ERROR: Failed to parse operands in IF statement.\n");
				return FAILURE;
			}
            else 
			{
				if(isWhileExpression)
					nestedWhileArray[WHILESTATEMENTLEVEL] = ifExpressionResult;
				else
					nestedIFArray[IFSTATEMENTLEVEL] = ifExpressionResult;
			}
			// Only evaluate section if it's true, otherwise skip
			shouldEvaluateSection = (ifExpressionResult == TRUE);
			
			// skip over if line
			continue;

		}
		else if(strcmp(parsedLine->opcode, "ENDIF") == SUCCESS || strcmp(parsedLine->opcode, "ENDW") == SUCCESS)
		{
			isWhileExpression = (strcmp(parsedLine->opcode, "ENDW") == SUCCESS);

			// Increment global variable
			if(isWhileExpression)
				WHILESTATEMENTLEVEL--;
			else
				IFSTATEMENTLEVEL--;
			
			//Check if global variable is less than 0
			if(IFSTATEMENTLEVEL < 0 || WHILESTATEMENTLEVEL < 0)
			{
				printf("ERROR: Number of ENDIF/ENDW Statements do not match with number of IF/WHILE statements");
				return FAILURE;
			}

			// Resets shouldEvaluateSection to the value from before the if/while started
			// IF/WHILE STATEMENT LEVELs were decremented before this section
			if (isWhileExpression)
				shouldEvaluateSection = (nestedWhileArray[WHILESTATEMENTLEVEL] == TRUE);
			else
			{
				shouldEvaluateSection = (nestedIFArray[IFSTATEMENTLEVEL] == TRUE);
			}
			continue;
			
		}
		else if(strcmp(parsedLine->opcode, "ELSE") == SUCCESS && IFSTATEMENTLEVEL > 0)
		{
			//Process the next line until endif only if IF evaluation was false
			shouldEvaluateSection = ! nestedIFArray[IFSTATEMENTLEVEL];

			//Replace the value inside the nestedIFArray with the new value in case a new if/endif disrupts parsing
			nestedIFArray[IFSTATEMENTLEVEL] = shouldEvaluateSection;

			continue;
		}

		if(shouldEvaluateSection == TRUE)
		{
			processLine(inputFileDes, outputFileDes, line);
		}
	
		deftabIndex++;
	}
	
	EXPANDING = FALSE;
	UNIQUE_ID++;        // increment invocation ID
    argtab_clear(argtab); // also clear the argtab -- otherwise, screws up getline

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
int setUpArguments (const char *currLine, const char *macroDef, const char *macroName, int maxArgs)
{
	int argCount = 0;
	char *operand = NULL, *defOperand = NULL;
	parse_info_t *splitInvLine = NULL;
    parse_info_t *splitDefLine = NULL;
	char *nextInvToken = NULL;
    char *nextDefToken = NULL;
    char tmpKey[ARGTAB_STRING_SIZE];
    char tmpValue[ARGTAB_STRING_SIZE];
	
    splitInvLine = parse_info_alloc(); // create empty parse_info_t
    splitDefLine = parse_info_alloc();

	/* 
	 * If ARGTAB creation succeeded, proceed to parse the input line
	 * into tokens - label, opcode, operands string.
	 */
	if ((argtab == NULL) || (splitInvLine == NULL) || (parse_line(splitInvLine, currLine) < 0) ||
        (splitDefLine == NULL) || (parse_line(splitDefLine, macroDef) < 0)) {
        parse_info_free(splitInvLine);
        parse_info_free(splitDefLine);
		return FAILURE;
	}
	
	if (splitDefLine->isComment == TRUE) {		// Input line is a comment
        parse_info_free(splitInvLine);
        parse_info_free(splitDefLine);
		return SUCCESS;
	}

	/* 
	 * If there is a label for the macro invocation, use it as a label
	 * for the first instruction in the macro definition, while expanding.
	 */
	if (splitDefLine->label != NULL) {
		currentLabel = _strdup(splitInvLine->label);
	}

    // make sure we have operators
    if(splitInvLine->operators == NULL || splitDefLine->operators == NULL)
    {
        parse_info_free(splitInvLine);
        parse_info_free(splitDefLine);
        return FAILURE;
    }

    // clear the ARGTAB
    argtab_clear(argtab);

    if(splitDefLine->hasKeywordMacroParameters)
    {
        /*
         * Keyword Macro Parameters are used, so construct ARGTAB from both
         * macro definition (default values) and macro invocation (passed
         * values).
         */

        // First, set the default values from the macro definition
        defOperand = strtok_s(splitDefLine->operators, ", ", &nextDefToken);
        while(defOperand != NULL)
        {
            splitKeyValuePair(defOperand, tmpKey, sizeof(tmpKey), tmpValue, sizeof(tmpValue));
            argtab_addOrSet(argtab, tmpKey, tmpValue);
            defOperand = strtok_s(NULL, ", ", &nextDefToken);
        }

        // Next, set the values passed in to the macro invocation
        operand = strtok_s(splitInvLine->operators, ", ", &nextInvToken);
        while(operand != NULL)
        {
            splitKeyValuePair(operand, tmpKey, sizeof(tmpKey), tmpValue, sizeof(tmpValue));
            argtab_addOrSet(argtab, tmpKey, tmpValue);
            operand = strtok_s(NULL, ", ", &nextInvToken);
        }
    }
    else
    {
        /*
	     * Fill ARGTAB with arguments from macro invocation.
	     * Format of arguments in operands field: &op1,&op2,&op3,...
	     * ARGTAB indexing starts at 1.
	     */
	    operand = strtok_s(splitInvLine->operators, ", ", &nextInvToken);
        defOperand = strtok_s(splitDefLine->operators, ", ", &nextDefToken);
        while(operand != NULL && defOperand != NULL)
        {
            argtab_add(argtab, defOperand, operand);
            operand = strtok_s(NULL, ", ", &nextInvToken);
            defOperand = strtok_s(NULL, ", ", &nextDefToken);
        }
    }
	
    parse_info_free(splitInvLine);
    parse_info_free(splitDefLine);
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
	if ((defLine == NULL) || (parse_line(defLine, line) < 0) || 
		strncmp(defLine->label, macroName, strlen(defLine->opcode)) != SUCCESS ) {
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


/*
 * evaluateOperands:
 * Returns result of evaluation of conditional macro expansion IF
 *
 * Parameters:
 *  - operands - string containing ( comparison )
 *  
 * Returns:
 *  - TRUE, true
 *  - FALSE, if false
 *  - FAILURE, for invalid syntax
 */
int evaluateOperands(char *operands)
{
	char * leftoperand = NULL;
	char * middleoperand = NULL;
	char * rightoperand = NULL;
	char *ptr = operands;
    char val[30];
    int n, count = 0;

	// parse the conditional statement
	while(sscanf(ptr, "%31[^ ]%n", val, &n) == 1)
    {
		// this assumes that there are only 3 operands; I'm going to leave this for now, but may change it if necessary
		if(count == 0) leftoperand = _strdup(val);
		if(count == 1) middleoperand = _strdup(val);
		if(count == 2) rightoperand = _strdup(val);
		count++;
        ptr += n;
        if ( *ptr != ' ' ) break;
        ++ptr;
    }

	memmove(leftoperand, leftoperand+1, strlen(leftoperand)); // remove the left parentheses from the leftoperand
	rightoperand[strlen(rightoperand)-1] = 0; // remove the right parentheses from the rightoperand

	leftoperand = argtab_get(argtab, leftoperand); //  get the value of leftoperand from argtab
	
	// this series of if/else statements will evaluate the conditional statement and return TRUE or FALSE
	if(strcmp(middleoperand,"EQ") == 0)
	{
		if(strcmp(leftoperand, rightoperand) == 0)
			return TRUE;
		else
			return FALSE;
	}
	else if(strcmp(middleoperand,"NE") == 0)
	{
		if(strcmp(leftoperand, rightoperand) != 0)
			return TRUE;
		else
			return FALSE;
	}
	else if(strcmp(middleoperand,"GT") == 0)
	{
		if(strcmp(leftoperand, rightoperand) > 0)
			return TRUE;
		else
			return FALSE;
	}
	else if(strcmp(middleoperand,"LT") == 0)
	{
		if(strcmp(leftoperand, rightoperand) < 0)
			return TRUE;
		else
			return FALSE;
	}

	return FAILURE;
}