//
// Created by whyiskra on 3/22/24.
//

#include <loaders.h>
#include <string.h>
#include <morphinec.h>
#include "userdata/readfile.h"
#include "sio/file.h"

void loader_source_file(morphine_coroutine_t U, const char *path) {
    const char *source = userdata_readfile(U, path);

    mcapi_compile(U, "main", source, strlen(source));

    mapi_rotate(U, 2);
    mapi_pop(U, 1);
}

void loader_binary_file(morphine_coroutine_t U, const char *path) {
    sio_file(U, path, true, false, true);
    mcapi_from_binary(U);
    mcapi_rollout_as_vector(U);

    mapi_rotate(U, 2);
    mapi_sio_close(U, false);
    mapi_rotate(U, 2);
}
