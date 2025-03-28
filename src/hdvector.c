#include <stdbool.h>
#include <string.h>

#include "hdvector.h"

#include "util.h"

uint32_t rng_state[4] = {
    0xECED57FC, 0xA8A913B8,
    0x646574DF, 0x2021309B,
};

#define HDVECTOR_FILE_MAGIC_NUMBER (0x48445653) // 'H' 'D' 'V' 'S'

void
hdvector_init_random(HDVector *vector, size_t count)
{
    UTIL_ASSERT(vector);
    for (size_t v = 0; v < count; v++) {
        for (size_t i = 0; i < ARRLEN(vector->data); i++) {
            uint64_t msw = xorshift128(rng_state);
            uint64_t lsw = xorshift128(rng_state);
            vector[v].data[i] = msw << 32 | lsw;
        }
    }
}

void
hdvector_shift(HDVector *vector, size_t shift)
{
    UTIL_ASSERT(vector);
    for (size_t i = 0; i < ARRLEN(vector->data); i++)
        vector->data[i] = (vector->data[i] << shift) | (vector->data[i] >> (BITS_IN_U64 - shift));
}

void
hdvector_copy(HDVector *dst, const HDVector *src)
{
    UTIL_ASSERT(dst);
    UTIL_ASSERT(src);
    memcpy(dst, src, sizeof(*dst));
}

void
hdvector_mult(HDVector *a, HDVector *b, HDVector *out)
{
    UTIL_ASSERT(a);
    UTIL_ASSERT(b);
    UTIL_ASSERT(out);
    for (size_t i = 0; i < ARRLEN(out->data); i++)
        out->data[i] = a->data[i] ^ b->data[i];
}

bool
hdvector_load_from_file(const char *filename, HDVector *profile, HDVector *symbols, size_t count)
{
    UTIL_ASSERT(filename);
    UTIL_ASSERT(profile);
    UTIL_ASSERT(symbols);

    FILE *file;
    int rc;
    size_t n;
    uint32_t magic_num;
    bool success = false;

    file = fopen(filename, "rb");
    if (!file) return false;

    n = fread(&magic_num, sizeof(magic_num), 1, file);
    if (n != 1) goto defer;
    if (magic_num != HDVECTOR_FILE_MAGIC_NUMBER) {
        util_log("WARN", "%s: invalid HDVector file", __func__);
        goto defer;
    }

    n = fread(profile, sizeof(*profile), 1, file);
    if (n != 1) goto defer;

    n = fread(symbols, sizeof(*symbols), count, file);
    if (n != count) goto defer;

    success = true;

defer:
    (void)fclose(file);
    return success;
}

bool
hdvector_store_to_file(const char *filename, HDVector *profile, HDVector *symbols, size_t count)
{
    UTIL_ASSERT(filename);
    UTIL_ASSERT(profile);
    UTIL_ASSERT(symbols);

    FILE *file;
    int rc;
    size_t n;
    uint32_t magic_num;
    bool success = false;

    file = fopen(filename, "wb");
    if (!file) return false;

    magic_num = HDVECTOR_FILE_MAGIC_NUMBER;
    n = fwrite(&magic_num, sizeof(magic_num), 1, file);
    if (n != 1) goto defer;

    n = fwrite(profile, sizeof(*profile), 1, file);
    if (n != 1) goto defer;

    n = fwrite(symbols, sizeof(*symbols), count, file);
    if (n != count) goto defer;

    success = true;

defer:
    (void)fclose(file);
    return success;
}

float
hdvector_distance(HDVector *a, HDVector *b)
{
    UTIL_ASSERT(a);
    UTIL_ASSERT(b);
    size_t diff = 0;
    for (size_t i = 0; i < ARRLEN(a->data); i++) {
        uint64_t word = a->data[i] ^ b->data[i];
        uint32_t most = (word >> 32) & 0xFFFFFFFF;
        uint32_t least = word & 0xFFFFFFFF;
        diff += bitcount(most) + bitcount(least);
    }
    return (float)diff / (float)DIMENSIONS;
}

void
hdvector_form_query(HDVector *symbols, size_t scount, size_t *indices, size_t icount, HDVector *query)
{
    UTIL_ASSERT(symbols && scount);
    UTIL_ASSERT(indices && icount);
    UTIL_ASSERT(query);
    HDVector tmp, q = {0};
    for (size_t i = 0; i < icount; i++) {
        hdvector_copy(&tmp, &symbols[indices[i]]);
        hdvector_shift(&tmp, icount-i);
        hdvector_mult(&tmp, &q, &q);
    }
    *query = q;
}

