//
// Created by why-iskra on 30.09.2024.
//

#include <morphine.h>
#include "morphine/libs/builtin.h"
#include "morphine/params.h"

static void create(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);

            mapi_push_exception(U);
            maux_nb_return();
    maux_nb_end
}

static void value(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);

            mapi_exception_value(U);
            maux_nb_return();
    maux_nb_end
}

static void message(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);

            mapi_exception_message(U);
            maux_nb_return();
    maux_nb_end
}

static void print(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            if (mapi_args(U) == 1) {
                mapi_push_sio_err(U);
            } else {
                maux_expect_args(U, 2);
                mapi_push_arg(U, 1);
            }

            mapi_push_arg(U, 0);
            mapi_exception_error_print(U);
            mapi_exception_stacktrace_print(U, MPARAM_TRACESTACK_COUNT);
            maux_nb_leave();
    maux_nb_end
}

static void error_print(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 2);
            mapi_push_arg(U, 1);
            mapi_push_arg(U, 0);
            mapi_exception_error_print(U);
            maux_nb_leave();
    maux_nb_end
}

static void stacktrace_print(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            ml_size count;
            if (mapi_args(U) == 3) {
                mapi_push_arg(U, 2);
                count = mapi_get_size(U, "count");
            } else {
                maux_expect_args(U, 2);
                mapi_push_arg(U, 0);
                count = mapi_exception_stacktrace_size(U);
            }
            mapi_pop(U, 1);

            mapi_push_arg(U, 1);
            mapi_push_arg(U, 0);
            mapi_exception_stacktrace_print(U, count);
            maux_nb_leave();
    maux_nb_end
}

static void stacktrace_record(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 2);

            mapi_push_arg(U, 1);
            morphine_coroutine_t coroutine = mapi_get_coroutine(U);

            mapi_push_arg(U, 0);
            mapi_exception_stacktrace_record(U, coroutine);
            maux_nb_leave();
    maux_nb_end
}

static void stacktrace_name(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);

            mapi_push_arg(U, 0);
            mapi_exception_stacktrace_name(U);
            maux_nb_return();
    maux_nb_end
}

static void stacktrace_size(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);

            mapi_push_arg(U, 0);
            ml_size size = mapi_exception_stacktrace_size(U);
            mapi_push_size(U, size, "size");
            maux_nb_return();
    maux_nb_end
}

static void stacktrace_get(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 2);

            mapi_push_arg(U, 1);
            ml_size index = mapi_get_size(U, "index");

            mapi_push_arg(U, 0);
            const char *type = mapi_exception_stacktrace_type(U, index);
            size_t position = mapi_exception_stacktrace_line(U, index);
            size_t state = mapi_exception_stacktrace_state(U, index);
            mapi_pop(U, 1);

            mapi_push_table(U);

            mapi_push_string(U, "callable");
            mapi_push_string(U, type);
            mapi_table_set(U);

            mapi_push_string(U, "name");
            mapi_push_arg(U, 0);
            mapi_exception_stacktrace_callable(U, index);
            mapi_rotate(U, 2);
            mapi_pop(U, 1);
            mapi_table_set(U);

            mapi_push_string(U, "position");
            mapi_push_size(U, position, "position");
            mapi_table_set(U);

            mapi_push_string(U, "state");
            mapi_push_size(U, state, "state");
            mapi_table_set(U);

            maux_nb_return();
    maux_nb_end
}

static maux_construct_element_t elements[] = {
    MAUX_CONSTRUCT_FUNCTION("create", create),
    MAUX_CONSTRUCT_FUNCTION("value", value),
    MAUX_CONSTRUCT_FUNCTION("message", message),
    MAUX_CONSTRUCT_FUNCTION("print", print),
    MAUX_CONSTRUCT_FUNCTION("error.print", error_print),
    MAUX_CONSTRUCT_FUNCTION("stacktrace.print", stacktrace_print),
    MAUX_CONSTRUCT_FUNCTION("stacktrace.record", stacktrace_record),
    MAUX_CONSTRUCT_FUNCTION("stacktrace.name", stacktrace_name),
    MAUX_CONSTRUCT_FUNCTION("stacktrace.size", stacktrace_size),
    MAUX_CONSTRUCT_FUNCTION("stacktrace.get", stacktrace_get),
    MAUX_CONSTRUCT_END
};

static void library_init(morphine_coroutine_t U) {
    maux_construct(U, elements);
}

MORPHINE_LIB morphine_library_t mlib_builtin_exception(void) {
    return (morphine_library_t) {
        .name = "exception",
        .sharedkey = NULL,
        .init = library_init
    };
}
