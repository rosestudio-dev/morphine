//
// Created by whyiskra on 3/22/24.
//

#include <loaders.h>
#include <dlfcn.h>
#include "compiler.h"
#include "userdata/readfile.h"
#include "userdata/tbcfile.h"

static uint8_t file_read(morphine_instance_t I, void *data, const char **error) {
    (void) (I);

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
    struct compiler_instance *compiler = libcompiler_userdata(U);
    char *source = userdata_readfile(U, path);

    bool iserror = libcompiler_compile(U, compiler, source, true);

    if (iserror) {
        const char *error = libcompiler_get_error(U, compiler);
        mapi_errorf(U, "Error while compiling\n%s", error);
    }

    int size = libcompiler_get_bytecode_size(U, compiler);
    const uint8_t *native = libcompiler_get_bytecode(U, compiler);

    if (native == NULL) {
        mapi_errorf(U, "Error while compiling");
    }

    mapi_push_function(U, (size_t) size, native);

    libcompiler_release(U, compiler);

    mapi_rotate(U, 3);
    mapi_pop(U, 2);
}

void loader_binary_file(morphine_coroutine_t U, const char *path) {
    FILE *file = userdata_tbc_file(U, path, "r");
    mapi_function_load(U, NULL, file_read, NULL, file);
    mapi_rotate(U, 2);
    mapi_pop(U, 1);
}
