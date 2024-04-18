//
// Created by whyiskra on 3/24/24.
//

#include <dlfcn.h>
#include <string.h>
#include "dlibcompiler.h"

struct compiler_instance {
    void *dlib;

    struct {
        bool dlib;
        bool graal;
    } inited;

    struct {
        graal_isolate_t *isolate;
        graal_isolatethread_t *thread;
    } vars;

    struct {
        graal_create_isolate_fn_t graal_create_isolate;
        graal_tear_down_isolate_fn_t graal_tear_down_isolate;

        libcompiler_assemble_fn_t assemble;
        libcompiler_geterror_fn_t geterror;
        libcompiler_getbytecodesize_fn_t getbytecodesize;
        libcompiler_getbytecodevector_fn_t getbytecodevector;
        libcompiler_version_fn_t version;
        libcompiler_versioncode_fn_t versioncode;
    } methods;
};

static void userdata_free(morphine_instance_t I, void *p) {
    (void) (I);

    struct compiler_instance *instance = p;

    if (instance->inited.graal) {
        instance->methods.graal_tear_down_isolate(instance->vars.thread);
    }

    if (instance->inited.dlib) {
        dlclose(instance->dlib);
    }
}


static void *safedlsym(morphine_coroutine_t U, void *dlib, const char *name) {
    void *result = dlsym(dlib, name);
    if (result == NULL) {
        mapi_errorf(U, "Cannot get function %s from dynamic library", name);
    }

    return result;
}

static void dlib_open(morphine_coroutine_t U, struct compiler_instance *instance, const char *path) {
    void *dlib = dlopen(path, RTLD_LAZY);
    if (dlib == NULL) {
        mapi_errorf(U, "Cannot open library %s", path);
    }

    instance->dlib = dlib;
    instance->inited.dlib = true;
}

static void graal_init(morphine_coroutine_t U, struct compiler_instance *instance) {
    graal_create_isolate_fn_t graal_create_isolate = safedlsym(U, instance->dlib, "graal_create_isolate");
    instance->methods.graal_tear_down_isolate = safedlsym(U, instance->dlib, "graal_tear_down_isolate");

    instance->methods.assemble = safedlsym(U, instance->dlib, "libcompiler_assemble");
    instance->methods.geterror = safedlsym(U, instance->dlib, "libcompiler_geterror");
    instance->methods.getbytecodesize = safedlsym(U, instance->dlib, "libcompiler_getbytecodesize");
    instance->methods.getbytecodevector = safedlsym(U, instance->dlib, "libcompiler_getbytecodevector");
    instance->methods.version = safedlsym(U, instance->dlib, "libcompiler_version");
    instance->methods.versioncode = safedlsym(U, instance->dlib, "libcompiler_versioncode");

    if (graal_create_isolate(NULL, &instance->vars.isolate, &instance->vars.thread) != 0) {
        mapi_errorf(U, "Cannot initialize compiler");
    }

    instance->inited.graal = true;
}

static void check_graal_inited(morphine_coroutine_t U, struct compiler_instance *instance) {
    if (!instance->inited.graal) {
        mapi_errorf(U, "Compiler isn't initialized");
    }
}

static void check_version(morphine_coroutine_t U, struct compiler_instance *instance) {
    check_graal_inited(U, instance);

    const char *version = instance->methods.version(instance->vars.thread);
    int code = instance->methods.versioncode(instance->vars.thread);
    if (version == NULL || strcmp(mapi_version(), version) != 0 || code != mapi_version_code()) {
        mapi_errorf(U, "Unsupported compiler version");
    }
}

struct compiler_instance *dlibcompiler_userdata(morphine_coroutine_t U, const char *path) {
    struct compiler_instance *instance = mapi_push_userdata(
        U, "dlibcompiler",
        sizeof(struct compiler_instance),
        NULL, userdata_free
    );

    (*instance) = (struct compiler_instance) {
        .inited.dlib = false,
        .inited.graal = false
    };

    dlib_open(U, instance, path);
    graal_init(U, instance);
    check_version(U, instance);

    return instance;
}

bool dlibcompiler_assemble(
    morphine_coroutine_t U, struct compiler_instance *instance, char *text, bool optimize
) {
    check_graal_inited(U, instance);
    return instance->methods.assemble(instance->vars.thread, text, optimize);
}

const char *dlibcompiler_geterror(morphine_coroutine_t U, struct compiler_instance *instance) {
    check_graal_inited(U, instance);
    return instance->methods.geterror(instance->vars.thread);
}

int dlibcompiler_getbytecodesize(morphine_coroutine_t U, struct compiler_instance *instance) {
    check_graal_inited(U, instance);
    return instance->methods.getbytecodesize(instance->vars.thread);
}

const uint8_t *dlibcompiler_getbytecodevector(morphine_coroutine_t U, struct compiler_instance *instance) {
    check_graal_inited(U, instance);
    return (const uint8_t *) instance->methods.getbytecodevector(instance->vars.thread);
}

const char *dlibcompiler_version(morphine_coroutine_t U, struct compiler_instance *instance) {
    check_graal_inited(U, instance);
    return instance->methods.version(instance->vars.thread);
}

int dlibcompiler_versioncode(morphine_coroutine_t U, struct compiler_instance *instance) {
    check_graal_inited(U, instance);
    return instance->methods.versioncode(instance->vars.thread);
}
