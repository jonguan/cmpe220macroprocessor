/*
* cmpe220macroprocessor.c - Defines the entry point for the console application.
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "definitions.h"
#include "parser.h"
#include "test.h"

// Initialize global variables
///////////////////////////////////////////////////////////////////////////////////////////////

// Verbose flag - prints line numbers to output file and debug information to console
BOOL VERBOSE = FALSE;

// Expanding flag - for function expand
BOOL EXPANDING = FALSE; 

// OPCODE - to determine what the opcode currently is
char OPCODE[SHORT_STRING_SIZE];

// Expanded Label - to keep track of labels included with macro invocations
BOOL EXPAND_LABEL = FALSE;
char EXPANDED_LABEL[SHORT_STRING_SIZE];

// Pointer to current index of definitions table
int deftabIndex;

// Pointer to current line of input file
char currentLine [CURRENT_LINE_SIZE];

// Pointers to table structures
deftab_t * deftab = NULL;
namtab_t * namtab = NULL;
argtab_t * argtab = NULL;


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
	//filenames of 24 characters max
	char *inputFileName = NULL;// (char*)malloc(24*sizeof(char));
	char *outputFileName = NULL;//(char*)malloc(24*sizeof(char));
	int result;
	errno_t rc;
	FILE *inputFile;
	FILE *outputFile;

	if (VERBOSE)
		printf("beginning cmpe220 macroprocessor\n");

	/** HANDLE ARGUMENTS **/
	result = parseInputCommand(&inputFileName, &outputFileName, argc, argv);

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

		if (VERBOSE)
		{
			printf("input file is opened\n");
		}
		// Open OUTPUT file
		//Output is the last operand (argc-1)
		rc = fopen_s(&outputFile, outputFileName, "w");

		// Error check
		if (outputFile == NULL) {
			fprintf(stderr, "Can't open output file in main!\n");
			return FAILURE;
		}

		if (VERBOSE)
		{
			printf("output file is opened\n");
		}

		// Cleanup names
		//free(inputFileName);
		//free(outputFileName);

		// MACROPROCESSOR LOOP
		///////////////////////////////////////////////////////////////////
 		EXPANDING = FALSE;
        argtab = argtab_alloc();
        deftab = deftab_alloc();
        namtab = namtab_alloc();

		// reset result
		result = FAILURE;

		do
		{
			//Getline will fill currentLine buffer
			getline(inputFile);

			if(VERBOSE)
			{
				printf("currentLine is %s", currentLine);
			}
			
			// Error check
			/*if (currentLine[0] == '\0')
				printf("ERROR in getLine\n");
				break;*/

			result = processLine(inputFile, outputFile, currentLine);

			if (result != SUCCESS)
			{
				printf("ERROR in processLine\n");
				result = FAILURE;
				break;
			}

		}while(strncmp("END", OPCODE, strlen("END")) != 0);

		// CLEANUP
		////////////////////////////////////////////////////////////////////////////

        // de-allocate data structures
        namtab_free(namtab);
        deftab_free(deftab);
        argtab_free(argtab);

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
* inputFileName - pointer to char* filename
* outputFileName - pointer to char* filename
* argc from main
* argv from main
*
* Returns:
* SUCCESS (0) or FAILURE (-1)
*/
int parseInputCommand(char **inputFileName, char **outputFileName, int argc, char * argv[])
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
					*inputFileName = argv[i];
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
					//dereference
				*outputFileName = argv[i];
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
		for(n = 1; n <= ARGTAB_MAX_ARRAY_SIZE; n++) // iterate through ARGTAB
		{
			sprintf_s(str,sizeof(str),"?%d",n); // create "?n" as char
			argtab_val = argtab_get(argtab, n); // gets the value from ARGTAB
			strReplace(currentLine, sizeof(currentLine), str, argtab_val); // replaces "?n" with value found in ARGTAB
		}
	}
	else
	{
		// read next line from input file;
		fgets(currentLine,sizeof(currentLine),inputFile);
	}

	return currentLine;
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

/**
* Function: printOutputLine
* Description:
*  - Print the specified line to the output file, while also taking care of any
*    pending labels that need to be included in expanded macro lines.
* Parameters:
*  - outputFile - FILE pointer to output file.
*  - line - Pointer to line of code that will be printed.
* Returns:
*  - none
*/
void printOutputLine(FILE * outputFile, const char * line)
{
    parse_info_t * parseInfo = NULL;
    if(outputFile != NULL && line != NULL)
    {
        parseInfo = parse_info_alloc();
        if(parseInfo == NULL)
        {
            return;
        }
        if(parse_line(parseInfo, line) != 0)
        {
            parse_info_free(parseInfo);
            return;
        }

        // check if we need to include a label in an expanded line
        if(parseInfo->isComment == FALSE && EXPAND_LABEL == TRUE)
        {
            fprintf(outputFile, EXPANDED_LABEL);
            line += strlen(EXPANDED_LABEL);
            fprintf(outputFile, line);
            memset(EXPANDED_LABEL, 0, sizeof(EXPANDED_LABEL));
            EXPAND_LABEL = FALSE;
        }
        else
        {
            // just print the line
            fprintf(outputFile, line);
        }
        parse_info_free(parseInfo);
    }
}