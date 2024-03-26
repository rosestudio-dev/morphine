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
    } methods;
};

static void userdata_free(morphine_instance_t I, void *p) {
    struct compiler_instance *instance = p;

    if (instance->inited.graal) {
        instance->methods.graal_tear_down_isolate(instance->vars.thread);
    }

    if (instance->inited.dlib) {
        dlclose(instance->dlib);
    }

    mapi_allocator_free(I, instance);
}


static void *safedlsym(morphine_state_t S, void *dlib, const char *name) {
    void *result = dlsym(dlib, name);
    if (result == NULL) {
        mapi_errorf(S, "Cannot get function %s from dynamic library", name);
    }

    return result;
}

static void dlib_open(morphine_state_t S, struct compiler_instance *instance, const char *path) {
    void *dlib = dlopen(path, RTLD_LAZY);
    if (dlib == NULL) {
        mapi_errorf(S, "Cannot open library %s", path);
    }

    instance->dlib = dlib;
    instance->inited.dlib = true;
}

static void graal_init(morphine_state_t S, struct compiler_instance *instance) {
    graal_create_isolate_fn_t graal_create_isolate = safedlsym(S, instance->dlib, "graal_create_isolate");
    instance->methods.graal_tear_down_isolate = safedlsym(S, instance->dlib, "graal_tear_down_isolate");

    instance->methods.assemble = safedlsym(S, instance->dlib, "libcompiler_assemble");
    instance->methods.geterror = safedlsym(S, instance->dlib, "libcompiler_geterror");
    instance->methods.getbytecodesize = safedlsym(S, instance->dlib, "libcompiler_getbytecodesize");
    instance->methods.getbytecodevector = safedlsym(S, instance->dlib, "libcompiler_getbytecodevector");
    instance->methods.version = safedlsym(S, instance->dlib, "libcompiler_version");

    if (graal_create_isolate(NULL, &instance->vars.isolate, &instance->vars.thread) != 0) {
        mapi_errorf(S, "Cannot initialize compiler");
    }

    instance->inited.graal = true;
}

static void check_graal_inited(morphine_state_t S, struct compiler_instance *instance) {
    if (!instance->inited.graal) {
        mapi_errorf(S, "Compiler isn't initialized");
    }
}

static void check_version(morphine_state_t S, struct compiler_instance *instance) {
    check_graal_inited(S, instance);

    const char *version = instance->methods.version(instance->vars.thread);
    if (version == NULL || strcmp(mapi_version(), version) != 0) {
        mapi_errorf(S, "Unsupported compiler version");
    }
}

struct compiler_instance *dlibcompiler_userdata(morphine_state_t S, const char *path) {
    struct compiler_instance *instance = mapi_allocator_uni(mapi_instance(S), NULL, sizeof(struct compiler_instance));
    (*instance) = (struct compiler_instance) {
        .inited.dlib = false,
        .inited.graal = false
    };

    mapi_push_userdata(S, "dlibcompiler", instance, NULL, userdata_free);

    dlib_open(S, instance, path);
    graal_init(S, instance);
    check_version(S, instance);

    return instance;
}

bool dlibcompiler_assemble(morphine_state_t S, struct compiler_instance *instance, char *text, bool optimize) {
    check_graal_inited(S, instance);
    return instance->methods.assemble(instance->vars.thread, text, optimize);
}

const char *dlibcompiler_geterror(morphine_state_t S, struct compiler_instance *instance) {
    check_graal_inited(S, instance);
    return instance->methods.geterror(instance->vars.thread);
}

int dlibcompiler_getbytecodesize(morphine_state_t S, struct compiler_instance *instance) {
    check_graal_inited(S, instance);
    return instance->methods.getbytecodesize(instance->vars.thread);
}

const uint8_t *dlibcompiler_getbytecodevector(morphine_state_t S, struct compiler_instance *instance) {
    check_graal_inited(S, instance);
    return (const uint8_t *) instance->methods.getbytecodevector(instance->vars.thread);
}

const char *dlibcompiler_version(morphine_state_t S, struct compiler_instance *instance) {
    check_graal_inited(S, instance);
    return instance->methods.version(instance->vars.thread);
}
