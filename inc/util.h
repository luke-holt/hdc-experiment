#ifndef UTIL_H
#define UTIL_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>


#define ARRLEN(arr) (sizeof(arr)/sizeof(*arr))

#define UTIL_ASSERT(cond) if (!(cond)) { util_log("ASSERT", "failed assertion"); abort(); }
#define TODO(msg) util_log("TODO", "%s:%s:%d %s", __FILE__, __func__, __LINE__, msg)

#define XORSHIFT128_RAND_MAX ((uint32_t)0xFFFFFFFFu)
uint32_t xorshift128(uint32_t *state);

uint32_t bitcount(uint32_t word);

void util_log(const char *tag, const char *fmt, ...);

bool util_file_exists(const char *filename);


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

#endif
