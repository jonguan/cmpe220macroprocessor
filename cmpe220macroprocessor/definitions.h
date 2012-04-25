/*
 * definitions.h - Contains global definitions.
 */

#ifndef DEFINITIONS_H_
#define DEFINITIONS_H_

#include "deftab.h"
#include "namtab.h"
#include "argtab.h"

// For those used to GCC.. :-)
#define __func__ __FUNCTION__
#define BOOL unsigned char


// Basic Constants
#define TRUE 1
#define FALSE 0
#define SUCCESS 0
#define FAILURE -1

// Function Definitions
////////////////////////////////////////////////////////////////////////////////////////
char* getline(FILE *inputFile);
int processLine(FILE * inputFile, FILE* outputFile, const char * macroLine);
int define(FILE * inputFile, FILE * outputFile, const char * macroLine);
int expand(FILE *inputFile, FILE *outputFile, const char *macroName);
void printUsage(void);
int getPositiveMin(int a, int b);
void strReplace(char * string, size_t bufsize, const char * replace, const char * with);
int parseInputCommand(char **inputFileName, char **outputFileName, int argc, char * argv[]);

// Declare global variables
///////////////////////////////////////////////////////////////////////////////////////////////

// Verbose flag - prints line numbers to output file and debug information to console
extern BOOL VERBOSE;

// Expanding flag - for function expand
extern BOOL EXPANDING; 

// OPCODE pointer - to determine what the opcode currently is
extern char* OPCODE;

// Pointer to current index of definitions table
extern int deftabIndex;

// Pointer to current line of input file
extern char currentLine[256];

// Pointers to table structures
extern deftab_t * deftab;
extern namtab_t * namtab;
extern argtab_t * argtab;



// Input file parameter description]
#define kOpCodeStart 10
#define kOpFlagSymStart 18
#define kOperandStart 19

#endif // DEFINITIONS_H_