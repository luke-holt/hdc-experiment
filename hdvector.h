#ifndef HDVECTOR_H
#define HDVECTOR_H

#include <stdint.h>

#define DIMENSIONS (2 << 12)
#define BITS_IN_U64 (8 * sizeof(uint64_t))
#define HDV_U64_LEN (DIMENSIONS / BITS_IN_U64)

typedef struct { uint64_t data[DIMENSIONS]; } HDVector;

void hdvector_init_random(HDVector *vector);
void hdvector_shift(HDVector *vector, size_t shift);
void hdvector_copy(HDVector *dst, const HDVector *src);

#endif // HDVECTOR_H
