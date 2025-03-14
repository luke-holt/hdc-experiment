#include <stdbool.h>
#include <stdio.h>
#include <stdarg.h>

#include "test.h"

#define UTIL_IMPLEMENTATION
#include "util.h"

// hdvector
extern void test_hdvector_mult(void);
extern void test_hdvector_copy(void);
extern void test_hdvector_load_and_store(void);
extern void test_hdvector_distance(void);


int test_counter = 0;
int pass_counter = 0;

int
main(void)
{
    test_hdvector_copy();
    test_hdvector_mult();
    test_hdvector_load_and_store();
    test_hdvector_distance();

    fprintf(stdout, "[TEST] Passed %d/%d tests\n", pass_counter, test_counter);

    return 0;
}

void
test_verify_(bool c, const char *func, const char *msg, ...)
{
    test_counter++;
    pass_counter += c;

    fprintf(stdout, "[TEST] ");
    fprintf(stdout, "%s ", c?ANSI_COLOR_GREEN"PASS"ANSI_COLOR_RESET:ANSI_COLOR_RED"FAIL"ANSI_COLOR_RESET);
    fprintf(stdout, "%s ", func);

    va_list args;
    va_start(args, msg);
    vfprintf(stdout, msg, args);
    va_end(args);

    fputc('\n', stdout);
}
