/**
 * test.c - Contains definitions for unit test functions.
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

/**
 * Function: runTests
 * Description:
 *  - Runs the unit tests.
 * Parameters:
 *  - none
 * Returns:
 *  - none
 */
void runTests(void)
{
    debug_testDataStructures();
    debug_testParser();
    debug_testUniqueLabelGenerator();
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
    argtab_add(argtab, "ONE", "ARGUMENT_ONE");
    argtab_add(argtab, "TWO", "ARGUMENT_TWO");
    argtab_add(argtab, "THREE", "ARGUMENT_THREE");
    printf("%s: Testing with invalid values\n", __func__);
    argtab_add(NULL, "ONE", "Oops!");
    argtab_add(argtab, NULL, "Another oops!");
    argtab_add(argtab, "ONE1", "minimum value");
    string = argtab_get(argtab, "TWO");
    printf("%s: Getting argument %s = '%s'\n", __func__, "TWO", string);

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
}

void debug_testUniqueLabelGenerator(void)
{
    int i = 0;
    char prefix[UNIQUE_LABEL_DIGITS + 2];
    printf("\n%s: START UNIQUE LABEL GENERATOR TESTS\n\n", __func__);
    for(i = 0; i < MAX_UNIQUE_LABELS; i++)
    {
        getUniquePrefix(i, prefix, sizeof(prefix));
        printf("id=%d, prefix=%s\n", i, prefix);
    }
}