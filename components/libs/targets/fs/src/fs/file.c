//
// Created by why-iskra on 31.07.2024.
//

#include "morphinel/fs/file.h"
#include <stdio.h>
#include <string.h>

struct file_args {
    char mode[4];
    const char *path;
};

static void *file_open(morphine_instance_t I, void *args) {
    struct file_args *file_args = args;

    FILE *file = fopen(file_args->path, file_args->mode);
    if (file == NULL) {
        mapi_ierror(I, "failed to open file");
    }

    return file;
}

static void *file_temp_open(morphine_instance_t I, mattr_unused void *args) {
    FILE *file = tmpfile();
    if (file == NULL) {
        mapi_ierror(I, "failed to open file");
    }

    return file;
}

static void file_close(mattr_unused morphine_instance_t I, void *data, mattr_unused bool force) {
    FILE *file = data;
    fclose(file);
}

static void file_flush(morphine_instance_t I, void *data) {
    FILE *file = data;

    fflush(file);

    if (ferror(file)) {
        mapi_ierror(I, "file error");
    }
}

static size_t file_write(morphine_instance_t I, void *data, const uint8_t *buffer, size_t size) {
    FILE *file = data;

    size_t result = fwrite(buffer, 1, size, file);

    if (ferror(file)) {
        mapi_ierror(I, "file error");
    }

    return result;
}

static size_t file_read(morphine_instance_t I, void *data, uint8_t *buffer, size_t size) {
    FILE *file = data;

    size_t result = fread(buffer, 1, size, file);

    if (ferror(file)) {
        mapi_ierror(I, "file error");
    }

    return result;
}

static bool file_eos(morphine_instance_t I, void *data) {
    FILE *file = data;

    bool result = feof(file) != 0;

    if (ferror(file)) {
        mapi_ierror(I, "file error");
    }

    return result;
}

static size_t file_tell(morphine_instance_t I, void *data) {
    FILE *file = data;

    if (ferror(file)) {
        mapi_ierror(I, "file error");
    }

    long int tell = ftell(file);
    if (tell < 0) {
        mapi_ierror(I, "file error");
    }

    return (size_t) tell;
}

static bool file_seek(morphine_instance_t I, void *data, size_t size, mtype_seek_t mode) {
    FILE *file = data;

    if (size > -1UL) {
        mapi_ierror(I, "too big offset");
    }

    bool result = false;
    switch (mode) {
        case MTYPE_SEEK_SET: result = fseek(file, (long) size, SEEK_SET) == 0; break;
        case MTYPE_SEEK_CUR: result = fseek(file, (long) size, SEEK_CUR) == 0; break;
        case MTYPE_SEEK_PRV: result = fseek(file, -((long) size), SEEK_CUR) == 0; break;
        case MTYPE_SEEK_END: result = fseek(file, (long) size, SEEK_END) == 0; break;
    }

    if (ferror(file)) {
        mapi_ierror(I, "file error");
    }

    return result;
}

MORPHINE_API void mlapi_fs_file(morphine_coroutine_t U, bool read, bool write, bool binary) {
    const char *path = mapi_get_cstr(U);

    struct file_args args = { .path = path };

    memset(args.mode, 0, sizeof(args.mode) / sizeof(args.mode[0]));

    size_t binary_offset;
    if (read && write) {
        args.mode[0] = 'a';
        args.mode[1] = '+';
        binary_offset = 2;
    } else if (read) {
        args.mode[0] = 'r';
        binary_offset = 1;
    } else if (write) {
        args.mode[0] = 'w';
        binary_offset = 1;
    } else {
        mapi_error(U, "file must be writable or readable");
    }

    if (binary) {
        args.mode[binary_offset] = 'b';
    }

    // create stream

    morphine_stream_interface_t interface = {
        .open = file_open,
        .close = file_close,
        .read = read ? file_read : NULL,
        .write = write ? file_write : NULL,
        .flush = write ? file_flush : NULL,
        .eos = file_eos,
        .tell = file_tell,
        .seek = file_seek,
    };

    mapi_push_stream(U, interface, false, &args);

    mapi_rotate(U, 2);
    mapi_pop(U, 1);
}

MORPHINE_API void mlapi_fs_temp(morphine_coroutine_t U) {
    morphine_stream_interface_t interface = {
        .open = file_temp_open,
        .close = file_close,
        .read = file_read,
        .write = file_write,
        .flush = file_flush,
        .eos = file_eos,
        .tell = file_tell,
        .seek = file_seek,
    };

    mapi_push_stream(U, interface, false, NULL);
}
