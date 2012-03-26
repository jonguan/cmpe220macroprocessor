// cmpe220macroprocessor.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

char* getline(char *inputFileName);
int processLine(char* inputLine, char* inputFileName, char* outputFileName);
int define(char *inputFileName);
int expand(char *inputFileName, char *outputFileName);


static bool EXPANDING; 
static char* OPCODE;

int main(int argc, char* argv[])
{
	printf("beginning cmpe220 macroprocessor");

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
	char *inputFileName = argv[argc-2];
	// Output file name
	// don't need to open here - do in subroutines
	char* outputFileName = argv[argc-1];

	// MACROPROCESSOR LOOP
	///////////////////////////////////////////////////////////////////
	EXPANDING = FALSE;

	while(strcmp(OPCODE, "END"))
	{
		char *line = getline(inputFileName);
		
		// Error check
		if (line == NULL)
			return SUCCESS;
		
		int result = processLine(line, inputFileName, outputFileName);

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
	int result = 1;

	// Error Check
	if(strlen(inputLine) < KOpFlagSymStart)
	{
		return result;
	}

	// Get OPCODE (strtok)
	char opCode[8];
	strncpy(opCode, inputLine + kOpCodeStart, (KOpFlagSymStart - kOpCodeStart));

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
		FILE *outputFile = fopen(outputFileName, "w");

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
	FILE *inputFile = fopen(inputFileName, "r");

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