//
// Created by whyiskra on 30.01.24.
//

#include <stdio.h>
#include <setjmp.h>
#include <morphine.h>
#include <morphinec.h>
#include <malloc.h>

struct environment {
    jmp_buf abort_jmp;
};

morphine_noret static void signal(morphine_instance_t I) {
    struct environment *env = mapi_instance_data(I);
    const char *message = mapi_signal_message(I);
    printf("morphine panic: %s\n", message);

    if (I != NULL && !mapi_is_nested_signal(I)) {
        mapi_close(I);
    }

    longjmp(env->abort_jmp, 1);
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

static void *vmmalloc(void *data, size_t size) { (void) data; return malloc(size); }
static void *vmrealloc(void *data, void *pointer, size_t size) { (void) data; return realloc(pointer, size); }
static void vmfree(void *data, void *pointer) { (void) data; free(pointer); }

static void launcher(struct environment *env, const char *text, size_t size) {
    morphine_settings_t settings = {
        .gc.limit_bytes = 64 * 1024 * 1024,
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

    morphine_sio_interface_t io_interface = {
        .write = io_write,
        .read = NULL,
        .flush = NULL,
        .open = NULL,
        .close = NULL,
        .seek = NULL,
        .tell = NULL,
        .eos = NULL
    };

    morphine_sio_interface_t error_interface = {
        .write = io_error_write,
        .read = NULL,
        .flush = NULL,
        .open = NULL,
        .close = NULL,
        .seek = NULL,
        .tell = NULL,
        .eos = NULL
    };

    morphine_platform_t instance_platform = {
        .functions.malloc = vmmalloc,
        .functions.realloc = vmrealloc,
        .functions.free = vmfree,
        .functions.signal = signal,
        .sio_io_interface = io_interface,
        .sio_error_interface = error_interface,
    };

    morphine_instance_t I = mapi_open(instance_platform, settings, env);
    mapi_library_load(I, mclib_compiler());

    morphine_coroutine_t U = mapi_coroutine(I);

    mapi_push_stringn(U, text, size);
    mcapi_compile(U, "main", false);

    mapi_call(U, 0);
    mapi_interpreter(I);

    mapi_close(I);
}

extern int morphine(const char *text, size_t size) {
    struct environment environment;
    if (setjmp(environment.abort_jmp) == 1) {
        return 1;
    }

    launcher(&environment, text, size);
    return 0;
}
