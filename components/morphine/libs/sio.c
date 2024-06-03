//
// Created by why-iskra on 19.05.2024.
//

#include <morphine.h>
#include <memory.h>
#include "morphine/libs/loader.h"

static void isopened(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            mapi_push_boolean(U, mapi_sio_is_opened(U));
            nb_return();
    nb_end
}

static void close(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
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
            nb_leave();
    nb_end
}

static void seekset(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            maux_expect_args(U, 2);

            mapi_push_arg(U, 1);
            ml_size offset = mapi_get_size(U);
            mapi_push_arg(U, 0);
            bool result = mapi_sio_seek_set(U, offset);

            mapi_push_boolean(U, result);
            nb_return();
    nb_end
}

static void seekcur(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            maux_expect_args(U, 2);

            mapi_push_arg(U, 1);
            ml_size offset = mapi_get_size(U);
            mapi_push_arg(U, 0);
            bool result = mapi_sio_seek_cur(U, offset);

            mapi_push_boolean(U, result);
            nb_return();
    nb_end
}

static void seekprv(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            maux_expect_args(U, 2);

            mapi_push_arg(U, 1);
            ml_size offset = mapi_get_size(U);
            mapi_push_arg(U, 0);
            bool result = mapi_sio_seek_prv(U, offset);

            mapi_push_boolean(U, result);
            nb_return();
    nb_end
}

static void seekend(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            maux_expect_args(U, 2);

            mapi_push_arg(U, 1);
            ml_size offset = mapi_get_size(U);
            mapi_push_arg(U, 0);
            bool result = mapi_sio_seek_end(U, offset);

            mapi_push_boolean(U, result);
            nb_return();
    nb_end
}

static void tell(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            size_t pos = mapi_sio_tell(U);
            mapi_push_index(U, pos);
            nb_return();
    nb_end
}

static void eos(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            bool result = mapi_sio_eos(U);
            mapi_push_boolean(U, result);
            nb_return();
    nb_end
}

static void flush(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            mapi_sio_flush(U);
            nb_leave();
    nb_end
}

static void io(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            maux_expect_args(U, 0);
            mapi_push_sio_io(U);
            nb_return();
    nb_end
}

static void error(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            maux_expect_args(U, 0);
            mapi_push_sio_error(U);
            nb_return();
    nb_end
}

static void read(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            maux_expect_args(U, 2);

            mapi_push_arg(U, 1);
            ml_size size = mapi_get_size(U);
            mapi_pop(U, 1);

            uint8_t *buffer = mapi_push_userdata(U, "buffer", size);
            memset(buffer, 0, size);

            mapi_push_arg(U, 0);
            size_t lost = mapi_sio_read(U, buffer, size);

            mapi_push_table(U);
            mapi_push_string(U, "result");
            mapi_push_stringn(U, (char *) buffer, size);
            mapi_table_set(U);
            mapi_push_string(U, "lost");
            mapi_push_size(U, lost);
            mapi_table_set(U);
            nb_return();
    nb_end
}

static void write(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            maux_expect_args(U, 2);

            mapi_push_arg(U, 1);
            const char *buffer = mapi_get_string(U);
            ml_size size = mapi_string_len(U);

            mapi_push_arg(U, 0);
            size_t written = mapi_sio_write(U, (const uint8_t *) buffer, size);

            mapi_push_size(U, written);
            nb_return();
    nb_end
}

static struct maux_construct_field table[] = {
    { "isopened", isopened },
    { "close",    close },
    { "flush",    flush },
    { "read",     read },
    { "write",    write },
    { "seekset",  seekset },
    { "seekcur",  seekcur },
    { "seekprv",  seekprv },
    { "seekend",  seekend },
    { "tell",     tell },
    { "eos",      eos },
    { "io",       io },
    { "error",    error },
    { NULL, NULL }
};

void mlib_sio_loader(morphine_coroutine_t U) {
    maux_construct(U, table, "sio.");
}

MORPHINE_LIB void mlib_sio_call(morphine_coroutine_t U, const char *name, size_t argc) {
    maux_construct_call(U, table, "sio.", name, argc);
}
