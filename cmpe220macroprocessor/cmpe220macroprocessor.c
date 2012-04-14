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

char* getline(char *inputFileName);
int processLine(char* inputLine, char* inputFileName, char* outputFileName);
int define(FILE * inputFile, FILE * outputFile, const char * macroLine);
int expand(char *inputFileName, char *outputFileName);
void printUsage(void);

// for development - unit testing
void debug_testDataStructures(void);
void debug_testParser(void);

// global variables
static BOOL EXPANDING; 
static char* OPCODE;
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

int main(int argc, char* argv[])
{
    char *inputFileName = NULL;
    char *outputFileName = NULL;
    int result;
    int i;
    BOOL verbose = FALSE;

    /** HANDLE ARGUMENTS **/

    printf("beginning cmpe220 macroprocessor\n");
    if(argc <= 1)
    {
        printUsage();
        return 0;
    }

    // these options can only be used by themselves
    if(strcmp("-?", argv[1]) == 0)
    {
        printUsage();
        return 0;
    }
    if(strcmp("-t", argv[1]) == 0)
    {
        debug_testDataStructures();
        debug_testParser();
        return 0;
    }

    // the rest of the options may appear in any order
    for(i = 1; i < argc; i++)
    {
        if(strcmp("-v", argv[i]) == 0)
        {
            verbose = TRUE;
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
                return -1;
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
                return -1;
            }
        }
        else // unrecognized options
        {
            printUsage();
            return -1;
        }
    }

    // make sure we have input and output files defined
    if(inputFileName == NULL || outputFileName == NULL)
    {
        printUsage();
        return -1;
    }

    // File I/O
    ////////////////////////////////////////////////////////////////////////////////////////////

    // Output file is last (argc-1);
    // input file is 2nd last (argc-2)
    inputFileName = argv[argc-2];
    // Output file name
    // don't need to open here - do in subroutines
    outputFileName = argv[argc-1];

    // MACROPROCESSOR LOOP
    ///////////////////////////////////////////////////////////////////
    EXPANDING = FALSE;

    while(strcmp(OPCODE, "END"))
    {
        char *line = getline(inputFileName);

        // Error check
        if (line == NULL)
            return SUCCESS;

        result = processLine(line, inputFileName, outputFileName);

        if (result != SUCCESS)
        {
            printf("ERROR in processLine");
            return 1;
        }

    }

    return SUCCESS;
}

int processLine(char* inputLine, char* inputFileName, char* outputFileName)
{
    char opCode[8];
    int result = 1;
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
        //expand
        result = expand(inputFileName, outputFileName);
    }
    else if (strcmp(opCode, "MACRO"))
    {
        //define
        result = define(inputFileName);
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
            return 1;
        }

        // write line out
        fprintf(outputFile, "inputLine");

        // close file
        fclose(outputFile);
    }

    return result;
}

char* getline(char *inputFileName)
{
    FILE *inputFile;
    errno_t rc = fopen_s(&inputFile, inputFileName, "r");
	char * line;

    // Error check
    if (inputFile == NULL) {
        fprintf(stderr, "Can't open input file in.list!\n");
        return NULL;
    }

	if(EXPANDING)
	{
	    // get next line of macro definition from DEFTAB
		// substitute arguments from ARGTAB for positional notation
		line = deftab_get(deftab, 1); // the 1 is hardcoded right now; need to figure out how to pass in macroline
	}
	else
	{
		// read next line from input file;
		fgets(line,255,inputFile);
	}

    fclose(inputFile);

    return line;
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
	/*
	 * Algorithm from the textbook:
	 *
	 *	enter macro name into NAMTAB
	 *	enter macro prototype into DEFTAB
	 *	LEVEL = 1
	 *	while(LEVEL > 0)
	 *	{
	 *		GETLINE
	 *		if(this is not a comment line)
	 *		{
	 *			substitute positional notation for parameters
	 *			enter line into DEFTAB
	 *			if(OPCODE = 'MACRO')
	 *			{
	 *				LEVEL++
	 *			}
	 *			else if(OPCODE = 'MEND')
	 *			{
	 *				LEVEL--
	 *			}
	 *		} // if not comment
	 *	} // while
	 *	store in NAMTAB pointers to beginning and end of definition
	 */

	parse_info_t * parse_info;
	int startIndex;
	int endIndex;

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

	//TODO finish this

    return SUCCESS;
}

int expand(char *inputFileName, char *outputFileName)
{
    return SUCCESS;
}

void debug_testDataStructures(void)
{
    deftab_t * deftab;
    argtab_t * argtab;
    namtab_t * namtab;
    namtab_entry_t * namtabEntry;
    char * string;
    int start = 0;
    int end = 0;
    int i;

    /* DEFTAB TESTS */
    printf("\n%s: START DEFTAB TESTS\n\n", __func__);
    deftab = deftab_alloc();
    start = deftab_add(deftab, "Hello, World!");
    deftab_add(deftab, "This is a line");
    deftab_add(deftab, "This is another line, just testing out different size lengths!");
    deftab_add(deftab, "Yet another string!");
    end = deftab_add(deftab, "And another one");
    printf("%s: start = %d, end = %d\n", __func__, start, end);
    start = deftab_add(deftab, "This is a different section");
    deftab_add(deftab, "Another line in this section!");
    deftab_add(deftab, "Yet another line in this section");
    end = deftab_add(deftab, "Finally, the last line in this section");
    printf("%s: start = %d, end = %d\n", __func__, start, end);
    printf("%s: testing with null pointers\n", __func__);
    deftab_add(NULL, "Oops!");
    deftab_add(deftab, NULL);
    string = deftab_get(deftab, 3);
    printf("%s: Getting line %d = '%s'\n", __func__, 3, string);

    /* ARGTAB TESTS */
    printf("\n%s: START ARGTAB TESTS\n\n", __func__);
    argtab = argtab_alloc();
    argtab_add(argtab, 1, "ARGUMENT_ONE");
    argtab_add(argtab, 2, "ARGUMENT_TWO");
    argtab_add(argtab, 3, "ARGUMENT_THREE");
    printf("%s: Testing with invalid values\n", __func__);
    argtab_add(NULL, 1, "Oops!");
    argtab_add(argtab, 0, "Another oops!");
    argtab_add(argtab, 1, "minimum value");
    argtab_add(argtab, ARGTAB_MAX_ARRAY_SIZE, "max value");
    argtab_add(argtab, ARGTAB_MAX_ARRAY_SIZE + 1, "out of bounds");
    argtab_add(argtab, -1, "out of bounds");
    string = argtab_get(argtab, 2);
    printf("%s: Getting argument %d = '%s'\n", __func__, 2, string);

    /* NAMTAB TESTS */
    printf("\n%s: START NAMTAB TESTS\n\n", __func__);
    namtab = namtab_alloc();
    namtab_add(namtab, "MACRO_ONE", 0, 4);
    namtab_add(namtab, "MACRO_TWO", 5, 8);
    printf("%s: Looking up MACRO_TWO:\n", __func__);
    namtabEntry = namtab_get(namtab, NULL);
    namtabEntry = namtab_get(namtab, "DOESNT_EXIST");
    namtabEntry = namtab_get(namtab, "MACRO_TWO");
    printf("    - symbol=%s, start=%d, end=%d\n", namtabEntry->symbol, namtabEntry->deftabStart, namtabEntry->deftabEnd);
    for(i = namtabEntry->deftabStart; i <= namtabEntry->deftabEnd; i++)
    {
        string = deftab_get(deftab, i);
        printf("    %s\n", string);
    }

    printf("\n%s: CLEAN-UP\n\n", __func__);
    namtab_free(namtab);
    argtab_free(argtab);
    deftab_free(deftab);
}

void debug_testParser(void)
{
    parse_info_t * parse_info = NULL;
    char buffer[128];

    printf("\n%s: START PARSER TESTS\n\n", __func__);

    printf("\nCase 1: Null pointer\n");
    parse_info_print(NULL);

    printf("\nCase 2: Macro definition\n");
    sprintf_s(buffer, sizeof(buffer), "WRBUFF    MACRO   &OUTDEV,&BUFADR,&RECLTH");
    printf("Buffer: %s\n", buffer);
    parse_info = parse_info_alloc();
    if(parse_info)
    {
        if(parse_line(parse_info, buffer) == 0)
        {
            parse_info_print(parse_info);
        }
        parse_info_free(parse_info);   
    }

    printf("\nCase 3: Comment line\n");
    sprintf_s(buffer, sizeof(buffer), ".         MACRO TO WRITE RECORD FROM BUFFER");
    printf("Buffer: %s\n", buffer);
    parse_info = parse_info_alloc();
    if(parse_info)
    {
        if(parse_line(parse_info, buffer) == 0)
        {
            parse_info_print(parse_info);
        }
        parse_info_free(parse_info);
    }

    printf("\nCase 4: No label, with trailing whitespace\n");
    sprintf_s(buffer, sizeof(buffer), "         +LDT    #4096          ");
    printf("Buffer: %s\n", buffer);
    parse_info = parse_info_alloc();
    if(parse_info)
    {
        if(parse_line(parse_info, buffer) == 0)
        {
            parse_info_print(parse_info);
        }
        parse_info_free(parse_info);
    }

    printf("Press any ENTER to exit...");
    getchar();
}