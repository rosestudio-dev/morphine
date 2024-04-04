//
// Created by whyiskra on 3/22/24.
//

#include <loaders.h>
#include <dlfcn.h>
#include "dlibcompiler.h"
#include "userdata/readfile.h"
#include "userdata/tbcfile.h"

static uint8_t file_read(morphine_coroutine_t U, void *data, const char **error) {
    (void) (U);

    FILE *file = (FILE *) data;

    if (feof(file)) {
        *error = "Binary corrupted";
        return 0;
    }

    uint8_t c = (uint8_t) fgetc(file);

    if (ferror(file)) {
        *error = "Error while reading";
        return 0;
    }

    return c;
}

void loader_source_file(morphine_coroutine_t U, const char *path) {
    struct compiler_instance *dlibcompiler = dlibcompiler_userdata(U, "libs/libcompiler.so");
    char *source = userdata_readfile(U, path);

    bool iserror = dlibcompiler_assemble(U, dlibcompiler, source, true);

    if (iserror) {
        const char *error = dlibcompiler_geterror(U, dlibcompiler);
        mapi_errorf(U, "Error while compiling\n%s", error);
    }

    int size = dlibcompiler_getbytecodesize(U, dlibcompiler);
    const uint8_t *native = dlibcompiler_getbytecodevector(U, dlibcompiler);

    if (native == NULL) {
        mapi_errorf(U, "Error while compiling");
    }

    mapi_rload(U, (size_t) size, native);

    mapi_rotate(U, 3);
    mapi_pop(U, 2);
}

void loader_binary_file(morphine_coroutine_t U, const char *path) {
    FILE *file = userdata_tbc_file(U, path, "r");
    mapi_load(U, NULL, file_read, NULL, file);
    mapi_rotate(U, 2);
    mapi_pop(U, 1);
}
