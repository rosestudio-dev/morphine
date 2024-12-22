//
// Created by whyiskra on 1/21/24.
//

#include "morphine/core/instance.h"
#include "morphine/object/table.h"
#include "morphine/object/sio.h"
#include "morphine/object/native.h"
#include "morphine/gc/finalizer.h"
#include "morphine/gc/control.h"
#include "morphine/api.h"
#include "morphine/auxiliary.h"
#include "morphine/libs/builtin.h"

static morphine_library_t (*builtins[])(void) = {
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
    mlib_builtin_exception,
    mlib_builtin_sharedstorage,
    mlib_builtin_iterator,
    mlib_builtin_assertion,
};

static void library(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);

            const char *name = mapi_get_cstr(U);
            maux_library_access(U, name);
            maux_nb_return();
    maux_nb_end
}

static void init_sio(morphine_instance_t I) {
    I->sio.io = sioI_create(I, I->platform.sio_io_interface);
    I->sio.error = sioI_create(I, I->platform.sio_error_interface);

    sioI_hold(I, I->sio.io, valueI_raw(0));
    sioI_hold(I, I->sio.error, valueI_raw(0));

    sioI_open(I, I->sio.io, I->data);
    sioI_open(I, I->sio.error, I->data);
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

    struct string *name = stringI_create(I, "library");
    struct value native = valueI_object(nativeI_create(I, name, library));
    tableI_set(I, I->env, valueI_object(name), native);
}

static void init(morphine_instance_t I) {
    init_sio(I);
    init_throw(I);
    init_env(I);
    init_localstorage(I);
    init_sharedstorage(I);
    init_metatables(I);
    init_libraries(I);
}

morphine_instance_t instanceI_open(morphine_platform_t platform, morphine_settings_t settings, void *data) {
    if (sizeof(struct instance) >= settings.gc.limit) {
        platform.functions.signal(NULL);
    }

    morphine_instance_t I = platform.functions.malloc(data, sizeof(struct instance));

    if (I == NULL) {
        platform.functions.signal(NULL);
    }

    *I = (struct instance) {
        .platform = platform,
        .settings = settings,
        .interpreter = interpreterI_prototype(),
        .throw = throwI_prototype(),
        .libraries = librariesI_prototype(),
        .usertypes = usertypeI_prototype(),
        .data = data,
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
    if (I->sio.io != NULL) {
        sioI_close(I, I->sio.io, true);
    }

    if (I->sio.error != NULL) {
        sioI_close(I, I->sio.error, true);
    }

    gcI_destruct(I, I->G);

    librariesI_free(I, &I->libraries);
    usertypeI_free(I, &I->usertypes);

    I->platform.functions.free(I->data, I);
}
