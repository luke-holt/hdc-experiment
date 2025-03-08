#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>

#define UTIL_IMPLEMENTATION
#include "util.h"


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

    rc = fread(str->data, str->size, 1, file);
    if (!rc) goto defer;

    success = true;

defer:
    fclose(file);
    return success;
}

void string_delete(String *str) {
    UTIL_ASSERT(str);
    UTIL_ASSERT(str->data);
    free(str->data);
    str->size = 0;
    str->data = NULL;
}


int
main(int argc, char *argv[])
{
    if (argc == 1) {
        util_log("FATAL", "No input file");
        util_log("INFO", "Usage: ./symbolcount <file>");
        return 1;
    }

    String content;
    if (!string_load_from_file(argv[1], &content)) {
        util_log("FATAL", "Could not open file '%s'", argv[1]);
        return 1;
    }

    size_t charlut[256] = {0};

    for (size_t i = 0; i < content.size; i++)
        charlut[content.data[i]]++;

    printf("Character count\n");
    for (int i = 0; i < 256; i++) if (charlut[i] > 0)
        printf("%02x | '%c' | %zu\n", i, (isprint((char)i)?(char)i:'.'), charlut[i]);

    string_delete(&content);
    return 0;
}
