//
// Created by whyiskra on 3/22/24.
//

#include <loaders.h>
#include <stdio.h>
#include <string.h>
#include "morphinec/compiler.h"
#include "userdata/readfile.h"
#include "userdata/tbcfile.h"

static size_t file_read(morphine_sio_accessor_t A, void *data, uint8_t *buffer, size_t size) {
    FILE *file = (FILE *) data;

    size_t read = 0;
    for(size_t i = 0; i < size; i ++) {
        if(feof(file)) {
            buffer[i] = 0;
        } else {
            buffer[i] = (uint8_t) fgetc(file);
            read ++;
        }

        if (ferror(file)) {
            mapi_sio_accessor_error(A, "Error while reading");
        }
    }

    return read;
}

void loader_source_file(morphine_coroutine_t U, const char *path) {
    const char *source = userdata_readfile(U, path);

    mcapi_compile(U, source, strlen(source));

    mapi_rotate(U, 2);
    mapi_pop(U, 1);
}

void loader_binary_file(morphine_coroutine_t U, const char *path) {
    FILE *file = userdata_tbc_file(U, path, "r");

    morphine_sio_interface_t interface = {
        .read = file_read,
        .open = NULL,
        .close = NULL,
        .write = NULL,
        .flush = NULL,
    };

    mapi_push_sio(U, interface);
    mapi_sio_open(U, file);
    mapi_push_function(U);

    mapi_rotate(U, 3);
    mapi_pop(U, 2);
}
