//
// Created by whyiskra on 25.12.23.
//

#include <morphine.h>
#include <stdio.h>
#include "morphine/libs/loader.h"

static void version(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            maux_expect_args(U, 0);
            mapi_push_string(U, mapi_version());
            nb_return();
    nb_end
}

static void print(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            maux_expect_args(U, 1);

            mapi_push_arg(U, 0);
            mlib_value_call(U, "tostr", 1);
        nb_state(1)
            mapi_push_result(U);

            FILE *out = mapi_io_out(mapi_instance(U));
            if (out != NULL) {
                fprintf(out, "%s", mapi_get_string(U));
            }

            mapi_pop(U, 1);
            nb_leave();
    nb_end
}

static void println(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            size_t args = mapi_args(U);
            if (args == 0) {
                FILE *out = mapi_io_out(mapi_instance(U));
                if (out != NULL) {
                    fprintf(out, "\n");
                }
            } else {
                maux_expect_args(U, 1);
                mapi_push_arg(U, 0);
            }

            mlib_value_call(U, "tostr", 1);
        nb_state(1)
            mapi_push_result(U);

            FILE *out = mapi_io_out(mapi_instance(U));
            if (out != NULL) {
                fprintf(out, "%s\n", mapi_get_string(U));
            }

            mapi_pop(U, 1);
            nb_leave();
        nb_state(3)
    nb_end
}

static void setmetatable(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            maux_expect_args(U, 2);

            mapi_push_arg(U, 0);
            maux_expect(U, "meta");
            mapi_push_arg(U, 1);
            maux_expect(U, "table");

            mapi_set_metatable(U);
            nb_return();
    nb_end
}

static void getmetatable(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            maux_expect_args(U, 1);

            mapi_push_arg(U, 0);
            maux_expect(U, "meta");

            mapi_get_metatable(U);
            nb_return();
    nb_end
}

static void setdefaultmetatable(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            maux_expect_args(U, 2);

            mapi_push_arg(U, 0);
            const char *type = mapi_type(U);
            mapi_pop(U, 1);
            mapi_push_arg(U, 1);
            maux_expect(U, "table");
            mapi_set_default_metatable(U, type);
            nb_return();
    nb_end
}

static void getdefaultmetatable(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            maux_expect_args(U, 1);

            mapi_push_arg(U, 0);
            const char *type = mapi_type(U);
            mapi_pop(U, 1);
            mapi_get_default_metatable(U, type);
            nb_return();
    nb_end
}

static void scall(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            size_t count = mapi_args(U);

            mapi_push_arg(U, 1);
            mapi_push_arg(U, 0);
            maux_expect(U, "callable");

            for (size_t i = 2; i < count; i++) {
                mapi_push_arg(U, i);
            }

            mapi_callself(U, count - 2);
        nb_state(1)
            mapi_push_result(U);
            nb_return();
    nb_end
}

static void pcall(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            size_t count = mapi_args(U);

            mapi_push_arg(U, 0);
            maux_expect(U, "callable");

            for (size_t i = 1; i < count; i++) {
                mapi_push_arg(U, i);
            }

            mapi_catchable(U, 2);
            mapi_call(U, count - 1);
        nb_state(1)
            mapi_push_table(U);

            mapi_push_stringf(U, "result");
            mapi_push_result(U);
            mapi_table_set(U);

            mapi_push_stringf(U, "error");
            mapi_push_nil(U);
            mapi_table_set(U);

            nb_return();
        nb_state(2)
            mapi_push_table(U);

            mapi_push_stringf(U, "result");
            mapi_push_nil(U);
            mapi_table_set(U);

            mapi_push_stringf(U, "error");
            mapi_push_thrown(U);
            mapi_table_set(U);

            nb_return();
    nb_end
}

static void pscall(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            size_t count = mapi_args(U);

            mapi_push_arg(U, 1);
            mapi_push_arg(U, 0);
            maux_expect(U, "callable");

            for (size_t i = 2; i < count; i++) {
                mapi_push_arg(U, i);
            }

            mapi_catchable(U, 2);
            mapi_callself(U, count - 2);
        nb_state(1)
            mapi_push_table(U);

            mapi_push_stringf(U, "result");
            mapi_push_result(U);
            mapi_table_set(U);

            mapi_push_stringf(U, "error");
            mapi_push_nil(U);
            mapi_table_set(U);

            nb_return();
        nb_state(2)
            mapi_push_table(U);

            mapi_push_stringf(U, "result");
            mapi_push_nil(U);
            mapi_table_set(U);

            mapi_push_stringf(U, "error");
            mapi_push_thrown(U);
            mapi_table_set(U);

            nb_return();
    nb_end
}

static void error(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            mapi_error(U, NULL);
            nb_leave();
    nb_end
}

static void changeenv(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            mapi_change_env(U);
            nb_leave();
    nb_end
}

static void load(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            maux_expect(U, "string");
            size_t len = mapi_string_len(U);
            const uint8_t *vector = (const uint8_t *) mapi_get_string(U);
            mapi_rload(U, len, vector);
            nb_leave();
    nb_end
}

static struct maux_construct_field table[] = {
    { "version",             version },
    { "print",               print },
    { "println",             println },
    { "setmetatable",        setmetatable },
    { "getmetatable",        getmetatable },
    { "setdefaultmetatable", setdefaultmetatable },
    { "getdefaultmetatable", getdefaultmetatable },
    { "scall",               scall },
    { "pcall",               pcall },
    { "pscall",              pscall },
    { "error",               error },
    { "changeenv",           changeenv },
    { "load",                load },
    { NULL, NULL }
};

void mlib_base_loader(morphine_coroutine_t U) {
    maux_construct(U, table, "base.");
}

MORPHINE_LIB void mlib_base_call(morphine_coroutine_t U, const char *name, size_t argc) {
    maux_construct_call(U, table, "base.", name, argc);
}
