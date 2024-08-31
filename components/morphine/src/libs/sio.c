//
// Created by why-iskra on 19.05.2024.
//

#include <morphine.h>
#include <memory.h>
#include "morphine/libs/builtin.h"

static void isopened(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            mapi_push_boolean(U, mapi_sio_is_opened(U));
            maux_nb_return();
    maux_nb_end
}

static void close(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            if (mapi_args(U) == 2) {
                mapi_push_arg(U, 1);
                bool force = mapi_get_boolean(U);
                mapi_push_arg(U, 0);
                mapi_sio_close(U, force);
            } else {
                maux_expect_args(U, 1);
                mapi_push_arg(U, 0);
                mapi_sio_close(U, false);
            }
            maux_nb_leave();
    maux_nb_end
}

static void seekset(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 2);

            mapi_push_arg(U, 1);
            ml_size offset = mapi_get_size(U, "position");
            mapi_push_arg(U, 0);
            bool result = mapi_sio_seek_set(U, offset);

            mapi_push_boolean(U, result);
            maux_nb_return();
    maux_nb_end
}

static void seekcur(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 2);

            mapi_push_arg(U, 1);
            ml_size offset = mapi_get_size(U, "offset");
            mapi_push_arg(U, 0);
            bool result = mapi_sio_seek_cur(U, offset);

            mapi_push_boolean(U, result);
            maux_nb_return();
    maux_nb_end
}

static void seekprv(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 2);

            mapi_push_arg(U, 1);
            ml_size offset = mapi_get_size(U, "offset");
            mapi_push_arg(U, 0);
            bool result = mapi_sio_seek_prv(U, offset);

            mapi_push_boolean(U, result);
            maux_nb_return();
    maux_nb_end
}

static void seekend(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 2);

            mapi_push_arg(U, 1);
            ml_size offset = mapi_get_size(U, "position");
            mapi_push_arg(U, 0);
            bool result = mapi_sio_seek_end(U, offset);

            mapi_push_boolean(U, result);
            maux_nb_return();
    maux_nb_end
}

static void tell(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            size_t pos = mapi_sio_tell(U);
            mapi_push_size(U, pos, "index");
            maux_nb_return();
    maux_nb_end
}

static void eos(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            bool result = mapi_sio_eos(U);
            mapi_push_boolean(U, result);
            maux_nb_return();
    maux_nb_end
}

static void flush(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            mapi_sio_flush(U);
            maux_nb_leave();
    maux_nb_end
}

static void io(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 0);
            mapi_push_sio_io(U);
            maux_nb_return();
    maux_nb_end
}

static void error(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 0);
            mapi_push_sio_error(U);
            maux_nb_return();
    maux_nb_end
}

static void read(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 2);

            mapi_push_arg(U, 1);
            ml_size size = mapi_get_size(U, NULL);
            mapi_pop(U, 1);

            uint8_t *buffer = mapi_push_userdata_uni(U, size);
            memset(buffer, 0, size);

            mapi_push_arg(U, 0);
            size_t lost = mapi_sio_read(U, buffer, size);

            mapi_push_table(U);
            mapi_push_string(U, "result");
            mapi_push_stringn(U, (char *) buffer, size);
            mapi_table_set(U);
            mapi_push_string(U, "lost");
            mapi_push_size(U, lost, NULL);
            mapi_table_set(U);
            maux_nb_return();
    maux_nb_end
}

static void write(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 2);

            mapi_push_arg(U, 1);
            const char *buffer = mapi_get_string(U);
            ml_size size = mapi_string_len(U);

            mapi_push_arg(U, 0);
            size_t written = mapi_sio_write(U, (const uint8_t *) buffer, size);

            mapi_push_size(U, written, NULL);
            maux_nb_return();
    maux_nb_end
}

static void buffer(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            if (mapi_args(U) != 4) {
                maux_expect_args(U, 3);
            }

            mapi_push_arg(U, 0);
            ml_size factor = mapi_get_size(U, NULL);

            mapi_push_arg(U, 1);
            bool read = mapi_get_boolean(U);

            mapi_push_arg(U, 2);
            bool write = mapi_get_boolean(U);

            if (mapi_args(U) == 4) {
                mapi_push_arg(U, 3);
            } else {
                mapi_push_string(U, "");
            }

            maux_push_sio_buffer(U, factor, read, write);
            maux_nb_return();
    maux_nb_end
}

static void extractstring(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            maux_sio_extract_string(U);
            maux_nb_return();
    maux_nb_end
}

static morphine_library_function_t functions[] = {
    { "isopened",      isopened },
    { "close",         close },
    { "flush",         flush },
    { "read",          read },
    { "write",         write },
    { "seekset",       seekset },
    { "seekcur",       seekcur },
    { "seekprv",       seekprv },
    { "seekend",       seekend },
    { "tell",          tell },
    { "eos",           eos },
    { "io",            io },
    { "error",         error },
    { "buffer",        buffer },
    { "extractstring", extractstring },
    { NULL, NULL }
};

static morphine_library_t library = {
    .name = "sio",
    .types = NULL,
    .functions = functions,
    .integers = NULL,
    .decimals = NULL,
    .strings = NULL
};

MORPHINE_LIB morphine_library_t *mlib_builtin_sio(void) {
    return &library;
}
