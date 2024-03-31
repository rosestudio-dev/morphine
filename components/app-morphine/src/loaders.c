//
// Created by whyiskra on 3/22/24.
//

#include <loaders.h>
#include <dlfcn.h>
#include "dlibcompiler.h"
#include "userdata/readfile.h"
#include "userdata/tbcfile.h"

static uint8_t file_read(morphine_state_t S, void *data, const char **error) {
    (void) (S);

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

void loader_source_file(morphine_state_t S, const char *path) {
    struct compiler_instance *dlibcompiler = dlibcompiler_userdata(S, "libs/libcompiler.so");
    char *source = userdata_readfile(S, path);

    bool iserror = dlibcompiler_assemble(S, dlibcompiler, source, true);

    if (iserror) {
        const char *error = dlibcompiler_geterror(S, dlibcompiler);
        mapi_errorf(S, "Error while compiling\n%s", error);
    }

    int size = dlibcompiler_getbytecodesize(S, dlibcompiler);
    const uint8_t *native = dlibcompiler_getbytecodevector(S, dlibcompiler);

    if (native == NULL) {
        mapi_errorf(S, "Error while compiling");
    }

    mapi_rload(S, (size_t) size, native);

    mapi_rotate(S, 3);
    mapi_pop(S, 2);
}

void loader_binary_file(morphine_state_t S, const char *path) {
    FILE *file = userdata_tbc_file(S, path, "r");
    mapi_load(S, NULL, file_read, NULL, file);
    mapi_rotate(S, 2);
    mapi_pop(S, 1);
}
