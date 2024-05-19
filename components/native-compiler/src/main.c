//
// Created by why-iskra on 19.05.2024.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *file(const char *path) {
    FILE *file = fopen(path, "rb");

    if (file == NULL) {
        abort();
    }

    // get size

    int fseek_end_res = fseek(file, 0, SEEK_END);

    long tell_result = ftell(file);

    if (tell_result < 0 || fseek_end_res != 0) {
        abort();
    }

    int fseek_set_res = fseek(file, 0, SEEK_SET);

    if (fseek_set_res != 0) {
        abort();
    }

    size_t size = (size_t) tell_result + 1;

    // read

    char *result = malloc(size);
    memset(result, 0, size);

    size_t count = fread(result, 1, size, file);

    if (size - 1 != count) {
        abort();
    }

    // close

    fclose(file);

    return result;
}

int main(int argc, const char **argv) {
    if (argc < 2) {
        return 1;
    }

    char *text = file(argv[1]);
    printf("%s\n", text);

    return 0;
}
