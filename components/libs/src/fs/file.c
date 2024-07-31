//
// Created by why-iskra on 31.07.2024.
//

#include <stdio.h>
#include <string.h>
#include "morphinel/fs/file.h"

struct file_data {
    char mode[4];
    const char *path;
    bool closed;
    FILE *file;
};

static void *file_open(morphine_sio_accessor_t A, void *data) {
    struct file_data *D = data;

    D->file = fopen(D->path, D->mode);
    if (D->file == NULL) {
        mapi_sio_accessor_error(A, "failed to open file");
    }

    D->closed = false;

    if (ferror(D->file)) {
        mapi_sio_accessor_error(A, "file error");
    }

    return D;
}

static void file_close(morphine_sio_accessor_t A, void *data) {
    (void) A;

    struct file_data *D = data;

    if (!D->closed) {
        D->closed = true;

        fclose(D->file);
    }
}

static void file_flush(morphine_sio_accessor_t A, void *data) {
    struct file_data *D = data;

    if (D->closed) {
        mapi_sio_accessor_error(A, "file closed");
    }

    fflush(D->file);

    if (ferror(D->file)) {
        mapi_sio_accessor_error(A, "file error");
    }
}

static size_t file_write(morphine_sio_accessor_t A, void *data, const uint8_t *buffer, size_t size) {
    struct file_data *D = data;

    if (D->closed) {
        mapi_sio_accessor_error(A, "file closed");
    }

    size_t result = fwrite(buffer, 1, size, D->file);

    if (ferror(D->file)) {
        mapi_sio_accessor_error(A, "file error");
    }

    return result;
}

static size_t file_read(morphine_sio_accessor_t A, void *data, uint8_t *buffer, size_t size) {
    struct file_data *D = data;

    if (D->closed) {
        mapi_sio_accessor_error(A, "file closed");
    }

    size_t result = fread(buffer, 1, size, D->file);

    if (ferror(D->file)) {
        mapi_sio_accessor_error(A, "file error");
    }

    return result;
}

static bool file_eos(morphine_sio_accessor_t A, void *data) {
    struct file_data *D = data;

    if (D->closed) {
        mapi_sio_accessor_error(A, "file closed");
    }

    bool result = feof(D->file) != 0;

    if (ferror(D->file)) {
        mapi_sio_accessor_error(A, "file error");
    }

    return result;
}

static size_t file_tell(morphine_sio_accessor_t A, void *data) {
    struct file_data *D = data;

    if (D->closed) {
        mapi_sio_accessor_error(A, "file closed");
    }

    if (ferror(D->file)) {
        mapi_sio_accessor_error(A, "file error");
    }

    long int tell = ftell(D->file);
    if (tell < 0) {
        mapi_sio_accessor_error(A, "file error");
    }

    return (size_t) tell;
}

static bool file_seek(morphine_sio_accessor_t A, void *data, size_t size, morphine_sio_seek_mode_t mode) {
    struct file_data *D = data;

    if (D->closed) {
        mapi_sio_accessor_error(A, "file closed");
    }

    if (size > -1UL) {
        mapi_sio_accessor_error(A, "too big offset");
    }

    bool result = false;
    switch (mode) {
        case MORPHINE_SIO_SEEK_MODE_SET:
            result = fseek(D->file, (long) size, SEEK_SET) == 0;
            break;
        case MORPHINE_SIO_SEEK_MODE_CUR:
            result = fseek(D->file, (long) size, SEEK_CUR) == 0;
            break;
        case MORPHINE_SIO_SEEK_MODE_PRV:
            result = fseek(D->file, -((long) size), SEEK_CUR) == 0;
            break;
        case MORPHINE_SIO_SEEK_MODE_END:
            result = fseek(D->file, (long) size, SEEK_END) == 0;
            break;
    }

    if (ferror(D->file)) {
        mapi_sio_accessor_error(A, "file error");
    }

    return result;
}

MORPHINE_API void mlapi_fs_file(
    morphine_coroutine_t U,
    bool read,
    bool write,
    bool binary
) {
    const char *path = mapi_get_string(U);
    struct file_data *D = mapi_push_userdata_uni(U, sizeof(struct file_data));

    (*D) = (struct file_data) {
        .path = path,
        .file = NULL,
        .closed = true
    };

    memset(D->mode, 0, sizeof(D->mode) / sizeof(D->mode[0]));

    size_t binary_offset;
    if (read && write) {
        mapi_error(U, "file cannot be writable and readable at the same time");
    } else if (read) {
        D->mode[0] = 'r';
        binary_offset = 1;
    } else if (write) {
        D->mode[0] = 'w';
        binary_offset = 1;
    } else {
        mapi_error(U, "file must be writable or readable");
    }

    if (binary) {
        D->mode[binary_offset] = 'b';
    }

    mapi_push_vector(U, 2);

    mapi_rotate(U, 2);
    mapi_vector_set(U, 0);

    mapi_rotate(U, 2);
    mapi_vector_set(U, 1);

    // create sio

    morphine_sio_interface_t interface = {
        .open = file_open,
        .close = file_close,
        .read = NULL,
        .write = NULL,
        .flush = NULL,
        .eos = file_eos,
        .tell = file_tell,
        .seek = file_seek
    };

    if (read) {
        interface.read = file_read;
    }

    if (write) {
        interface.write = file_write;
        interface.flush = file_flush;
    }

    mapi_push_sio(U, interface);

    mapi_rotate(U, 2);
    mapi_sio_hold(U);

    mapi_sio_open(U, D);
}

