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

char* getline(char *inputFileName);
int processLine(char* inputLine, char* inputFileName, char* outputFileName);
int define(FILE * inputFile, FILE * outputFile, const char * macroLine);
int expand(char *inputFileName, char *outputFileName);
void printUsage(void);

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
        //TODO call define
        //result = define(inputFileName);
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
    int index = 0;
    int level = 1;
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

    while(level > 0)
    {
        //TODO CALL GETLINE
        currentLine = dummyLines[currentLineIndex++];
        parse_info_clear(parse_info);
        if(parse_line(parse_info, currentLine) != 0)
        {
            parse_info_free(parse_info);
            return FAILURE;
        }

        if(parse_info->isComment == FALSE)
        {
            //TODO substitute positional notation for parameters
            index = deftab_add(deftab, currentLine);
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
    parse_info_free(parse_info);
    return SUCCESS;
}

int expand(char *inputFileName, char *outputFileName)
{
    return SUCCESS;
}
