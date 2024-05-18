//
// Created by whyiskra on 3/24/24.
//

#include <dlfcn.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "compiler.h"
#include "execute.h"

typedef libsharedcompiler_ExportedSymbols *(get_symbols_t)(void);

struct compiler_instance {
    bool inited;
    libsharedcompiler_ExportedSymbols *symbols;
    libsharedcompiler_kref_morphine_library_NativeCompiler compiler;
};

static void *safedlsym(void *dlib, const char *name) {
    void *result = dlsym(dlib, name);
    if (result == NULL) {
        fprintf(stderr, "Cannot get symbol %s\n", name);
        exit(1);
    }

    return result;
}

static libsharedcompiler_ExportedSymbols *dlib_open(const char *path) {
    void *dlib = dlopen(path, RTLD_NOW);
    if (dlib == NULL) {
        return NULL;
    }

    get_symbols_t *get_symbols = safedlsym(dlib, "libsharedcompiler_symbols");

    return get_symbols();
}

struct libcompiler libcompiler_open(const char *path) {
    return (struct libcompiler) {
        .symbols = dlib_open(path)
    };
}

static void userdata_free(morphine_instance_t I, void *p) {
    (void) (I);

    struct compiler_instance *instance = p;

    if (instance->inited) {
        instance->symbols->kotlin.root.morphine.library.NativeCompiler.release(instance->compiler);
        instance->symbols->DisposeStablePointer(instance->compiler.pinned);
        instance->inited = false;
    }
}

static void kotlin_init(struct compiler_instance *instance) {
    instance->compiler = instance->symbols->kotlin.root.morphine.library.NativeCompiler.NativeCompiler();
    instance->inited = true;
}

static void check_inited(morphine_coroutine_t U, struct compiler_instance *instance) {
    if (!instance->inited) {
        mapi_errorf(U, "Compiler isn't initialized");
    }
}

static void check_version(morphine_coroutine_t U, struct compiler_instance *instance) {
    check_inited(U, instance);

    const char *version = instance->symbols->kotlin.root.morphine.library.NativeCompiler.version(instance->compiler);
    int code = instance->symbols->kotlin.root.morphine.library.NativeCompiler.versionCode(instance->compiler);
    if (version == NULL || strcmp(mapi_version(), version) != 0 || code != mapi_version_code()) {
        mapi_errorf(U, "Unsupported compiler version");
    }
}

struct compiler_instance *libcompiler_userdata(morphine_coroutine_t U) {
    struct compiler_instance *instance = mapi_push_userdata(
        U, "libcompiler", sizeof(struct compiler_instance)
    );

    struct vmdata *data = mapi_instance_data(mapi_instance(U));

    (*instance) = (struct compiler_instance) {
        .inited = false,
        .symbols = data->libcompiler->symbols
    };

    mapi_userdata_set_free(U, userdata_free);

    if (instance->symbols == NULL) {
        mapi_errorf(U, "Compiler isn't initialized");
    }

    kotlin_init(instance);
    check_version(U, instance);

    return instance;
}

bool libcompiler_compile(
    morphine_coroutine_t U, struct compiler_instance *instance, char *text, bool optimize
) {
    check_inited(U, instance);
    return instance->symbols->kotlin.root.morphine.library.NativeCompiler.compile(
        instance->compiler,
        text,
        optimize
    );
}

const char *libcompiler_get_error(morphine_coroutine_t U, struct compiler_instance *instance) {
    check_inited(U, instance);
    return instance->symbols->kotlin.root.morphine.library.NativeCompiler.getError(instance->compiler);
}

int libcompiler_get_bytecode_size(morphine_coroutine_t U, struct compiler_instance *instance) {
    check_inited(U, instance);
    return instance->symbols->kotlin.root.morphine.library.NativeCompiler.getBytecodeSize(instance->compiler);
}

const uint8_t *libcompiler_get_bytecode(morphine_coroutine_t U, struct compiler_instance *instance) {
    check_inited(U, instance);
    return (const uint8_t *) instance->symbols->kotlin.root.morphine.library.NativeCompiler.getBytecode(
        instance->compiler
    );
}

const char *libcompiler_version(morphine_coroutine_t U, struct compiler_instance *instance) {
    check_inited(U, instance);
    return instance->symbols->kotlin.root.morphine.library.NativeCompiler.version(instance->compiler);
}

int libcompiler_version_code(morphine_coroutine_t U, struct compiler_instance *instance) {
    check_inited(U, instance);
    return instance->symbols->kotlin.root.morphine.library.NativeCompiler.versionCode(instance->compiler);
}

void libcompiler_release(morphine_coroutine_t U, struct compiler_instance *instance) {
    check_inited(U, instance);
    instance->symbols->kotlin.root.morphine.library.NativeCompiler.release(instance->compiler);
}
