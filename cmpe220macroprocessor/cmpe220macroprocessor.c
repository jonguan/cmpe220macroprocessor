// cmpe220macroprocessor.c : Defines the entry point for the console application.
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "definitions.h"
#include "common.h"
#include "argtab.h"
#include "deftab.h"
#include "namtab.h"

char* getline(char *inputFileName);
int processLine(char* inputLine, char* inputFileName, char* outputFileName);
int define(char *inputFileName);
int expand(char *inputFileName, char *outputFileName);

// for debug
#define TESTBIN_MODE	(FALSE)
void debug_testDataStructures(void);


static BOOL EXPANDING; 
static char* OPCODE;

int main(int argc, char* argv[])
{
	char *inputFileName;
	char *outputFileName;
	int result;

	printf("beginning cmpe220 macroprocessor");

	// Ugly, but effective unit testing method
	// Note that the VS Unit Test Framework only works for C++
	// IMPORTANT! - Set TESTBIN_MODE to FALSE for production release
	if(TESTBIN_MODE)
	{
		debug_testDataStructures();
		return 0;
	}

	// CHECK INPUT PARAMS
	//Example input: macroProcessor -options fileInput outputFile
	//argc = 3
	//argv[0] = macroProcessor; argv[1] = -options; argv[2] = fileInput; argv[3] = outputFile
	//////////////////////////////////////////////////////////////////////////////////////////
	if(argc < 2 || argc > 4)
	{
		printf("Error in syntax. Example input:macroProcessor -options fileInput");
		return 1;
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
	if(strlen(inputLine) < KOpFlagSymStart)
	{
		return result;
	}

	// Get OPCODE (strtok)
	strncpy_s(opCode, (KOpFlagSymStart - kOpCodeStart), inputLine + kOpCodeStart, _TRUNCATE);

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

	// Error check
	if (inputFile == NULL) {
		fprintf(stderr, "Can't open input file in.list!\n");
		return NULL;
	}

	fclose(inputFile);

	return NULL;
}

int define(char *inputFileName)
{
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

	printf("Press any ENTER to exit...");
	getchar();
}