#ifndef HDVECTOR_H
#define HDVECTOR_H

#include <stdint.h>

#define DIMENSIONS (2 << 12)
#define BITS_IN_U64 (8 * sizeof(uint64_t))
#define HDV_U64_LEN (DIMENSIONS / BITS_IN_U64)

typedef struct { uint64_t data[HDV_U64_LEN]; } HDVector;

void hdvector_init_random(HDVector *vector, size_t count);
void hdvector_shift(HDVector *vector, size_t shift);
void hdvector_copy(HDVector *dst, const HDVector *src);
void hdvector_mult(HDVector *a, HDVector *b, HDVector *out);
bool hdvector_load_from_file(const char *filename, HDVector *profile, HDVector *symbols, size_t count);
bool hdvector_store_to_file(const char *filename, HDVector *profile, HDVector *symbols, size_t count);

#endif // HDVECTOR_H
