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

// Constants
#define CURRENT_LINE_SIZE   (256)
#define SHORT_STRING_SIZE   (16)
#define UNIQUE_LABEL_DIGITS (2)
#define MAX_UNIQUE_LABELS   (26 * 26)
#define MAX_NESTED_IF_SIZE  (20)

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
void printOutputLine(FILE * outputFile, const char * line);
void getUniquePrefix(int id, char * prefix, size_t bufferSize);

// Declare global variables
///////////////////////////////////////////////////////////////////////////////////////////////

// Verbose flag - prints line numbers to output file and debug information to console
extern BOOL VERBOSE;

// Expanding flag - for function expand
extern BOOL EXPANDING; 

// OPCODE - to determine what the opcode currently is
extern char OPCODE[SHORT_STRING_SIZE];

// Expanded Label - to keep track of labels included with macro invocations
extern BOOL EXPAND_LABEL;
extern char EXPANDED_LABEL[SHORT_STRING_SIZE];

// Unique ID - Used to identify a macro invocation, for unique label generation
extern int  UNIQUE_ID;

// Pointer to current index of definitions table
extern int deftabIndex;

// Pointer to current line of input file
extern char currentLine[CURRENT_LINE_SIZE];

// If Statement level - For conditional macro expansion
extern int IFSTATEMENTLEVEL;

// Pointers to table structures
extern deftab_t * deftab;
extern namtab_t * namtab;
extern argtab_t * argtab;



// Input file parameter description]
#define kOpCodeStart 10
#define kOpFlagSymStart 18
#define kOperandStart 19

#endif // DEFINITIONS_H_