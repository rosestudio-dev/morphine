//
// Created by why-iskra on 12.06.2024.
//

#include <stdio.h>
#include <string.h>
#include "sio/file.h"

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
}

static size_t file_write(morphine_sio_accessor_t A, void *data, const uint8_t *buffer, size_t size) {
    struct file_data *D = data;

    if (D->closed) {
        mapi_sio_accessor_error(A, "file closed");
    }

    return fwrite(buffer, 1, size, D->file);
}

static size_t file_read(morphine_sio_accessor_t A, void *data, uint8_t *buffer, size_t size) {
    struct file_data *D = data;

    if (D->closed) {
        mapi_sio_accessor_error(A, "file closed");
    }

    return fread(buffer, 1, size, D->file);
}

static bool file_eos(morphine_sio_accessor_t A, void *data) {
    struct file_data *D = data;

    if (D->closed) {
        mapi_sio_accessor_error(A, "file closed");
    }

    return feof(D->file) != 0;
}

static size_t file_tell(morphine_sio_accessor_t A, void *data) {
    struct file_data *D = data;

    if (D->closed) {
        mapi_sio_accessor_error(A, "file closed");
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

    switch (mode) {
        case MORPHINE_SIO_SEEK_MODE_SET:
            return fseek(D->file, (long) size, SEEK_SET) == 0;
        case MORPHINE_SIO_SEEK_MODE_CUR:
            return fseek(D->file, (long) size, SEEK_CUR) == 0;
        case MORPHINE_SIO_SEEK_MODE_PRV:
            return fseek(D->file, -((long) size), SEEK_CUR) == 0;
        case MORPHINE_SIO_SEEK_MODE_END:
            return fseek(D->file, (long) size, SEEK_END) == 0;
    }

    return false;
}

void sio_file(
    morphine_coroutine_t U,
    const char *path,
    bool read,
    bool write,
    bool binary
) {
    size_t path_len = strlen(path);
    size_t size = sizeof(struct file_data) + sizeof(char) * (path_len + 1);
    struct file_data *D = mapi_push_userdata(U, "file", size);

    char *file_path = ((void *) D) + sizeof(struct file_data);
    memcpy(file_path, path, path_len);
    file_path[path_len] = '\0';

    (*D) = (struct file_data) {
        .path = file_path,
        .file = NULL,
        .closed = true
    };

    memset(D->mode, 0, sizeof(D->mode) / sizeof(D->mode[0]));

    if (read && write) {
        D->mode[0] = 'r';
        D->mode[1] = 'w';
    } else if (read) {
        D->mode[0] = 'r';
    } else if (write) {
        D->mode[0] = 'w';
    } else {
        mapi_error(U, "file must be writable or readable");
    }

    if (binary) {
        D->mode[strlen(D->mode)] = 'b';
    }

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
