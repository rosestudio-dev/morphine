//
// Created by whyiskra on 25.12.23.
//

#include <morphine.h>
#include "morphine/libs/builtin.h"

static void version(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 0);
            mapi_push_string(U, mapi_version_name());
            maux_nb_return();
    maux_nb_end
}

static void print(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);

            maux_library_access(U, "value.tostr");
            mapi_push_arg(U, 0);
            mapi_call(U, 1);
        maux_nb_state(1)
            mapi_push_result(U);

            const uint8_t *string = (const uint8_t *) mapi_get_string(U);
            size_t size = mapi_string_len(U);
            mapi_push_sio_io(U);
            mapi_sio_write(U, string, size);

            mapi_pop(U, 1);
            maux_nb_leave();
    maux_nb_end
}

static void println(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            size_t args = mapi_args(U);
            if (args == 0) {
                mapi_push_sio_io(U);
                mapi_sio_write(U, (const uint8_t *) "\n", 1);
                maux_nb_leave();
            } else {
                maux_expect_args(U, 1);
                mapi_push_arg(U, 0);
            }

            maux_library_access(U, "value.tostr");
            mapi_rotate(U, 2);
            mapi_call(U, 1);
        maux_nb_state(1)
            mapi_push_result(U);

            const uint8_t *string = (const uint8_t *) mapi_get_string(U);
            size_t size = mapi_string_len(U);
            mapi_push_sio_io(U);
            mapi_sio_write(U, string, size);
            mapi_sio_write(U, (const uint8_t *) "\n", 1);
            maux_nb_leave();
    maux_nb_end
}

static void setmetatable(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            if (mapi_args(U) == 1) {
                mapi_push_table(U);
                mapi_push_arg(U, 0);
            } else {
                maux_expect_args(U, 2);

                mapi_push_arg(U, 0);
                maux_expect(U, "meta");

                mapi_push_arg(U, 1);
            }

            mapi_set_metatable(U);
            maux_nb_return();
    maux_nb_end
}

static void getmetatable(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);

            mapi_push_arg(U, 0);
            maux_expect(U, "meta");

            mapi_get_metatable(U);
            maux_nb_return();
    maux_nb_end
}

static void pcall(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            ml_size count = mapi_args(U);
            if (count < 1) {
                maux_expect_args(U, 1);
            }

            mapi_push_arg(U, 0);
            maux_expect(U, "callable");

            for (ml_size i = 1; i < count; i++) {
                mapi_push_arg(U, i);
            }

            mapi_catchable(U, 2);
            mapi_call(U, count - 1);
        maux_nb_state(1)
            mapi_push_result(U);
            maux_nb_return();
        maux_nb_state(2)
            mapi_push_thrown(U);
            maux_nb_return();
    maux_nb_end
}

static void ucall(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            ml_size count = mapi_args(U);
            if (count < 1) {
                maux_expect_args(U, 1);
            }

            mapi_push_arg(U, 0);
            maux_expect(U, "callable");

            for (ml_size i = 1; i < count; i++) {
                mapi_push_arg(U, i);
            }

            mapi_crashable(U);
            mapi_call(U, count - 1);
        maux_nb_state(1)
            mapi_push_result(U);
            maux_nb_return();
    maux_nb_end
}

static void error(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            mapi_error(U, NULL);
            maux_nb_leave();
    maux_nb_end
}

static void ecall(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            ml_size count = mapi_args(U);
            if (count < 2) {
                maux_expect_args(U, 2);
            }

            mapi_push_arg(U, 1);
            maux_expect(U, "callable");

            mapi_push_arg(U, 0);

            for (ml_size i = 2; i < count; i++) {
                mapi_push_arg(U, i);
            }

            mapi_ecall(U, count - 2);
        maux_nb_state(1)
            mapi_push_result(U);
            maux_nb_return();
    maux_nb_end
}

static void vcall(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 2);

            mapi_push_arg(U, 0);
            maux_expect(U, "callable");

            mapi_push_arg(U, 1);
            ml_size size = mapi_vector_len(U);
            mapi_pop(U, 1);
            for (ml_size i = 0; i < size; i++) {
                mapi_push_arg(U, 1);
                mapi_vector_get(U, i);
                mapi_rotate(U, 2);
                mapi_pop(U, 1);
            }

            mapi_call(U, size);
        maux_nb_state(1)
            mapi_push_result(U);
            maux_nb_return();
    maux_nb_end
}

static void callable(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            mapi_extract_callable(U);
            maux_nb_return();
    maux_nb_end
}

static maux_construct_element_t elements[] = {
    MAUX_CONSTRUCT_FUNCTION("version", version),
    MAUX_CONSTRUCT_FUNCTION("print", print),
    MAUX_CONSTRUCT_FUNCTION("println", println),
    MAUX_CONSTRUCT_FUNCTION("setmetatable", setmetatable),
    MAUX_CONSTRUCT_FUNCTION("getmetatable", getmetatable),
    MAUX_CONSTRUCT_FUNCTION("error", error),
    MAUX_CONSTRUCT_FUNCTION("pcall", pcall),
    MAUX_CONSTRUCT_FUNCTION("ucall", ucall),
    MAUX_CONSTRUCT_FUNCTION("ecall", ecall),
    MAUX_CONSTRUCT_FUNCTION("vcall", vcall),
    MAUX_CONSTRUCT_FUNCTION("callable", callable),

#define mspec_metatable_field(n, s) MAUX_CONSTRUCT_STRING("metafield."#s, MORPHINE_METATABLE_FIELD_PREFIX#s),

#include "morphine/misc/metatable/specification.h"

#undef mspec_metatable_field

    MAUX_CONSTRUCT_END
};

static void library_init(morphine_coroutine_t U) {
    maux_construct(U, elements);
}

MORPHINE_LIB morphine_library_t mlib_builtin_base(void) {
    return (morphine_library_t) {
        .name = "base",
        .sharedkey = NULL,
        .init = library_init
    };
}
