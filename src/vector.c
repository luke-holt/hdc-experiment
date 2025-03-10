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

char
symbol_to_char(uint8_t s)
{
    return (s == 26) ? ' ' : (char)s + 'a';
}

uint8_t
char_to_symbol(char c)
{
    if (isalpha(c)) {
        c = (c >= 'a') ? c - ('a'-'A') : c;
    } else {
        c = ' ';
    }

    uint8_t sym;
    if (c == ' ') {
        sym = 26;
    } else {
        sym = (uint8_t)c - 'A';
    }

    return sym;
}


struct prob_map {
    size_t index;
    float probability; 
};
int prob_map_cmp_asc(const void *a, const void *b) {
    float pa = ((struct prob_map *)a)->probability;
    float pb = ((struct prob_map *)b)->probability;
    if (pa > pb) return 1;
    if (pa < pb) return -1;
    return 0;
}

void
print_most_likely_next_symbols(HDVector *symbols, size_t symbol_count, HDVector *query, size_t count)
{
    UTIL_ASSERT(symbols);
    UTIL_ASSERT(query);

    struct prob_map pmap[symbol_count];
    memset(pmap, 0, sizeof(pmap));

    for (size_t i = 0; i < symbol_count; i++) {
        pmap[i].index = i;
        pmap[i].probability = hdvector_distance(&symbols[i], query);
    }

    qsort(pmap, symbol_count, sizeof(*pmap), prob_map_cmp_asc);

    for (size_t i = 0; i < count; i++) {
        printf("%02zX '%c' %.04f\n",
               pmap[i].index,
               symbol_to_char(pmap[i].index),
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
tmp(void)
{
    HDVector v[11], pv;
    hdvector_init_random(v, ARRLEN(v));

    uint32_t profile[DIMENSIONS] = {0};

    for (size_t i = 0; i < ARRLEN(v); i++)
        profile_vector_add(profile, &v[i]);

    for (size_t i = 0; i < DIMENSIONS; i++)
        pv.data[i/BITS_IN_U64] |= ((profile[i] > (ARRLEN(v)/2)) << (i&(BITS_IN_U64-1)));

    printf("0.1 %f\n", hdvector_distance(&v[0], &v[1]));
    for (size_t i = 0; i < ARRLEN(v); i++)
        printf("%02zu.p %f\n", i, hdvector_distance(&v[i], &pv));
}

void
compute_new_profile(const char *filename, HDVector *profile, HDVector *symbols, size_t count)
{
    UTIL_ASSERT(filename);
    UTIL_ASSERT(profile);
    UTIL_ASSERT(symbols);
    UTIL_ASSERT(count == 27);

    uint32_t sumvector[DIMENSIONS] = {0};
    uint32_t skipped = 0;
    String content;

    if (!string_load_from_file(filename, &content)) {
        util_log("FATAL", "failed to open %s", filename);
        exit(1);
    }

    hdvector_init_random(symbols, count);

    for (size_t i = 0; i < content.size - 2; i++) {
        uint8_t a = char_to_symbol(content.data[i+0]);
        uint8_t b = char_to_symbol(content.data[i+1]);
        uint8_t c = char_to_symbol(content.data[i+2]);
        uint8_t space = char_to_symbol(' ');

        if ((a == space && b == space) || (b == space && c == space)) {
            skipped++;
            continue;
        }

        HDVector hdv[3];
        hdvector_copy(&hdv[0], &symbols[a]);
        hdvector_copy(&hdv[1], &symbols[b]);
        hdvector_copy(&hdv[2], &symbols[c]);

        hdvector_shift(&hdv[0], 2);
        hdvector_shift(&hdv[1], 1);

        hdvector_mult(&hdv[0], &hdv[1], &hdv[0]);
        hdvector_mult(&hdv[0], &hdv[2], &hdv[0]);

        profile_vector_add(sumvector, &hdv[0]);
    }

    memset(profile, 0, sizeof(*profile));

    for (size_t i = 0; i < DIMENSIONS; i++) {
        profile->data[i/BITS_IN_U64] |= ((sumvector[i] > ((content.size-2-skipped)/2)) << (i&(BITS_IN_U64-1)));
    }

    string_delete(&content);
}

void
digram_prob_map(HDVector *symbols, size_t count, HDVector *profile, struct prob_map *pmap, size_t a, size_t b)
{
    UTIL_ASSERT(symbols);
    UTIL_ASSERT(pmap);
    UTIL_ASSERT(profile);

    memset(pmap, 0, count*sizeof(*pmap));

    HDVector query;

    // form query
    size_t indices[2] = {a, b};
    hdvector_form_query(symbols, count, indices, ARRLEN(indices), &query);

    // perform query
    hdvector_mult(profile, &query, &query);

    // populate map
    for (size_t i = 0; i < count; i++) {
        pmap[i].index = i;
        pmap[i].probability = hdvector_distance(&symbols[i], &query);
    }

    // sort map
    qsort(pmap, count, sizeof(*pmap), prob_map_cmp_asc);
}

void
digram_table(HDVector *symbols, size_t count, HDVector *profile, float distance_threshold, const char *filename)
{
    UTIL_ASSERT(symbols);

    uint32_t *results = malloc(count*count*sizeof(uint32_t));
    memset(results, 0, count*count*sizeof(uint32_t));

    struct prob_map pmap[count];

    for (size_t i = 0; i < count; i++) {
        for (size_t j = 0; j < count; j++) {
            digram_prob_map(symbols, count, profile, pmap, i, j);
            for (size_t k = 0; k < count; k++) {
                if (pmap[k].probability < distance_threshold) {
                    results[i*count+j]++;
                }
                else continue;
            }
        }
    }

    FILE *file;
    int rc;

    file = fopen(filename, "w");
    UTIL_ASSERT(file);

    fputc(',', file);

    // title row
    for (size_t i = 0; i < count; i++)
        fprintf(file, "%c,", symbol_to_char(i));
    fputc('\n', file);

    // rows
    for (size_t i = 0; i < count; i++) {
        fprintf(file, "%c,", symbol_to_char(i));
        // cols
        for (size_t j = 0; j < count; j++) {
            fprintf(file, "%d,", results[j*count+i]);
        }
        fputc('\n', file);
    }

    fclose(file);

    free(results);
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
    // tmp(); return 0;

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
    HDVector symbol_table[27];

    if (text_filename) {
        compute_new_profile(text_filename, &profile, symbol_table, ARRLEN(symbol_table));
        util_log("INFO", "generated new profile based on '%s'", text_filename);

        util_log("INFO", "saving profile to '%s'", profile_filename);
        if (!hdvector_store_to_file(profile_filename, &profile, symbol_table, ARRLEN(symbol_table))) {
            util_log("WARN", "failed to store profile", profile_filename);
        }
    }
    else {
        if (!hdvector_load_from_file(profile_filename, &profile, symbol_table, ARRLEN(symbol_table))) {
            util_log("FATAL", "could not load profile '%s'", profile_filename);
            exit(1);
        }
        util_log("INFO", "loaded profile from file %s", profile_filename);
    }

    // get input, show probability of output

    digram_table(symbol_table, ARRLEN(symbol_table), &profile, 0.492, "digram-table.csv");
    // return 0;

    char input[16];

    for (;;) {

        memset(input, 0, sizeof(input));

        if (!fgets(input, sizeof(input), stdin)) break;
        if (!strcmp(input, "q")) break;

        // form query vector
        HDVector query;
        size_t indices[2] = { char_to_symbol(input[0]), char_to_symbol(input[1]) };
        hdvector_form_query(symbol_table, ARRLEN(symbol_table), indices, ARRLEN(indices), &query);

        // perform query
        hdvector_mult(&profile, &query, &query);

        print_most_likely_next_symbols(symbol_table, ARRLEN(symbol_table), &query, 27);
    }

    return 0;
}

