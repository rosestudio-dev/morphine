//
// Created by why-iskra on 30.03.2024.
//

#include "impl.h"
#include "morphine/core/instance.h"
#include "morphine/object/table.h"
#include "morphine/object/string.h"
#include "morphine/object/native.h"
#include "morphine/libs/builtin.h"
#include "morphine/auxiliary.h"
#include "morphine/api.h"

static morphine_library_t *(*builtins[])(void) = {
    mlib_builtin_base,
    mlib_builtin_value,
    mlib_builtin_gc,
    mlib_builtin_coroutine,
    mlib_builtin_string,
    mlib_builtin_table,
    mlib_builtin_userdata,
    mlib_builtin_vector,
    mlib_builtin_sio,
    mlib_builtin_binary,
    mlib_builtin_bitwise,
    mlib_builtin_function,
    mlib_builtin_closure,
};

static void library(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            bool reload = false;
            if (mapi_args(U) == 2) {
                mapi_push_arg(U, 1);
                reload = mapi_get_boolean(U);
            } else {
                maux_expect_args(U, 1);
            }

            mapi_push_arg(U, 0);
            const char *name = mapi_get_cstr(U);

            mapi_library(U, name, reload);
            maux_nb_return();
    maux_nb_end
}

static void init_library_function(morphine_instance_t I) {
    struct value name = valueI_object(stringI_create(I, "library"));
    struct value native = valueI_object(nativeI_create(I, "library", library));

    tableI_set(I, I->env, name, native);
}

static void init_builtin_functions(morphine_instance_t I) {
    size_t size = sizeof(builtins) / sizeof(builtins[0]);
    for (size_t i = 0; i < size; i++) {
        librariesI_load(I, builtins[i]());
    }
}

void init_libraries(morphine_instance_t I) {
    init_builtin_functions(I);
    init_library_function(I);
}