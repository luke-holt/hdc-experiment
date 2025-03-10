#include <stdbool.h>
#include <string.h>

#include "test.h"

#include "hdvector.h"

#include "util.h"

// hdvector_init_random(HDVector *vector, size_t count);
// hdvector_shift(HDVector *vector, size_t shift);
// hdvector_distance(HDVector *a, HDVector *b);

void
test_hdvector_copy(void)
{
    HDVector vector, copy;
    float distance;

    hdvector_init_random(&vector, 1);
    hdvector_copy(&copy, &vector);

    test_verify(memcmp(&copy, &vector, sizeof(vector)) == 0, "copy");
}

void
test_hdvector_mult(void)
{
    HDVector a, b, c;
    hdvector_init_random(&a, 1);
    hdvector_init_random(&b, 1);

    hdvector_mult(&a, &b, &c);

    size_t idx;
    idx = 0;
    test_verify((a.data[idx] ^ b.data[idx]) == c.data[idx], "first word mult");
    idx = ARRLEN(a.data) - 1;
    test_verify((a.data[idx] ^ b.data[idx]) == c.data[idx], "last word mult");
}

void
test_hdvector_load_and_store(void)
{
    HDVector profile, table[256];
    HDVector ploaded, tloaded[ARRLEN(table)];
    bool rc;

    const char *filename = "test_hdvector_load_and_store.hdv.tmp";

    hdvector_init_random(&profile, 1);
    hdvector_init_random(table, ARRLEN(table));

    rc = hdvector_store_to_file(filename, &profile, table, ARRLEN(table));
    test_verify(rc, "store");

    rc = hdvector_load_from_file(filename, &ploaded, tloaded, ARRLEN(tloaded));
    test_verify(rc, "load");

    test_verify(memcmp(&profile, &ploaded, sizeof(profile)) == 0, "profile integrity");
    test_verify(memcmp(table, tloaded, sizeof(table)) == 0, "table integrity");

    remove(filename);
}

void
test_hdvector_distance(void)
{
    HDVector a, b;
    hdvector_init_random(&a, 1);
    hdvector_init_random(&b, 1);

    float distance;
    distance = hdvector_distance(&a, &a);
    test_verify(distance == 0.0, "distance to same vector is 0.00");
    distance = hdvector_distance(&a, &b);
    test_verify(distance > 0.48 && distance < 0.52, "distance to random vector ~0.50");
}
