//
// Created by whyiskra on 3/22/24.
//

#include <loaders.h>
#include <string.h>
#include <morphinec.h>
#include "morphinel/fs/file.h"

#define DEFAULT_MAIN_NAME     "main"
#define DEFAULT_MAIN_NAME_LEN ((sizeof(DEFAULT_MAIN_NAME) / sizeof(char)) - 1)

void loader_source_file(morphine_coroutine_t U, const char *path) {
    mapi_push_string(U, path);
    mlapi_fs_file(U, true, false, false);

    maux_sio_extract_string(U);

    mapi_rotate(U, 2);
    mapi_sio_close(U, false);

    const char *source = mapi_get_string(U);

    mcapi_compile(U, DEFAULT_MAIN_NAME, source, DEFAULT_MAIN_NAME_LEN, strlen(source));

    mapi_rotate(U, 2);
    mapi_pop(U, 1);
}

void loader_binary_file(morphine_coroutine_t U, const char *path) {
    mapi_push_string(U, path);
    mlapi_fs_file(U, true, false, true);

    mcapi_from_binary(U);
    mcapi_rollout_as_vector(U);

    mapi_rotate(U, 2);
    mapi_sio_close(U, false);
}
