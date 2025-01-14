//
// Created by why-iskra on 31.07.2024.
//

#include "morphinel/fs/file.h"
#include <stdio.h>
#include <string.h>

struct file_data {
    FILE *file;
};

struct file_args {
    char mode[4];
    const char *path;
};

static void file_open(morphine_instance_t I, void *data, void *args) {
    struct file_args *file_args = args;
    struct file_data *file_data = data;

    file_data->file = fopen(file_args->path, file_args->mode);
    if (file_data->file == NULL) {
        mapi_ierror(I, "failed to open file");
    }

    if (ferror(file_data->file)) {
        mapi_ierror(I, "file error");
    }
}

static void file_temp_open(morphine_instance_t I, void *data, void *args) {
    (void) args;
    struct file_data *file_data = data;

    file_data->file = tmpfile();
    if (file_data->file == NULL) {
        mapi_ierror(I, "failed to open temp file");
    }

    if (ferror(file_data->file)) {
        mapi_ierror(I, "file error");
    }
}

static void file_close(morphine_instance_t I, void *data) {
    (void) I;
    struct file_data *file_data = data;
    fclose(file_data->file);
}

static void file_flush(morphine_instance_t I, void *data) {
    struct file_data *D = data;

    fflush(D->file);

    if (ferror(D->file)) {
        mapi_ierror(I, "file error");
    }
}

static size_t file_write(morphine_instance_t I, void *data, const uint8_t *buffer, size_t size) {
    struct file_data *D = data;

    size_t result = fwrite(buffer, 1, size, D->file);

    if (ferror(D->file)) {
        mapi_ierror(I, "file error");
    }

    return result;
}

static size_t file_read(morphine_instance_t I, void *data, uint8_t *buffer, size_t size) {
    struct file_data *D = data;

    size_t result = fread(buffer, 1, size, D->file);

    if (ferror(D->file)) {
        mapi_ierror(I, "file error");
    }

    return result;
}

static bool file_eos(morphine_instance_t I, void *data) {
    struct file_data *D = data;

    bool result = feof(D->file) != 0;

    if (ferror(D->file)) {
        mapi_ierror(I, "file error");
    }

    return result;
}

static size_t file_tell(morphine_instance_t I, void *data) {
    struct file_data *D = data;

    if (ferror(D->file)) {
        mapi_ierror(I, "file error");
    }

    long int tell = ftell(D->file);
    if (tell < 0) {
        mapi_ierror(I, "file error");
    }

    return (size_t) tell;
}

static bool file_seek(morphine_instance_t I, void *data, size_t size, morphine_stream_seek_mode_t mode) {
    struct file_data *D = data;

    if (size > -1UL) {
        mapi_ierror(I, "too big offset");
    }

    bool result = false;
    switch (mode) {
        case MORPHINE_STREAM_SEEK_MODE_SET: result = fseek(D->file, (long) size, SEEK_SET) == 0; break;
        case MORPHINE_STREAM_SEEK_MODE_CUR: result = fseek(D->file, (long) size, SEEK_CUR) == 0; break;
        case MORPHINE_STREAM_SEEK_MODE_PRV: result = fseek(D->file, -((long) size), SEEK_CUR) == 0; break;
        case MORPHINE_STREAM_SEEK_MODE_END: result = fseek(D->file, (long) size, SEEK_END) == 0; break;
    }

    if (ferror(D->file)) {
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
        .data_size = sizeof(struct file_data),
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
        .data_size = sizeof(struct file_data),
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
