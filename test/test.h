#ifndef TEST_H
#define TEST_H

#include <stdbool.h>

#define test_verify(cond, msg) test_verify_((cond), __func__, (msg))

void test_verify_(bool c, const char *func, const char *msg, ...);

#endif // TEST_H
