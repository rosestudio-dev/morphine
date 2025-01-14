//
// Created by whyiskra on 1/21/24.
//

#include "morphine/core/instance.h"
#include "morphine/api.h"
#include "morphine/auxiliary.h"
#include "morphine/gc/control.h"
#include "morphine/gc/finalizer.h"
#include "morphine/libs/builtin.h"
#include "morphine/object/coroutine.h"
#include "morphine/object/native.h"
#include "morphine/object/stream.h"
#include "morphine/object/table.h"

static morphine_library_t (*builtins[])(void) = {
    mlib_builtin_base,
    mlib_builtin_value,
    mlib_builtin_gc,
    mlib_builtin_coroutine,
    mlib_builtin_string,
    mlib_builtin_table,
    mlib_builtin_userdata,
    mlib_builtin_vector,
    mlib_builtin_stream,
    mlib_builtin_packer,
    mlib_builtin_bitwise,
    mlib_builtin_function,
    mlib_builtin_closure,
    mlib_builtin_exception,
    mlib_builtin_sharedstorage,
    mlib_builtin_iterator,
    mlib_builtin_assertion,
    mlib_builtin_algorithm,
};

static void lib(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);

            const char *name = mapi_get_cstr(U);
            maux_library_access(U, name);
            maux_nb_return();
    maux_nb_end
}

static void init_stream(morphine_instance_t I) {
    I->stream.err = streamI_create(I, I->platform.stream.err, valueI_nil, I->data);
    I->stream.io = streamI_create(I, I->platform.stream.io, valueI_nil, I->data);
}

static void init_throw(morphine_instance_t I) {
    throwI_special(I);
}

static void init_env(morphine_instance_t I) {
    I->env = tableI_create(I);
    tableI_mode_lock(I, I->env);
}

static void init_localstorage(morphine_instance_t I) {
    I->localstorage = tableI_create(I);
    tableI_mode_lock(I, I->localstorage);
}

static void init_sharedstorage(morphine_instance_t I) {
    I->sharedstorage = tableI_create(I);
    tableI_mode_lock(I, I->sharedstorage);
}

static void init_metatables(morphine_instance_t I) {
    for (morphine_metatable_field_t field = MORPHINE_METATABLE_FIELDS_START;
         field < MORPHINE_METATABLE_FIELDS_COUNT; field++) {
        const char *name = metatableI_field2string(I, field);
        I->metatable.names[field] = stringI_create(I, name);
    }
}

static void init_libraries(morphine_instance_t I) {
    size_t size = sizeof(builtins) / sizeof(builtins[0]);
    for (size_t i = 0; i < size; i++) {
        librariesI_load(I, builtins[i]());
    }

    struct string *name = stringI_create(I, "lib");
    struct value native = valueI_object(nativeI_create(I, name, lib));
    tableI_set(I, I->env, valueI_object(name), native);
}

static void init_main_coroutine(morphine_instance_t I) {
    struct string *name = stringI_create(I, MPARAM_MAIN_COROUTINE_NAME);
    I->main = coroutineI_create(I, name, valueI_object(I->env));
}

static void init(morphine_instance_t I) {
    init_stream(I);
    init_throw(I);
    init_metatables(I);
    init_env(I);
    init_localstorage(I);
    init_sharedstorage(I);
    init_main_coroutine(I);
    init_libraries(I);
}

morphine_instance_t instanceI_open(morphine_platform_t platform, morphine_settings_t settings, void *data) {
    if (sizeof(struct instance) >= settings.gc.limit) {
        platform.signal(NULL, data, false);
    }

    morphine_instance_t I = platform.memory.alloc(data, sizeof(struct instance));

    if (I == NULL) {
        platform.signal(NULL, data, true);
    }

    *I = (struct instance) {
        .platform = platform,
        .settings = settings,
        .interpreter = interpreterI_prototype(),
        .throw = throwI_prototype(),
        .libraries = librariesI_prototype(),
        .usertypes = usertypeI_prototype(),
        .data = data,
        .main = NULL,
        .env = NULL,
        .localstorage = NULL,
        .sharedstorage = NULL,

#ifdef MORPHINE_ENABLE_SSO
        .sso = ssoI_prototype(),
#endif
    };

    gcI_prototype(I, sizeof(struct instance));
    init(I);
    gcI_init_finalizer(I);
    gcI_enable(I);

    return I;
}

void instanceI_close(morphine_instance_t I) {
    throwI_danger_enter(I);
    if (I->stream.io != NULL) {
        streamI_close(I, I->stream.io, true);
    }

    if (I->stream.err != NULL) {
        streamI_close(I, I->stream.err, true);
    }

    gcI_destruct(I, I->G);

    librariesI_free(I, &I->libraries);
    usertypeI_free(I, &I->usertypes);
    throwI_danger_exit(I);

    I->platform.memory.free(I->data, I);
}
