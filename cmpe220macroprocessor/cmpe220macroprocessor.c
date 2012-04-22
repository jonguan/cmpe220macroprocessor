/*
* cmpe220macroprocessor.c - Defines the entry point for the console application.
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "definitions.h"
#include "argtab.h"
#include "deftab.h"
#include "namtab.h"
#include "parser.h"
#include "test.h"

char* getline(FILE *inputFile);
int processLine(FILE * inputFile, FILE* outputFile, const char * macroLine);
int define(FILE * inputFile, FILE * outputFile, const char * macroLine);
int expand(FILE *inputFile, FILE *outputFile, const char *macroName);
void printUsage(void);
int getPositiveMin(int a, int b);
void strReplace(char * string, size_t bufsize, const char * replace, const char * with);
int parseInputCommand(char *inputFileName, char *outputFileName, int argc, char * argv[]);

// global variables
static 	BOOL VERBOSE = FALSE;
static BOOL EXPANDING; 
static char* OPCODE;
static int deftabIndex;
static char currentLine[256];
static deftab_t * deftab = NULL;
static namtab_t * namtab = NULL;
static argtab_t * argtab = NULL;

/**
* Function: printUsage
* Description:
*  - Documents the various options that can be supplied to the executable.
*    Prints the information to the user.
* Parameters:
*  - none
* Returns:
*  - none
*/
void printUsage(void)
{
	printf("\nUsage:\n");
	printf("    -i inputFile (Input file name)\n");
	printf("    -o outputFile (Output file name)\n");
	printf("    -v (Verbose mode)\n");
	printf("    -t (Unit test mode - will be removed in production code)\n");
	printf("    -? (Display usage info)\n\n");
}

/**
* Function: main
* Description:
*  - Documents the various options that can be supplied to the executable.
*    Prints the information to the user.
* Parameters:
* Flags 
* -i inputFile (required)
* -o outputFile (required)
* -v (optional - verbose mode)
* -t (optional - test mode)
* -? (optional - display usage info)
* Returns:
* SUCCESS (0) or FAILURE (-1)
*/
int main(int argc, char* argv[])
{
	char *inputFileName = NULL;
	char *outputFileName = NULL;
	int result;
	errno_t rc;
	FILE *inputFile = NULL;
	FILE *outputFile = NULL;
	if (VERBOSE)
		printf("beginning cmpe220 macroprocessor\n");

	/** HANDLE ARGUMENTS **/
	result = parseInputCommand(inputFileName, outputFileName, argc, argv);

	if (result == FAILURE || argc < 3)
	{
		// Break early, if possible
		return result;
	}
	else
	{
		// File I/O
		////////////////////////////////////////////////////////////////////////////////////////////
			// Open INPUT file
		//Output is the last operand (argc-1)
		rc = fopen_s(&inputFile, inputFileName, "r");

		// Error check
		if (inputFile == NULL) {
			fprintf(stderr, "Can't open input file in main!\n");
			return FAILURE;
		}

		// Open OUTPUT file
		//Output is the last operand (argc-1)
		rc = fopen_s(&outputFile, outputFileName, "w");

		// Error check
		if (outputFile == NULL) {
			fprintf(stderr, "Can't open output file in main!\n");
			return FAILURE;
		}

		// MACROPROCESSOR LOOP
		///////////////////////////////////////////////////////////////////
		EXPANDING = FALSE;

		// reset result
		result = FAILURE;

		while(strcmp(OPCODE, "END") != 0)
		{
			currentLine = getline(inputFile);

			// Error check
			if (currentLine == NULL)
				printf("ERROR in getLine\n");
				break;

			result = processLine(inputFile, outputFile, currentLine);

			if (result != SUCCESS)
			{
				printf("ERROR in processLine\n");
				result = FAILURE;
				break;
			}

		}

		// CLEANUP
		////////////////////////////////////////////////////////////////////////////

		// Close Files
		fclose(inputFile);
		fclose(outputFile);

		return result;
	}

	return SUCCESS;
}

/**
* Function: parseInputCommand
* Description:
*  - Parses the input line of the command.
*	 Sets (Mutates) the inputFileName and outputFileName so that calling function can open/close
*    Prints the information to the user.
*
* Parameters:
* inputFileName - char* filename
* outputFileName - char* filename
* argc from main
* argv from main
*
* Returns:
* SUCCESS (0) or FAILURE (-1)
*/
int parseInputCommand(char *inputFileName, char *outputFileName, int argc, char * argv[])
{
	int i;

	if(argc <= 1)
	{
		printUsage();
		return SUCCESS;
	}
	else if (argc == 2)
	{
		// these options can only be used by themselves
		if(strcmp("-?", argv[1]) == 0)
		{
			printUsage();
			return SUCCESS;
		}
		if(strcmp("-t", argv[1]) == 0)
		{
			debug_testDataStructures();
			debug_testParser();
			return SUCCESS;
		}
	}
	else
	{
		// the rest of the options may appear in any order
		for(i = 1; i < argc; i++)
		{
			if(strcmp("-v", argv[i]) == 0)
			{
				VERBOSE = TRUE;
			}
			else if(strcmp("-i", argv[i]) == 0)
			{
				// must also be followed by input file name
				if(i+1 < argc) // make sure there's another argument
				{
					i++;
					inputFileName = argv[i];
				}
				else
				{
					// bad arguments - print usage
					printUsage();
					return FAILURE;
				}
			}
			else if(strcmp("-o", argv[i]) == 0)
			{
				// must also be followed by output file name
				if(i+1 < argc) // make sure there's another argument
				{
					i++;
					outputFileName = argv[i];
				}
				else
				{
					// bad arguments - print usage
					printUsage();
					return FAILURE;
				}
			}
			else // unrecognized options
			{
				printUsage();
				return FAILURE;
			}
		}

		// make sure we have input and output files defined
		if(inputFileName == NULL || outputFileName == NULL)
		{
			printUsage();
			return FAILURE;
		}

	
	}

	return SUCCESS;
}


char* getline(FILE * inputFile)
{
	char * line = NULL;
	char * argtab_val = NULL;
	char str[8];
	int n;

	// Error check
	if (inputFile == NULL) {
		fprintf(stderr, "Null inputFile passed to getLine!\n");
		return NULL;
	}

	if(EXPANDING)
	{
		// get next line of macro definition from DEFTAB
		line = deftab_get(deftab, deftabIndex);
		strcpy_s(currentLine, sizeof(currentLine), line);
		// substitute arguments from ARGTAB for positional notation  
		for(n = 0; n < ARGTAB_MAX_ARRAY_SIZE; n++) // iterate through ARGTAB
		{
			char ntext[3]; 
			_itoa(n, ntext, 10); // need to convert int n to char
			sprintf(str,"?%d",n); // create "?n" as char
			argtab_val = argtab_get(argtab, n); // gets the value from ARGTAB
			strReplace(currentLine, sizeof(currentLine), str, argtab_val); // replaces "?n" with value found in ARGTAB
		}
	}
	else
	{
		// read next line from input file;
		fgets(currentLine,255,inputFile);
	}

	return currentLine;
}

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
	//TODO get rid of this - only until get line is implemented
	int currentLineIndex = 1;
	char dummyLines[][128] = {
		"RDBUFF    MACRO   &INDEV, &BUFADR, &RECLTH",
		".",
		".         MACRO TO READ RECORD INTO BUFFER",
		".",
		"          CLEAR   X             CLEAR LOOP COUNTER",
		"          CLEAR   A",
		"          CLEAR   S",
		"         +LDT    #4096          SET MAXIMUM RECORD LENGTH",
		"          TD     =X'&INDEV'     TEST INPUT DEVICE",
		"          JEQ     *-3           LOOP UNTIL READY",
		"          RD     =X'&INDEV'     READ CHARACTER INTO REG A",
		"          COMPR   A,S           TEST FOR END OF RECORD",
		"          JEQ     *+11          EXIT LOOP IF EOR",
		"          STCH    &BUFADR,X     STORE CHARACTER IN BUFFER",
		"          TIXR    T             LOOP UNLESS MAXIMUM LENGTH",
		"          JLT     *-19             HAS BEEN REACHED",
		"          STX     &RECLTH       SAVE RECORD LENGTH",
		"          MEND",
	};
	char * currentLine = dummyLines[0];

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
	if(parse_info->opcode == NULL || parse_info->label == NULL || strcmp(parse_info->opcode, "MACRO") != 0)
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
		//TODO CALL GETLINE
		currentLine = dummyLines[currentLineIndex++];
		parse_info_clear(parse_info);
		if(parse_line(parse_info, currentLine) != 0)
		{
			argtab_free(tmp_argtab);
			parse_info_free(parse_info);
			return FAILURE;
		}

		if(parse_info->isComment == FALSE)
		{
			//TODO substitute positional notation for parameters
			tmpString = _strdup(currentLine);
			for(i = 1; i < argidx; i++)
			{
				argReplace = argtab_get(tmp_argtab, i);
				sprintf_s(argWith, sizeof(argWith), "?%d", i);
				strReplace(tmpString, strlen(tmpString) + 1, argReplace, argWith);
			}
			index = deftab_add(deftab, tmpString);
			free(tmpString);
			if(parse_info->opcode && strcmp(parse_info->opcode, "MACRO") == 0)
			{
				level++;
			}
			else if(parse_info->opcode && strcmp(parse_info->opcode, "MEND") == 0)
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

/**
* Funtion: getPositiveMin
* Description:
*  - Returns the lesser of two integers.
* Parameters:
*  - a: Integer one.
*  - b: Integer two.
* Returns:
*  - The lesser of the two integer values.
*/
int getPositiveMin(int a, int b)
{
	int result = 0;
	if(a >= 0 && b >= 0)
	{
		result = (a < b)? a : b;
	}
	else if(a >= 0)
	{
		result = a;
	}
	else if(b >= 0)
	{
		result = b;
	}

	return result;
}

/**
* Function: strReplace
* Description:
*  - Given a string and its buffer size, replaces all occurrences of substring
with another substring.
* Parameters:
*  - string: String to search in. Result will be written here too.
*  - bufsize: Size of the string buffer.
*  - replace: String to search for.
*  - with: String to replace with.
* Returns:
*  - none
*/
void strReplace(char * string, size_t bufsize, const char * replace, const char * with)
{
	char * tmpString = NULL;
	char * searchptr = NULL;
	char * srcptr = string;
	char * srcmax = string + strlen(string);
	char * dstptr = NULL;
	char * dstmax = NULL;
	int i = 0;
	int replaceSize;
	int withSize;
	int copySize;

	// some error checking
	if(string == NULL || bufsize <= 0 || replace == NULL || with == NULL)
	{
		return;
	}

	// allocate a temporary buffer, same size as the one supplied
	tmpString = (char *)malloc(bufsize);
	dstptr = tmpString;
	dstmax = dstptr + bufsize - 1;
	replaceSize = strlen(replace);
	searchptr = strstr(srcptr, replace);
	copySize = getPositiveMin(searchptr - srcptr, dstmax - dstptr);
	while(searchptr != NULL && dstptr < dstmax)
	{
		if(copySize > 0)
		{
			memcpy(dstptr, srcptr, copySize);   // copy until match
			dstptr += copySize;
			srcptr += copySize;
		}
		withSize = getPositiveMin(strlen(with), dstmax - dstptr);
		memcpy(dstptr, with, withSize);
		dstptr += withSize;
		srcptr += replaceSize;

		searchptr = strstr(srcptr, replace);
		copySize = getPositiveMin(searchptr - srcptr, dstmax - dstptr);
	}
	if(searchptr == NULL && srcmax > srcptr)
	{
		copySize = getPositiveMin(srcmax - srcptr, dstmax - dstptr);
		memcpy(dstptr, srcptr, copySize);
		dstptr += copySize;
		srcptr += copySize;
	}
	*dstptr = '\0'; // null termination
	strcpy_s(string, bufsize, tmpString);
	free(tmpString);
}
