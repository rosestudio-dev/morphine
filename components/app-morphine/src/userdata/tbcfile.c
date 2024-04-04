//
// Created by whyiskra on 3/24/24.
//

#include "userdata/tbcfile.h"

static void tbc_file_free(morphine_instance_t I, void *p) {
    (void) I;

    fclose(p);
}

FILE *userdata_tbc_file(morphine_coroutine_t U, const char *path, const char *mode) {
    FILE *file = fopen(path, mode);

    if (file == NULL) {
        mapi_errorf(U, "Cannot open file %s", path);
    }

    mapi_push_userdata(U, "tbc_file", file, NULL, tbc_file_free);

    return file;
}