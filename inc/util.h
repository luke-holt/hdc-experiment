#ifndef UTIL_H
#define UTIL_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"


#define STREQ(a, b) (0 == strcmp((a), (b)))

#define ARRLEN(arr) (sizeof(arr)/sizeof(*arr))

#define UTIL_ASSERT(cond) if (!(cond)) { util_log("ASSERT", "%s:%s:%d assertion failed", __FILE__, __func__, __LINE__); abort(); }
#define TODO(msg) util_log("TODO", "%s:%s:%d %s", __FILE__, __func__, __LINE__, msg)

#define XORSHIFT128_RAND_MAX ((uint32_t)0xFFFFFFFFu)
uint32_t xorshift128(uint32_t *state);

uint32_t bitcount(uint32_t word);

void util_log(const char *tag, const char *fmt, ...);

bool util_file_exists(const char *filename);

typedef struct { size_t size; char *data; } String;
bool string_load_from_file(const char *filename, String *str);
void string_delete(String *str);

#endif // UTIL_H


#ifdef UTIL_IMPLEMENTATION

#include <stdarg.h>
#include <unistd.h>

bool util_file_exists(const char *filename) {
    return access(filename, F_OK) == 0;
}

void util_log(const char *tag, const char *fmt, ...) {
    fprintf(stderr, "[%s] ", tag);
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fprintf(stderr, "\n");
}

uint32_t xorshift128(uint32_t *state) {
    uint32_t t, s;
    t = state[3];
    s = state[0];
    state[3] = state[2];
    state[2] = state[1];
    state[1] = s;
    t ^= t << 11;
    t ^= t >> 8;
    state[0] = t ^ s ^ (s >> 19);
    return state[0];
}

uint32_t bitcount(uint32_t word) {
    word = word - ((word >> 1) & 0x55555555);
    word = (word & 0x33333333) + ((word >> 2) & 0x33333333);
    word = ((word + (word >> 4) & 0x0F0F0F0F) * 0x01010101) >> 24;
    return word;
}

bool string_load_from_file(const char *filename, String *str) {
    UTIL_ASSERT(filename);
    UTIL_ASSERT(str);

    FILE *file;
    int rc;
    bool success = false;

    file = fopen(filename, "r");
    if (!file) return false;

    rc = fseek(file, 0, SEEK_END);
    if (rc) goto defer;

    str->size = ftell(file);

    rc = fseek(file, 0, SEEK_SET);
    if (rc) goto defer;

    str->data = malloc(str->size);
    UTIL_ASSERT(str->data);

    rc = fread(str->data, 1, str->size, file);
    if (rc != str->size) goto defer;

    success = true;

defer:
    (void)fclose(file);
    return success;
}

void string_delete(String *str) {
    UTIL_ASSERT(str);
    UTIL_ASSERT(str->data);
    free(str->data);
    str->size = 0;
    str->data = NULL;
}

#endif
