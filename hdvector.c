#include <string.h>

#include "hdvector.h"

#include "util.h"

#define ARRLEN(arr) (sizeof(arr)/sizeof(*arr))

uint32_t rng_state[4] = {
    0xECED57FC, 0xA8A913B8,
    0x646574DF, 0x2021309B,
};


void
hdvector_init_random(HDVector *vector)
{
    UTIL_ASSERT(vector);
    for (size_t i = 0; i < ARRLEN(vector->data); i++) {
        uint64_t msw = xorshift128(rng_state);
        uint64_t lsw = xorshift128(rng_state);
        vector->data[i] = msw << 32 | lsw;
    }
}

void
hdvector_shift(HDVector *vector, size_t shift)
{
    UTIL_ASSERT(vector);
    for (size_t i = 0; i < ARRLEN(vector->data); i++) {
        vector->data[i] = (vector->data[i] << shift) | (vector->data[i] >> (BITS_IN_U64 - shift));
    }
}

void
hdvector_copy(HDVector *dst, const HDVector *src)
{
    UTIL_ASSERT(dst);
    UTIL_ASSERT(src);
    memcpy(dst, src, sizeof(*dst));
}
