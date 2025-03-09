#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#define UTIL_IMPLEMENTATION
#include "util.h"

#include "hdvector.h"

typedef struct {
    size_t size;
    char *data;
} String;

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
}


struct idx_prob_map {
    size_t index;
    float probability; 
};
int idx_prob_map_cmp_desc(const void *a, const void *b) {
    float pa = ((struct idx_prob_map *)a)->probability;
    float pb = ((struct idx_prob_map *)b)->probability;
    if (pa > pb) return -1;
    if (pa < pb) return 1;
    return 0;
}

void
print_most_likely_next_symbols(HDVector *symbols, size_t symbol_count, HDVector *query, size_t count)
{
    UTIL_ASSERT(symbols);
    UTIL_ASSERT(query);

    struct idx_prob_map pmap[symbol_count];
    memset(pmap, 0, sizeof(pmap));

    for (size_t i = 0; i < symbol_count; i++) {
        pmap[i].index = i;
        pmap[i].probability = hdvector_distance(&symbols[i], query);
    }

    qsort(pmap, symbol_count, sizeof(*pmap), idx_prob_map_cmp_desc);

    for (size_t i = 0; i < count; i++) {
        printf("%02zX '%c' %.04f\n",
               pmap[i].index,
               isprint(pmap[i].index)?(char)pmap[i].index:'_',
               pmap[i].probability);
    }
}


void
profile_vector_add(uint32_t *sum, HDVector *vector)
{
    UTIL_ASSERT(sum);
    UTIL_ASSERT(vector);
    for (size_t i = 0; i < ARRLEN(vector->data); i++) {
        for (size_t bit = 0; bit < BITS_IN_U64; bit++) {
            sum[i*BITS_IN_U64+bit] += (vector->data[i] >> bit) & 1;
        }
    }
}

void
compute_new_profile(const char *filename, HDVector *profile, HDVector *symbols, size_t count)
{
    UTIL_ASSERT(filename);
    UTIL_ASSERT(profile);
    UTIL_ASSERT(symbols);
    UTIL_ASSERT(count == 256);

    uint32_t sumvector[DIMENSIONS] = {0};
    String content;

    if (!string_load_from_file(filename, &content)) {
        util_log("FATAL", "failed to open %s", filename);
        exit(1);
    }

    hdvector_init_random(symbols, count);

    for (size_t i = 0; i < content.size - 2; i++) {
        char trigram[3] = { content.data[i], content.data[i+1], content.data[i+2] };

        HDVector hdv[3];
        hdvector_copy(&hdv[0], &symbols[trigram[0]]);
        hdvector_copy(&hdv[1], &symbols[trigram[1]]);
        hdvector_copy(&hdv[2], &symbols[trigram[2]]);

        hdvector_shift(&hdv[0], 2);
        hdvector_shift(&hdv[1], 1);

        hdvector_mult(&hdv[0], &hdv[1], &hdv[0]);
        hdvector_mult(&hdv[0], &hdv[2], &hdv[0]);

        profile_vector_add(sumvector, &hdv[0]);
    }

    memset(profile, 0, sizeof(*profile));

    for (size_t i = 0; i < DIMENSIONS; i++) {
        profile->data[i/BITS_IN_U64] |= (sumvector[i] > ((content.size-2)/2) << (i&(BITS_IN_U64-1)));
    }

    string_delete(&content);
}

void
help(const char *prog)
{
    util_log("INFO", "Usage: %s [-n textfile] profile", prog);
    exit(0);
}

int
main(int argc, char *argv[])
{
    const char *profile_filename;
    const char *text_filename;

    if (argc != 2 && argc != 4) {
        help(argv[0]);
    }
    else if (argc == 4) {
        if (strcmp("-n", argv[1]) != 0) {
            util_log("FATAL", "unknown option '%s'", argv[1]);
            help(argv[0]);
        }
        if (!util_file_exists(argv[2])) {
            util_log("FATAL", "could not access file '%s'", argv[2]);
            help(argv[0]);
        }
        if (util_file_exists(argv[3])) {
            util_log("FATAL", "found existing profile %s", argv[3]);
            help(argv[0]);
        }
        text_filename = argv[2];
        profile_filename = argv[3];
    }
    else if (argc == 2) {
        if (!util_file_exists(argv[1])) {
            util_log("FATAL", "could not access file '%s'", argv[1]);
            help(argv[0]);
        }
        text_filename = NULL;
        profile_filename = argv[1];
    }

    HDVector profile;
    HDVector symbol_table[256];

    if (text_filename) {
        compute_new_profile(text_filename, &profile, symbol_table, ARRLEN(symbol_table));
        hdvector_store_to_file(profile_filename, &profile, symbol_table, ARRLEN(symbol_table));
    }
    else {
        if (!hdvector_load_from_file(profile_filename, &profile, symbol_table, ARRLEN(symbol_table))) {
            util_log("FATAL", "could not load profile '%s'", profile_filename);
            exit(1);
        }
    }

    // get input, show probability of output

    char input[16];

    for (;;) {

        memset(input, 0, sizeof(input));

        if (!fgets(input, sizeof(input), stdin)) break;
        if (!strcmp(input, "q")) break;

        uint8_t c0 = (uint8_t)input[0];
        uint8_t c1 = (uint8_t)input[1];

        HDVector hdv0, hdv1, result;
        hdvector_copy(&hdv0, &symbol_table[c0]);
        hdvector_copy(&hdv1, &symbol_table[c1]);
        hdvector_shift(&hdv0, 2);
        hdvector_shift(&hdv1, 1);
        hdvector_mult(&hdv0, &hdv1, &result);
        hdvector_mult(&profile, &result, &result);

        print_most_likely_next_symbols(symbol_table, ARRLEN(symbol_table), &result, 5);
    }

    return 0;
}

