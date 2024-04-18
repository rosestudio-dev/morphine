//
// Created by whyiskra on 3/24/24.
//

#include "userdata/tbcfile.h"

struct file {
    FILE *file;
};

static void tbc_file_free(morphine_instance_t I, void *p) {
    (void) I;

    struct file *file = p;
    if (file->file != NULL) {
        fclose(file->file);
    }
}

FILE *userdata_tbc_file(morphine_coroutine_t U, const char *path, const char *mode) {
    struct file *file = mapi_push_userdata(U, "tbc_file", sizeof(struct file), NULL, tbc_file_free);
    file->file = fopen(path, mode);

    if (file->file == NULL) {
        mapi_errorf(U, "Cannot open file %s", path);
    }

    return file->file;
}