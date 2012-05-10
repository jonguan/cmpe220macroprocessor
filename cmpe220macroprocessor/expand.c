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
int setUpArguments (const char * macroDef, const char *line, const char *macroName);
int commentOutMacroCall(char *inputLine, FILE *outputfd);
int getNumArguments(char *line);
int evaluateIFOperands(char *operands);
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
	//int argCount;
	int endOfMacroDef;
	int bufferLen;
	int sizeOfTAB; 
	namtab_entry_t *nameEntry;
	parse_info_t *parsedLine = parse_info_alloc();
	int i = 0;

	/* 
		Initialize variables
	*/
	EXPANDING = TRUE;
	memset(nestedIFArray, '\0', MAX_NESTED_IF_SIZE);
	memset(nestedWhileArray, '\0', MAX_NESTED_WHILE_SIZE);
	nestedIFArray[0] = TRUE;
	nestedWhileArray[0] = TRUE;

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
	

	/* Create ARGTAB with arguments from macro invocation */
	if (setUpArguments(macroInvocation, line, macroName) == FAILURE) {
        free(macroInvocation);
		return FAILURE;
	}
    free(macroInvocation);
	

	// Increment deftabIndex to point to first line of definition
	deftabIndex++;

	while (deftabIndex < endOfMacroDef) {	// Assumes the MACRO definition ends with MEND in DEFTAB!
		line = getline(inputFileDes);

		/* If macro invocation came with a label, copy the label down to next available line
			where there is not a conditional macro variable
		*/
		if (currentLabel != NULL && *line != '&') {
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
			Substitute arguments for operators here
		*/
		if(parsedLine->operators != NULL && shouldEvaluateSection)
		{
			// Copy parsedLine->operators to currentLine buffer
			strncpy_s(currentLine, CURRENT_LINE_SIZE, parsedLine->operators, strlen(parsedLine->operators));

			argtab_substituteValues(argtab, currentLine, sizeof(currentLine));

			// Move currentLine back to parsedLine->operators
			free(parsedLine->operators);
			parsedLine->operators = _strdup(currentLine);

			
		}
		// Write back into currentLine
		parse_reconstruct_string(parsedLine, currentLine); 

		/* 
			Check for conditional expansion keywords - IF, ELSE, ENDIF, WHILE, ENDW
			Algorithm is as follows:
			1. When line hits IF or WHILE, increment the respective counter
			2. If is allowed to evaluate the entire section, then evaluate the expression
			3. else set the array value to SKIP.
			4. when endif is hit, decrement the respective counter and restore the previous result
			4A.If endw is hit, evaluate the while expression again.  if result is still good, loop
			4B.If while result is bad, then skip to the endw line, decrement counter, and restore previous while result.
			5. if an ELSE is hit, and allowed to evaluate the entire section, evaluate what follows until endif.
			6. if else is hit and not allowed to evaluate the entire section, skip the section.
		*/
		if(strncmp(parsedLine->opcode, "IF", strlen("IF")) == SUCCESS || 
			strncmp(parsedLine->opcode, "WHILE", strlen("WHILE")) == SUCCESS)
		{
			isWhileExpression = (strncmp(parsedLine->opcode, "WHILE", strlen("WHILE")) == SUCCESS);

			// Increment global variable
			if(isWhileExpression)
				WHILESTATEMENTLEVEL++;
			else
				IFSTATEMENTLEVEL++;
			
			// Evaluate operands only if allowed to evaluate entire section
			if(shouldEvaluateSection)
			{
				ifExpressionResult = evaluateIFOperands(parsedLine->operators);
			}
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
					nestedWhileArray[WHILESTATEMENTLEVEL] = ifExpressionResult ? deftabIndex : SKIP;
				else
					nestedIFArray[IFSTATEMENTLEVEL] = ifExpressionResult;
			}
			// Only evaluate section if it's true, otherwise skip
			shouldEvaluateSection = (ifExpressionResult == TRUE);
			
			// skip over if line
			deftabIndex++;
			continue;

		}
		else if(strncmp(parsedLine->opcode, "ENDIF", strlen("ENDIF")) == SUCCESS || 
			strncmp(parsedLine->opcode, "ENDW", strlen("ENDW")) == SUCCESS)
		{
			isWhileExpression = strncmp(parsedLine->opcode, "ENDW", strlen("ENDW")) == SUCCESS;

			// Decrement global variable
			if (!isWhileExpression)
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
			{
				//If the while expression evaluated to false, break out of the loop
				if(shouldEvaluateSection == SKIP)
				{
					WHILESTATEMENTLEVEL--;
				}else if(shouldEvaluateSection == FALSE)
				{
					WHILESTATEMENTLEVEL--;
					shouldEvaluateSection = TRUE;
				}
				else{
					//While statements need to loop back if still true
					deftabIndex = nestedWhileArray[WHILESTATEMENTLEVEL];
					WHILESTATEMENTLEVEL --;
					continue;
				}
				
			}
			else
			{
				shouldEvaluateSection = (nestedIFArray[IFSTATEMENTLEVEL] == TRUE);
			}

			deftabIndex++;
			continue;
			
		}
		else if(strncmp(parsedLine->opcode, "ELSE", strlen("ELSE")) == SUCCESS && IFSTATEMENTLEVEL > 0)
		{
			//Process the next line until endif only if IF evaluation was false
			// Likewise, if the IF was true, then evaluate is FALSE
			shouldEvaluateSection = (shouldEvaluateSection > -1) ? !shouldEvaluateSection : shouldEvaluateSection; 
			
			//Replace the value inside the nestedIFArray with the new value in case a new if/endif disrupts parsing
			nestedIFArray[IFSTATEMENTLEVEL] = shouldEvaluateSection;

			deftabIndex++;
			continue;
		}
	

		if(shouldEvaluateSection == TRUE)
		{
			if(VERBOSE)
			{
				printf("currentLine is %s\n", currentLine);
			}
			processLine(inputFileDes, outputFileDes, currentLine);
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
 *  - currLine - macro invocation - does not yet have arguments substituted
 *  - macroName - Name of MACRO being expanded
 *  - maxArgs - number of Parameters for macro macroName
 * Returns:
 *  - >0, Argument count OR
 *  - 0, if inputLine is a comment or if no arguments found in macro invocation OR
 *  - -1, for all FAILURE cases
 */
int setUpArguments (const char *currLine, const char *macroDef, const char *macroName)
{
	int n=0, argCount = 0;
	char *operand, *defOperand = NULL;
	char *startPtr, *endPtr = NULL;
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
		// Declare memory for operand
		operand = (char *)malloc(SHORT_STRING_SIZE);
		memset(operand, '\0', SHORT_STRING_SIZE);

		startPtr = splitInvLine->operators;
		endPtr = strpbrk(splitInvLine->operators, ",");
		strncpy_s(operand, SHORT_STRING_SIZE, startPtr, (endPtr-startPtr));
	    //operand = strtok_s(splitInvLine->operators, ", ", &nextInvToken);
        defOperand = strtok_s(splitDefLine->operators, ", ", &nextDefToken);
        
		// Operand can be null
		while(defOperand != NULL)
        {
            argtab_add(argtab, defOperand, operand);

			//clear out previous operand
			memset(operand, '\0', SHORT_STRING_SIZE);

			if(endPtr != NULL)
			{
				startPtr = endPtr + 1;
				endPtr = strpbrk(startPtr, ",");
				
                if(endPtr != NULL)
                {
					if(*startPtr == '(')
					{
						// Expression (val1, val2, val3)
						endPtr = strpbrk(startPtr, ")");
						
						if(endPtr != NULL)
							endPtr++;
						else
							return FAILURE;
						

					}
					
					strncpy_s(operand, SHORT_STRING_SIZE, startPtr, (endPtr-startPtr));
					
                }
			}
			
           // operand = strtok_s(NULL, ", ", &nextInvToken);
            defOperand = strtok_s(NULL, ", ", &nextDefToken);
		}

		free(operand);
    }
	
    parse_info_free(splitInvLine);
    parse_info_free(splitDefLine);
	return argCount;
}


/*
 * evaluateExpressionOperands:
 * Returns number of arguments in the line passed in .
 *
 * Parameters:
 *  - inputLine - input string to be evaluated
 *  - outputBuffer
 *  
 * Returns:
 *  - Evaluation of expression in string format
 *  - count, if operands starts with %NITEMS
 *  - expression result, if operands are some sort of mathematical function
 *  - NULL if error in evaluation
 */
int evaluateExpressionOperands(char *operands)
{
	int result = 0;
	char delimiters[] = "(), ";
	char *lineCopy = NULL;
	char val[30];
	char operatorVal[2];
	int n, count = 0;
	char *start, *end;

	/*
	Check to see if arguments are of form %NITEMS
	Assumes that %NITEMS will start at index 0
	*/
	result = strncmp(operands, "%NITEMS", strlen("%NITEMS"));
	if(result == SUCCESS)
	{
		result = getNumArguments(operands + strlen("%NITEMS"));
		return result;
	}

	result = getNumArguments(operands);
	if(result == 0)
	{
		return 0;
	}
	//else if (result == 1 && strlen(operands) == 1)
	//	return atoi(operands);
	else
	{
		// TODO: evaluate the mathematical expression
		// parse the conditional statement to remove spaces and math operators
		start = operands;
		end = strpbrk(operands, "+-*/%() ");

		if(end == NULL)
			return atoi(start);

		strncpy_s(val, 30, start, (end-start));

		//Val now contains left operand
		start = end;

		//start should now either point to space or math operator
		n = strcspn(start, "+-*/%()");
		start += n;
		end = start+1;
		//get the math operator
			
		strncpy_s(operatorVal, 2, start, 1);

		start = end;

		// skip spaces
		while( *start == ' ' ) 
			start++;

		result = evaluateExpressionOperands(start);

		switch(operatorVal[0])
		{
		case '+':
			return atoi(val) + result;
		case '-':
			return atoi(val) - result;
		case '*':
			return atoi(val) * result;
		case '/':
			return atoi(val) / result;
		case '%':
			return atoi(val) % result;
		default:
			return atoi(val);
		}
	}
	/*else
			return atoi(operands);*/


	

	return 0;
}



/*
 * getNumArguments:
 * Returns number of arguments in the line passed in .
 *
 * Parameters:
 *  - inputLine - input macro definition line
 *  
 * Returns:
 *  - >0, Argument count OR
 *  - 0, if inputLine is a comment or if no arguments found in inputLine OR
 *  - -1, for all FAILURE cases
 */
int getNumArguments(char *line)
{
	int argCount = 0;
	char *operand;
	char *nextToken = NULL;
	const char argDelim[] = "(), ";
	
	/*
	 * Count number of arguments from macro invocation.
	 * Format of arguments in operands field: &op1,&op2,&op3,...
	 * Parameters must have & in front, with no space after.
	 */
	operand = strtok_s(line, argDelim, &nextToken);
	while (operand != NULL){
		argCount++;	
		operand = strtok_s(NULL, argDelim, &nextToken);
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
 * evaluateIFOperands:
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
int evaluateIFOperands(char *operands)
{
	char * leftoperand = NULL;
	char * middleoperand = NULL;
	char * rightoperand = NULL;
	char *ptr = operands;
    char val[30];
    int n, count = 0, result = 0;



	// parse the conditional statement
	while(sscanf(ptr, "%31[^ ]%n", val, &n) == 1)
    {
		// this assumes that there are only 3 operands; I'm going to leave this for now, but may change it if necessary
		if(count == 0) leftoperand = _strdup(val);
		if(count == 1) middleoperand = _strdup(val);
		if(count == 2) rightoperand = _strdup(val);
		count++;
        ptr += n;
		// skip spaces
        if ( *ptr != ' ' ) break;
        ++ptr;
    }


	memmove(leftoperand, leftoperand+1, strlen(leftoperand)); // remove the left parentheses from the leftoperand
	rightoperand[strlen(rightoperand)-1] = 0; // remove the right parentheses from the rightoperand

	// Special case for null characters
	if(strncmp(rightoperand, "''", strlen("''")) == SUCCESS)
		memset(rightoperand, '\0', strlen(rightoperand));

	//leftoperand = argtab_get(argtab, leftoperand); //  get the value of leftoperand from argtab
	
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
	}else if(strcmp(middleoperand, "LE") == 0)
		return (strcmp(leftoperand, rightoperand) <= 0);
	else if(strcmp(middleoperand, "GE") == 0)
		return (strcmp(leftoperand, rightoperand) >= 0);

	return FAILURE;
}