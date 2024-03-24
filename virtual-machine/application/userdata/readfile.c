//
// Created by whyiskra on 3/24/24.
//

#include <string.h>
#include "userdata/readfile.h"

struct readfile {
    void *data;
    FILE *file;
    bool closed;
};

static void readfile_free(morphine_instance_t I, void *p) {
    struct readfile *readfile = p;

    if (!readfile->closed) {
        fclose(readfile->file);
    }

    mapi_allocator_free(I, readfile->data);
    mapi_allocator_free(I, readfile);
}

void *userdata_readfile(morphine_state_t S, const char *path) {
    struct readfile *readfile = mapi_allocator_uni(mapi_instance(S), NULL, sizeof(struct readfile));

    (*readfile) = (struct readfile) {
        .data = NULL,
        .file = fopen(path, "r"),
        .closed = false
    };

    if (readfile->file == NULL) {
        mapi_allocator_free(mapi_instance(S), readfile);
        mapi_errorf(S, "Cannot open file %s", path);
    }

    mapi_push_userdata(S, "readfile", readfile, NULL, readfile_free);

    // get size

    long int size;
    {
        int fseek_end_res = fseek(readfile->file, 0, SEEK_END);

        size = ftell(readfile->file);

        if (size < 0 || fseek_end_res != 0) {
            mapi_errorf(S, "Cannot get file size");
        }

        int fseek_set_res = fseek(readfile->file, 0, SEEK_SET);

        if (fseek_set_res != 0) {
            goto cannot_read;
        }
    }

    // read

    readfile->data = mapi_allocator_uni(mapi_instance(S), NULL, size + 1);
    memset(readfile->data, 0, size + 1);

    size_t result = fread(readfile->data, 1, size, readfile->file);

    if (result != size) {
        goto cannot_read;
    }

    // close

    fclose(readfile->file);
    readfile->closed = true;

    return readfile->data;

cannot_read:
    mapi_errorf(S, "Cannot read file");
}