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

#define DISTANCE_THRESHOLD (0.492)

#define SYMCOUNT (27)
HDVector hdv_symbol_table[SYMCOUNT] = {0};
HDVector hdv_profile = {0};


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

struct prob_map { size_t index; float probability; };
int prob_map_cmp_asc(const void *a, const void *b) {
    float pa = ((struct prob_map *)a)->probability;
    float pb = ((struct prob_map *)b)->probability;
    if (pa > pb) return 1;
    if (pa < pb) return -1;
    return 0;
}

void
make_profile(const char *filename)
{
    UTIL_ASSERT(filename);

    uint32_t sumvector[DIMENSIONS] = {0};
    uint32_t skipped = 0;
    String content;

    if (!string_load_from_file(filename, &content)) {
        util_log("FATAL", "failed to open %s", filename);
        exit(1);
    }

    hdvector_init_random(hdv_symbol_table, SYMCOUNT);

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
        hdvector_copy(&hdv[0], &hdv_symbol_table[a]);
        hdvector_copy(&hdv[1], &hdv_symbol_table[b]);
        hdvector_copy(&hdv[2], &hdv_symbol_table[c]);

        hdvector_shift(&hdv[0], 2);
        hdvector_shift(&hdv[1], 1);

        hdvector_mult(&hdv[0], &hdv[1], &hdv[0]);
        hdvector_mult(&hdv[0], &hdv[2], &hdv[0]);

        // add trigram vector bits to sumvector
        for (size_t i = 0; i < ARRLEN(hdv[0].data); i++) {
            for (size_t bit = 0; bit < BITS_IN_U64; bit++) {
                sumvector[i*BITS_IN_U64+bit] += (hdv[0].data[i] >> bit) & 1;
            }
        }
    }

    // populate profile based on sumvector bit totals
    for (size_t i = 0; i < DIMENSIONS; i++) {
        hdv_profile.data[i/BITS_IN_U64] |= ((sumvector[i] > ((content.size-2-skipped)/2)) << (i&(BITS_IN_U64-1)));
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
digram_table(const char *filename)
{
    uint32_t results[SYMCOUNT*SYMCOUNT] = {0};

    struct prob_map pmap[SYMCOUNT];

    for (size_t i = 0; i < SYMCOUNT; i++) {
        for (size_t j = 0; j < SYMCOUNT; j++) {
            digram_prob_map(hdv_symbol_table, SYMCOUNT, &hdv_profile, pmap, i, j);
            for (size_t k = 0; k < SYMCOUNT; k++) {
                if (pmap[k].probability < DISTANCE_THRESHOLD) {
                    results[i*SYMCOUNT+j]++;
                } else continue;
            }
        }
    }

    // write to file

    FILE *file;
    int rc;

    file = fopen(filename, "w");
    UTIL_ASSERT(file);

    fputc(',', file);

    // title row
    for (size_t i = 0; i < SYMCOUNT; i++)
        fprintf(file, "%c,", symbol_to_char(i));
    fputc('\n', file);

    // rows
    for (size_t i = 0; i < SYMCOUNT; i++) {
        fprintf(file, "%c,", symbol_to_char(i));
        // cols
        for (size_t j = 0; j < SYMCOUNT; j++) {
            fprintf(file, "%d,", results[j*SYMCOUNT+i]);
        }
        fputc('\n', file);
    }

    fclose(file);

    util_log("INFO", "wrote digram table to %s", filename);
}

void
load_profile(const char *filename)
{
    if (!hdvector_load_from_file(filename, &hdv_profile, hdv_symbol_table, ARRLEN(hdv_symbol_table))) {
        util_log("FATAL", "could not load profile '%s'", filename);
        exit(1);
    }
    util_log("INFO", "loaded profile from '%s'", filename);
}

void
help(const char *prog)
{
    fprintf(stdout, "Usage: %s {make-profile <text> | digram-table <output-csv> | interactive} <profile>\n", prog);
    fprintf(stdout, "\n");
    fprintf(stdout, "Commands:\n");
    fprintf(stdout, "    make-profile  generate hyper-dimensional vector profile based on <text>\n");
    fprintf(stdout, "    digram-table  generate digram table for <profile>. write to <output-csv>\n");
    fprintf(stdout, "    interactive   display most likely letters following user-input digram, based on <profile>\n");
}

int
main(int argc, char *argv[])
{
    const char *profile_filename = NULL;
    const char *text_filename = NULL;
    const char *digram_filename = NULL;

    bool make_profile_arg = false;
    bool digram_table_arg = false;
    bool interactive_arg = false;

    if (argc == 4 && STREQ("make-profile", argv[1])) {
        text_filename = argv[2];
        profile_filename = argv[3];
        make_profile_arg = true;
    } else if (argc == 4 && STREQ("digram-table", argv[1])) {
        profile_filename = argv[3];
        digram_filename = argv[2];
        digram_table_arg = true;
    } else if (argc == 3 && STREQ("interactive", argv[1])) {
        profile_filename = argv[2];
        interactive_arg = true;
    } else {
        help(argv[0]);
        exit(1);
    }

    if (make_profile_arg) {
        make_profile(text_filename);
        util_log("INFO", "generated new profile based on '%s'", text_filename);

        if (!hdvector_store_to_file(profile_filename, &hdv_profile, hdv_symbol_table, ARRLEN(hdv_symbol_table)))
            util_log("WARN", "failed to store profile", profile_filename);
        else
            util_log("INFO", "saved profile to '%s'", profile_filename);

        exit(0);
    }

    if (digram_table_arg) {
        load_profile(profile_filename);
        digram_table(digram_filename);
        exit(0);
    }

    UTIL_ASSERT(interactive_arg);

    load_profile(profile_filename);

    char input[16];

    for (;;) {
        fprintf(stdout, "Enter digram (q to quit): ");

        memset(input, 0, sizeof(input));

        // read stdin
        if (!fgets(input, sizeof(input), stdin)) break;

        // check for quit cmd
        if (input[0] == 'q' && input[1] == '\n') break;

        if (!(isalpha(input[0]) || input[0] == ' ') ||
            !(isalpha(input[1]) || input[1] == ' ') ||
            strlen(input) != 3)
        {
            fprintf(stdout, ANSI_COLOR_RED"E: "ANSI_COLOR_RESET);
            fprintf(stdout, "Invalid digram. Must be a combination of two letters, or a letter and a space.\n");
            continue;
        }

        // form query vector
        HDVector query;
        size_t indices[2] = { char_to_symbol(input[0]), char_to_symbol(input[1]) };
        hdvector_form_query(hdv_symbol_table, SYMCOUNT, indices, ARRLEN(indices), &query);

        // perform query
        hdvector_mult(&hdv_profile, &query, &query);

        struct prob_map pmap[SYMCOUNT];
        memset(pmap, 0, sizeof(pmap));

        // populate probability map
        for (size_t i = 0; i < SYMCOUNT; i++) {
            pmap[i].index = i;
            pmap[i].probability = hdvector_distance(&hdv_symbol_table[i], &query);
        }

        // sort by probability, ascending
        qsort(pmap, SYMCOUNT, sizeof(*pmap), prob_map_cmp_asc);

        // print in order of most likely (lowest distance)
        for (size_t i = 0; i < SYMCOUNT; i++) {
            printf("  '%c': %.04f\n",
                   symbol_to_char(pmap[i].index),
                   pmap[i].probability);
        }
    }

    return 0;
}

