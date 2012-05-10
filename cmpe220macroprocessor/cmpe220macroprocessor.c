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

// Unique ID - Used to identify a macro invocation, for unique label generation
int  UNIQUE_ID = 0;

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
			runTests();
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

		// substitute arguments from ARGTAB with values 
		// MOVED TO EXPAND FUNCTION SO THAT WE PASS IN OPERATORS ONLY!
		//argtab_substituteValues(argtab, currentLine, sizeof(currentLine));

		
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
void strReplace(char * string, size_t bufsize, const char * replace, const char * with, BOOL valIsArray)
{
	char * tmpString = NULL; //New string is temporarily written here
	char * searchptr = NULL; // Pointer to each instance of replace
	char * srcptr = string;  // Pointer to current string
	char * srcmax = string + strlen(string); //End of string
	char * dstptr = NULL; // Pointer to tmpString
	char * dstmax = NULL; // Pointer to end of tmpString
	int i = 0;
	int replaceSize; //strlen of replace
	int withSize; // strlen of with
	int copySize; // length of string to copy 

	// Variables to replace array values
	char arrayValBuffer[ARGTAB_STRING_SIZE]; //place to put array value if "with" is an array
	char arrayIndexBuffer[ARGTAB_STRING_SIZE];
	char *arrayIndexPtr = NULL;
	char *arrayEndPtr = NULL;

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
	// distance from replace and current place in string vs 
	// distance from end of destination string and current place in destination
	copySize = getPositiveMin(searchptr - srcptr, dstmax - dstptr);
	
	

	while(searchptr != NULL && dstptr < dstmax)
	{
	
		if(copySize > 0)
		{
			//copies into dstptr, srcptr of size copySize
			memcpy(dstptr, srcptr, copySize);   // copy original string up to matched string
			dstptr += copySize;
			srcptr += copySize;
		}

		/*
		Check if with variable is an array
		*/
		if(valIsArray && *(arrayIndexPtr = searchptr + strlen(replace)) == '[')
		{
			// Check if value inside [] is replaced
			
			//get value on inside
			if(arrayIndexPtr+1 == NULL || *(arrayIndexPtr+1) == ']')
			{
				// Error - can not end without value
				return;
			}
			// Get index value
			arrayEndPtr = strpbrk(arrayIndexPtr, "]");
			strncpy_s(arrayIndexBuffer, ARGTAB_STRING_SIZE, ++arrayIndexPtr, (arrayEndPtr - arrayIndexPtr));

			if (*arrayIndexBuffer == '&')
			{
				// Get value and put into arrayIndexBuffer
				strcpy_s(arrayIndexBuffer, ARGTAB_STRING_SIZE, argtab_get(argtab, arrayIndexBuffer));
			}

			// Get array value with index
			arrayValueForIndex(with, arrayValBuffer, arrayIndexBuffer);

			withSize = getPositiveMin(strlen(arrayValBuffer), dstmax - dstptr);
			memcpy(dstptr, arrayValBuffer, withSize);
			dstptr += withSize;
			srcptr = ++arrayEndPtr;
		}
		else
		{
			withSize = getPositiveMin(strlen(with), dstmax - dstptr); 

			memcpy(dstptr, with, withSize); // copy with size into dstptr
			dstptr += withSize;
			srcptr += replaceSize;
		}
		searchptr = strstr(srcptr, replace); //get next instance of replace
		copySize = getPositiveMin(searchptr - srcptr, dstmax - dstptr);
		
	}
	if(searchptr == NULL && srcmax > srcptr)
	{
		copySize = getPositiveMin(srcmax - srcptr, dstmax - dstptr);
		memcpy(dstptr, srcptr, copySize); // copy original string tail to destination string
		dstptr += copySize;
		srcptr += copySize;
	}
	*dstptr = '\0'; // null termination
	strcpy_s(string, bufsize, tmpString); //move buffer back into original string
	free(tmpString);
}

/**
* Function: printOutputLine
* Description:
*  - Print the specified line to the output file, while also taking care of any
*    pending labels that need to be included in expanded macro lines. Also takes
*    care of any post-line processing, such as unique label generation.
* Parameters:
*  - outputFile - FILE pointer to output file.
*  - line - Pointer to line of code that will be printed.
* Returns:
*  - none
*/
void printOutputLine(FILE * outputFile, const char * line)
{
    parse_info_t * parseInfo = NULL;
    char tmpLine[CURRENT_LINE_SIZE];
    char uniquePrefix[UNIQUE_LABEL_DIGITS + 2];
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
        else // just print the line
        {
            // unique label generation
            strcpy_s(tmpLine, sizeof(tmpLine), line);
            memset(uniquePrefix, 0, sizeof(uniquePrefix));
            getUniquePrefix(UNIQUE_ID, uniquePrefix, sizeof(uniquePrefix));
			strReplace(tmpLine, sizeof(tmpLine), "$", uniquePrefix, FALSE);
            fprintf(outputFile, "%s\n", tmpLine);
        }
        parse_info_free(parseInfo);
    }
}

/**
* Function: getUniquePrefix
* Description:
*  - Given an integer representing a unique ID, returns the corresponding
*    unique label prefix, a two-letter identifier.
* Parameters:
*  - id: Unique identifier in decimal form.
*  - prefix: Pointer to string buffer for result
*  - bufferSize: Size of the string buffer
* Returns:
*  - none
*/
void getUniquePrefix(int id, char * prefix, size_t bufferSize)
{
    char digit1 = 'A';
    char digit2 = 'A';
    if(id >= 0 && id < MAX_UNIQUE_LABELS && prefix != NULL)
    {
        digit1 += (id / 26);
        digit2 += (id % 26);
        sprintf_s(prefix, bufferSize, "$%c%c", digit1, digit2);
    }
}

/**
 * Function: spitKeyValuePair
 * Description:
 *  - Given a string in the format of "{x}={y}", splits into two tokens, where
 *    {x} is the key and {y} is the value.
 * Parameters:
 *  - string: String containing key/value pair.
 *  - key: Pointer to string buffer for key.
 *  - keysize: Size of the string buffer for key.
 *  - value: Pointer to string buffer for value.
 *  - valuesize: Size of the string buffer for value.
 * Returns:
 *  - none
 */
void splitKeyValuePair(const char * string, char * key, size_t keysize, char * value, size_t valuesize)
{
    char * tmp;
    char * tmp2;
    char * token;
    char * nextToken;
    const char delimiters[] = "=";

    if(string != NULL && key != NULL && value != NULL)
    {
        tmp = _strdup(string);
        token = strtok_s(tmp, delimiters, &nextToken);
        if(token != NULL)
        {
            if(*token != '&')
            {
                // key doesn't start w/ &, so prepend it
                tmp2 = _strdup(token);
                sprintf_s(key, keysize, "&%s", tmp2);
                free(tmp2);
            }
            else
            {
                strcpy_s(key, keysize, token);
            }
            token = strtok_s(NULL, delimiters, &nextToken);
            if(token != NULL)
            {
                strcpy_s(value, valuesize, token);
            }
            else
            {
                // value is blank
                strcpy_s(value, valuesize, "");
            }
        }
        free(tmp);
    }
}


/**
 * Function: arrayValueForIndex
 * Description:
 *  - Given a string in the format of "val1,val2,val3", returns val depending on the index
 *  - NOTE: Index starts at 1, not 0!
 * Parameters:
 *  - stringArray: String containing array in format of val1,val2,val3
 *  - arrayVal: buffer to put result in
 *  - index: number in string format
 *  
 * Returns:
 *  - SUCCESS if array replacement succeeds
 *  - FAILURE if index is out of bounds or other error
 */
int arrayValueForIndex(const char *stringArray, char *arrayVal, char *index)
{
	char *value = NULL;
	char *nextToken = NULL;
	int indexAsInt = atoi(index);
	char *inputArray = _strdup(stringArray);
	int n = 0;

	if(indexAsInt == 0)
		return FAILURE;

	
	value = strtok_s(inputArray, ", ", &nextToken);
	for(n=1; n<indexAsInt; n++)
	{
		value = strtok_s(NULL, ", ", &nextToken);
	}

	strcpy_s(arrayVal, ARGTAB_STRING_SIZE, value);
	
	free(inputArray);

	return SUCCESS;
}