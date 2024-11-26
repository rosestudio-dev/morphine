//
// Created by whyiskra on 30.01.24.
//

#include <stdio.h>
#include <setjmp.h>
#include <morphine.h>
#include <morphinec.h>
#include <morphinel.h>
#include <malloc.h>
#include <scripts.h>

#include "env.h"
#include "library.h"

static size_t io_write(morphine_sio_accessor_t A, void *data, const uint8_t *buffer, size_t size) {
    (void) A;
    (void) data;

    return fwrite(buffer, 1, size, stdout);
}

static size_t io_read(morphine_sio_accessor_t A, void *data, uint8_t *buffer, size_t size) {
    (void) A;
    (void) data;

    return fread(buffer, 1, size, stdin);
}

static void io_flush(morphine_sio_accessor_t A, void *data) {
    (void) A;
    (void) data;

    fflush(stdout);
}

static size_t io_error_write(morphine_sio_accessor_t A, void *data, const uint8_t *buffer, size_t size) {
    (void) A;
    (void) data;

    fwrite(buffer, 1, size, stderr);
    return size;
}

static void io_error_flush(morphine_sio_accessor_t A, void *data) {
    (void) A;
    (void) data;

    fflush(stderr);
}

static void *vmmalloc(void *data, size_t size) {
    (void) data;
    return malloc(size);
}

static void *vmrealloc(void *data, void *pointer, size_t size) {
    (void) data;
    return realloc(pointer, size);
}

static void vmfree(void *data, void *pointer) {
    (void) data;
    free(pointer);
}

static void init_args(morphine_coroutine_t U, int argc, char **args) {
    if (argc < 0) {
        argc = 0;
    }

    ml_size argc_size = mapi_csize2size(U, (size_t) argc, NULL);

    mapi_push_env(U);
    mapi_push_string(U, "args");
    mapi_push_vector(U, argc_size);
    for (ml_size i = 0; i < argc_size; i++) {
        mapi_push_string(U, args[i]);
        mapi_vector_set(U, i);
    }
    mapi_table_set(U);
    mapi_pop(U, 1);
}

static int launcher(struct env *env, int argc, char **argv) {
    if (setjmp(env->exit.jmp) == 1) {
        if (env->exit.I) {
            mapi_close(env->exit.I);
        }

        return env->exit.code;
    }

    morphine_settings_t settings = {
        .gc.limit = 8 * 1024 * 1024,
        .gc.threshold = 16384,
        .gc.grow = 150,
        .gc.deal = 200,
        .gc.pause = 13,
        .gc.cache.callinfo = 16,
        .coroutines.stack.limit = 65536
    };

    morphine_platform_t instance_platform = {
        .functions.malloc = vmmalloc,
        .functions.realloc = vmrealloc,
        .functions.free = vmfree,
        .functions.signal = env_signal,
        .sio_io_interface = maux_sio_interface_srwf(io_read, io_write, io_flush),
        .sio_error_interface = maux_sio_interface_swf(io_error_write, io_error_flush),
    };

    morphine_instance_t I = mapi_open(instance_platform, settings, env);
    mlapi_import_all(I);
    mapi_library_load(I, mclib_compiler());
    mapi_library_load(I, mllib_launcher());

    morphine_coroutine_t U = mapi_coroutine(I, "app");

    init_args(U, argc, argv);

    mapi_push_stringn(U, launcher_data, launcher_size);
    mcapi_compile(U, "launcher", false);

    mapi_call(U, 0);
    mapi_interpreter(I);

    mapi_close(I);

    return 0;
}

int main(int argc, char **argv) {
    struct env environment;

    if (setjmp(environment.abort_jmp) == 1) {
        return 1;
    }

    return launcher(&environment, argc, argv);
}
