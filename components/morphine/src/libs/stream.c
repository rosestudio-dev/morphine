//
// Created by why-iskra on 19.05.2024.
//

#include <morphine.h>
#include <memory.h>
#include "morphine/libs/builtin.h"

static void close(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            if (mapi_args(U) == 2) {
                mapi_push_arg(U, 1);
                bool force = mapi_get_boolean(U);
                mapi_push_arg(U, 0);
                mapi_stream_close(U, force);
            } else {
                maux_expect_args(U, 1);
                mapi_push_arg(U, 0);
                mapi_stream_close(U, false);
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
            bool result = mapi_stream_seek_set(U, offset);

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
            bool result = mapi_stream_seek_cur(U, offset);

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
            bool result = mapi_stream_seek_prv(U, offset);

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
            bool result = mapi_stream_seek_end(U, offset);

            mapi_push_boolean(U, result);
            maux_nb_return();
    maux_nb_end
}

static void tell(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            size_t pos = mapi_stream_tell(U);
            mapi_push_size(U, pos, "index");
            maux_nb_return();
    maux_nb_end
}

static void eos(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            bool result = mapi_stream_eos(U);
            mapi_push_boolean(U, result);
            maux_nb_return();
    maux_nb_end
}

static void flush(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            mapi_stream_flush(U);
            maux_nb_leave();
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
            size_t read = mapi_stream_read(U, buffer, size);

            mapi_push_table(U);
            mapi_push_string(U, "result");
            mapi_push_stringn(U, (char *) buffer, size);
            mapi_table_set(U);
            mapi_push_string(U, "read");
            mapi_push_size(U, read, NULL);
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
            size_t size = mapi_string_len(U);

            mapi_push_arg(U, 0);
            size_t written = mapi_stream_write(U, (const uint8_t *) buffer, size * sizeof(char));

            mapi_push_size(U, written, NULL);
            maux_nb_return();
    maux_nb_end
}

static void readall(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            maux_stream_read_all(U);
            maux_nb_return();
    maux_nb_end
}

static void readto(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 3);
            mapi_push_arg(U, 1);
            const char *exit = mapi_get_cstr(U);

            mapi_push_arg(U, 2);
            bool eof = mapi_get_boolean(U);

            mapi_push_arg(U, 0);
            maux_stream_read_to(U, exit, eof);
            maux_nb_return();
    maux_nb_end
}

static void readline(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            maux_stream_read_line(U);
            maux_nb_return();
    maux_nb_end
}

static void buffer(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            ml_size factor = 32;
            if (mapi_args(U) != 0) {
                maux_expect_args(U, 1);

                mapi_push_arg(U, 0);
                factor = mapi_get_size(U, NULL);
            }

            maux_push_stream_buffer(U, factor);
            maux_nb_return();
    maux_nb_end
}

static void empty(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            bool read;
            bool write;

            if (mapi_args(U) == 0) {
                read = true;
                write = true;
            } else {
                maux_expect_args(U, 2);

                mapi_push_arg(U, 0);
                read = mapi_get_boolean(U);

                mapi_push_arg(U, 1);
                write = mapi_get_boolean(U);
            }

            maux_push_stream_empty(U, read, write);
            maux_nb_return();
    maux_nb_end
}

static maux_construct_element_t elements[] = {
    MAUX_CONSTRUCT_FUNCTION("close", close),
    MAUX_CONSTRUCT_FUNCTION("flush", flush),
    MAUX_CONSTRUCT_FUNCTION("read", read),
    MAUX_CONSTRUCT_FUNCTION("write", write),
    MAUX_CONSTRUCT_FUNCTION("seekset", seekset),
    MAUX_CONSTRUCT_FUNCTION("seekcur", seekcur),
    MAUX_CONSTRUCT_FUNCTION("seekprv", seekprv),
    MAUX_CONSTRUCT_FUNCTION("seekend", seekend),
    MAUX_CONSTRUCT_FUNCTION("tell", tell),
    MAUX_CONSTRUCT_FUNCTION("eos", eos),

    MAUX_CONSTRUCT_FUNCTION("readall", readall),
    MAUX_CONSTRUCT_FUNCTION("readto", readto),
    MAUX_CONSTRUCT_FUNCTION("readline", readline),

    MAUX_CONSTRUCT_FUNCTION("buffer", buffer),
    MAUX_CONSTRUCT_FUNCTION("eof", empty),
    MAUX_CONSTRUCT_END
};

static void library_init(morphine_coroutine_t U) {
    maux_construct(U, elements);

    mapi_push_stream_io(U);
    maux_table_set(U, "io");
    mapi_push_stream_err(U);
    maux_table_set(U, "err");
}

MORPHINE_LIB morphine_library_t mlib_builtin_stream(void) {
    return (morphine_library_t) {
        .name = "stream",
        .sharedkey = NULL,
        .init = library_init
    };
}
