//
// Created by whyiskra on 3/24/24.
//

#include <string.h>
#include <stdio.h>
#include "userdata/readfile.h"

struct readfile {
    void *data;
    FILE *file;
    bool closed;
};

static void readfile_free(morphine_instance_t I, void *p) {
    struct readfile *readfile = p;

    if (!readfile->closed && readfile->file != NULL) {
        fclose(readfile->file);
    }

    mapi_allocator_free(I, readfile->data);
}

void *userdata_readfile(morphine_coroutine_t U, const char *path) {
    struct readfile *readfile = mapi_push_userdata(
        U, "readfile", sizeof(struct readfile)
    );

    (*readfile) = (struct readfile) {
        .data = NULL,
        .file = NULL,
        .closed = false
    };

    mapi_userdata_set_free(U, readfile_free);

    readfile->file = fopen(path, "r");

    if (readfile->file == NULL) {
        mapi_errorf(U, "cannot open file %s", path);
    }

    // get size

    long int size;
    {
        int fseek_end_res = fseek(readfile->file, 0, SEEK_END);

        size = ftell(readfile->file);

        if (size < 0 || fseek_end_res != 0) {
            mapi_errorf(U, "cannot get file size");
        }

        int fseek_set_res = fseek(readfile->file, 0, SEEK_SET);

        if (fseek_set_res != 0) {
            goto cannot_read;
        }
    }

    // read

    readfile->data = mapi_allocator_uni(mapi_instance(U), NULL, (size_t) (size + 1));
    memset(readfile->data, 0, (unsigned long) (size + 1));

    size_t result = fread(readfile->data, 1, (unsigned long) size, readfile->file);

    if (result != (size_t) size) {
        goto cannot_read;
    }

    // close

    fclose(readfile->file);
    readfile->closed = true;

    return readfile->data;

cannot_read:
    mapi_errorf(U, "cannot read file");
}