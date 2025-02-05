//
// Created by why-iskra on 13.06.2024.
//

#include "morphine/auxiliary/stream.h"
#include "morphine/api.h"
#include "morphine/utils/overflow.h"
#include <string.h>

// buffer

struct buffer_data {
    size_t allocated;
    size_t factor;

    size_t size;
    size_t pointer;
    uint8_t *vector;
};

struct buffer_args {
    size_t factor;
};

static void *buffer_open(morphine_instance_t I, void *args) {
    struct buffer_args *buffer_args = args;

    struct buffer_data *buffer_data = mapi_allocator_uni(I, NULL, sizeof(struct buffer_data));
    *buffer_data = (struct buffer_data) {
        .allocated = 0,
        .factor = buffer_args->factor,
        .size = 0,
        .pointer = 0,
        .vector = NULL,
    };

    return buffer_data;
}

static void buffer_close(morphine_instance_t I, void *data, mattr_unused bool force) {
    struct buffer_data *buffer_data = data;
    mapi_allocator_free(I, buffer_data->vector);
    mapi_allocator_free(I, buffer_data);
}

static size_t buffer_read(mattr_unused morphine_instance_t I, void *data, uint8_t *buffer, size_t size) {
    struct buffer_data *B = data;

    size_t read = 0;
    for (size_t i = 0; i < size; i++) {
        if (B->pointer >= B->size) {
            buffer[i] = 0;
        } else {
            buffer[i] = B->vector[B->pointer];
            B->pointer++;
            read++;
        }
    }

    return read;
}

static size_t buffer_write(morphine_instance_t I, void *data, const uint8_t *buffer, size_t size) {
    struct buffer_data *B = data;

    for (size_t i = 0; i < size; i++) {
        if (B->pointer >= B->size) {
            mm_overflow_add(B->size, 1) {
                mapi_ierror(I, "stream buffer overflow");
            }

            B->size++;
        }

        if (B->size >= B->allocated) {
            mm_overflow_add(B->allocated, B->factor) {
                mapi_ierror(I, "stream buffer overflow");
            }

            size_t new_size = B->allocated + B->factor;
            B->vector = mapi_allocator_vec(I, B->vector, new_size, sizeof(char));
            B->allocated = new_size;
        }

        B->vector[B->pointer] = buffer[i];
        B->pointer++;
    }

    return size;
}

static bool buffer_seek(morphine_instance_t I, void *data, size_t offset, mtype_seek_t mode) {
    (void) I;
    struct buffer_data *B = data;

    switch (mode) {
        case MTYPE_SEEK_SET: {
            if (offset > B->size) {
                return false;
            } else {
                B->pointer = offset;
                return true;
            }
        }
        case MTYPE_SEEK_CUR: {
            if (B->pointer > B->size) {
                B->pointer = B->size;
            }

            if (offset > B->size - B->pointer) {
                return false;
            } else {
                B->pointer += offset;
                return true;
            }
        }
        case MTYPE_SEEK_PRV: {
            if (B->pointer > B->size) {
                B->pointer = B->size;
            }

            if (offset > B->pointer) {
                return false;
            } else {
                B->pointer -= offset;
                return true;
            }
        }
        case MTYPE_SEEK_END: {
            if (offset > B->size) {
                return false;
            } else {
                B->pointer = B->size - offset;
                return true;
            }
        }
    }

    return false;
}

static size_t buffer_tell(morphine_instance_t I, void *data) {
    (void) I;
    struct buffer_data *B = data;

    if (B->pointer > B->size) {
        return B->size;
    } else {
        return B->pointer;
    }
}

static bool buffer_eos(morphine_instance_t I, void *data) {
    (void) I;
    struct buffer_data *B = data;
    return B->pointer >= B->size;
}

MORPHINE_AUX void maux_push_stream_buffer(morphine_coroutine_t U, size_t factor) {
    if (factor == 0) {
        mapi_error(U, "stream buffer extension factor is zero");
    }

    morphine_stream_interface_t interface = {
        .open = buffer_open,
        .close = buffer_close,
        .read = buffer_read,
        .write = buffer_write,
        .flush = NULL,
        .eos = buffer_eos,
        .tell = buffer_tell,
        .seek = buffer_seek,
    };

    struct buffer_args args = {
        .factor = factor,
    };

    mapi_push_stream(U, interface, false, &args);
}

// empty

static size_t empty_eof_read(mattr_unused morphine_instance_t I, mattr_unused void *data, uint8_t *buffer, size_t size) {
    memset(buffer, 0, size);
    return size;
}

static size_t empty_zero_read(mattr_unused morphine_instance_t I, mattr_unused void *data, uint8_t *buffer, size_t size) {
    memset(buffer, 0, size);
    return 0;
}

static size_t empty_eof_write(
    mattr_unused morphine_instance_t I,
    mattr_unused void *data,
    const mattr_unused uint8_t *buffer,
    size_t size
) {
    return size;
}

static size_t empty_zero_write(
    mattr_unused morphine_instance_t I,
    mattr_unused void *data,
    const mattr_unused uint8_t *buffer,
    mattr_unused size_t size
) {
    return 0;
}

static bool empty_eof_eos(mattr_unused morphine_instance_t I, mattr_unused void *data) {
    return false;
}

static bool empty_zero_eos(mattr_unused morphine_instance_t I, mattr_unused void *data) {
    return true;
}

MORPHINE_AUX void maux_push_stream_empty(morphine_coroutine_t U, bool read_eof, bool write_eof) {
    morphine_stream_interface_t interface = {
        .open = NULL,
        .close = NULL,
        .read = read_eof ? empty_eof_read : empty_zero_read,
        .write = write_eof ? empty_eof_write : empty_zero_write,
        .eos = read_eof ? empty_eof_eos : empty_zero_eos,
        .flush = NULL,
        .tell = NULL,
        .seek = NULL,
    };

    mapi_push_stream(U, interface, false, NULL);
}

// other stuff

MORPHINE_AUX void maux_stream_read_all(morphine_coroutine_t U) {
    if (!mapi_stream_seek(U, 0, MTYPE_SEEK_END)) {
        mapi_error(U, "cannot read all");
    }

    size_t size = mapi_stream_tell(U);

    if (!mapi_stream_seek(U, 0, MTYPE_SEEK_SET)) {
        mapi_error(U, "cannot read all");
    }

    char *buffer = mapi_push_userdata_vec(U, size, sizeof(char));

    mapi_rotate(U, 2);

    size_t result = mapi_stream_read(U, (uint8_t *) buffer, size * sizeof(char));
    if (result != size * sizeof(char)) {
        mapi_error(U, "cannot read all");
    }

    mapi_push_stringn(U, buffer, size);
    mapi_rotate(U, 3);
    mapi_rotate(U, 3);
    mapi_pop(U, 1);
}

MORPHINE_AUX bool maux_stream_read_to(morphine_coroutine_t U, const char *exit, bool eof) {
    mapi_push_string(U, "");
    while (true) {
        char read = 0;

        mapi_rotate(U, 2);
        size_t read_count = mapi_stream_read(U, (uint8_t *) &read, sizeof(char));
        mapi_rotate(U, 2);

        if (read_count != sizeof(char)) {
            return false;
        }

        if ((eof && read == '\0') || strchr(exit, read) != NULL) {
            break;
        }

        mapi_push_stringn(U, &read, 1);
        mapi_string_concat(U);
    }

    return true;
}

MORPHINE_AUX bool maux_stream_read_line(morphine_coroutine_t U) {
    return maux_stream_read_to(U, "\n", true);
}

MORPHINE_AUX morphine_stream_interface_t
maux_stream_interface_srwf(mfunc_read_t read, mfunc_write_t write, mfunc_flush_t flush) {
    return (morphine_stream_interface_t) {
        .write = write,
        .read = read,
        .flush = flush,
        .open = NULL,
        .close = NULL,
        .seek = NULL,
        .tell = NULL,
        .eos = NULL,
    };
}
