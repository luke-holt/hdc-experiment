#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

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
    if (!rc) goto defer;

    str->size = ftell(file);

    rc = fseek(file, 0, SEEK_SET);
    if (!rc) goto defer;

    str->data = malloc(str->size);
    UTIL_ASSERT(str->data);

    rc = fread(str->data, str->size, 1, file);
    if (rc != str->size) goto defer;

    success = true;

defer:
    fclose(file);
    return success;
}

void string_delete(String *str) {
    UTIL_ASSERT(str);
    UTIL_ASSERT(str->data);
    free(str->data);
}



#define FILENAME "./test.txt"

int
main(void)
{
    HDVector *symbol_table = malloc(sizeof(HDVector) * 27);
    UTIL_ASSERT(symbol_table);

    for (size_t i = 0; i < 27; i++)
        hdvector_init_random(&symbol_table[i]);

    String content;

    if (!string_load_from_file(FILENAME, &content)) {
        util_log("failed to open %s", FILENAME);
        return 1;
    }

    for (size_t i = 0; i < content.size - 2; i++) {

    }

    free(symbol_table);
    return 0;
}

