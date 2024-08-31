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

struct environment {
    jmp_buf abort_jmp;

    struct {
        int code;
        morphine_instance_t I;
        jmp_buf jmp;
    } exit;
};

morphine_noret static void signal(morphine_instance_t I) {
    struct environment *env = mapi_instance_data(I);
    const char *message = mapi_signal_message(I);
    fprintf(stderr, "morphine panic: %s\n", message);

    if (I != NULL && !mapi_is_nested_signal(I)) {
        mapi_close(I);
    }

    longjmp(env->abort_jmp, 1);
}

morphine_noret static void cexit(morphine_instance_t I, ml_integer code) {
    struct environment *env = mapi_instance_data(I);

    env->exit.I = I;
    env->exit.code = (int) code;

    longjmp(env->exit.jmp, 1);
}

static size_t io_write(morphine_sio_accessor_t A, void *data, const uint8_t *buffer, size_t size) {
    (void) A;
    (void) data;

    fwrite(buffer, size, 1, stdout);
    return size;
}

static size_t io_error_write(morphine_sio_accessor_t A, void *data, const uint8_t *buffer, size_t size) {
    (void) A;
    (void) data;

    fwrite(buffer, size, 1, stderr);
    return size;
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

static void exit_function(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            ml_integer code = mapi_get_integer(U);
            cexit(mapi_instance(U), code);
    maux_nb_end
}

static void init_args(morphine_coroutine_t U, size_t argc, char **args) {
    ml_size argc_size = mapi_csize2size(U, argc);

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

static void init_functions(morphine_coroutine_t U) {
    mapi_push_env(U);

    mapi_push_string(U, "exit");
    mapi_push_native(U, "exit", exit_function);
    mapi_table_set(U);

    mapi_pop(U, 1);
}

static int launcher(struct environment *env, int argc, char **argv) {
    if (setjmp(env->exit.jmp) == 1) {
        if (env->exit.I) {
            mapi_close(env->exit.I);
        }

        return env->exit.code;
    }

    morphine_settings_t settings = {
        .gc.limit_bytes = 8 * 1024 * 1024,
        .gc.threshold = 16384,
        .gc.grow = 150,
        .gc.deal = 200,
        .gc.pause = 13,
        .gc.cache_callinfo_holding = 16,
        .finalizer.stack_limit = 256,
        .finalizer.stack_grow = 32,
        .states.stack_limit = 4096,
        .states.stack_grow = 64,
    };

    morphine_platform_t instance_platform = {
        .functions.malloc = vmmalloc,
        .functions.realloc = vmrealloc,
        .functions.free = vmfree,
        .functions.signal = signal,
        .sio_io_interface = maux_sio_interface_swo(io_write),
        .sio_error_interface = maux_sio_interface_swo(io_error_write),
    };

    morphine_instance_t I = mapi_open(instance_platform, settings, env);
    mapi_library_load(I, mclib_compiler());
    mapi_library_load(I, mllib_math());
    mapi_library_load(I, mllib_fs());

    morphine_coroutine_t U = mapi_coroutine(I);

    init_args(U, (size_t) argc, argv);
    init_functions(U);

    mapi_push_stringn(U, launcher_data, launcher_size);
    mcapi_compile(U, "launcher", false);

    mapi_call(U, 0);
    mapi_interpreter(I);

    mapi_close(I);

    return 0;
}

int main(int argc, char **argv) {
    struct environment environment = {
        .exit.I = NULL,
        .exit.code = 0,
    };

    if (setjmp(environment.abort_jmp) == 1) {
        return 1;
    }

    return launcher(&environment, argc, argv);
}
