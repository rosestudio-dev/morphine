//
// Created by whyiskra on 30.01.24.
//

#include <stdio.h>
#include <setjmp.h>
#include <morphine.h>
#include <morphinec.h>
#include <morphinec/disassembler.h>
#include <morphinel.h>
#include <malloc.h>
#include <memory.h>
#include <scripts.h>
#include "human.h"

static jmp_buf abort_jmp;

__attribute__((noreturn)) static void cabort(void) {
    longjmp(abort_jmp, 1);
}

morphine_noret static void signal(morphine_instance_t I) {
    const char *message = mapi_signal_message(I);
    printf("morphine panic: %s\n", message);

    if (I != NULL && !mapi_is_nested_signal(I)) {
        mapi_close(I);
    }

    cabort();
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

int main(int argc, char **argv) {
    if (setjmp(abort_jmp) == 1) {
        return 1;
    }

    morphine_settings_t settings = {
        .gc.limit_bytes = 256 * 1024 * 1024,
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

    morphine_instance_t I = mapi_open(instance_platform, settings, NULL);
    mapi_library_load(I, mclib_compiler());
    mapi_library_load(I, mllib_math());
    mapi_library_load(I, mllib_fs());

    morphine_coroutine_t U = mapi_coroutine(I);

    init_args(U, (size_t) argc, argv);

    mapi_push_stringn(U, launcher_data, launcher_size);
    mcapi_compile(U, "launcher");
    mapi_push_sio_io(U);
    mapi_peek(U, 1);
    mcapi_disassembly(U);

    mapi_call(U, 0);
    mapi_interpreter(I);

    char buffer[128];
    memset(buffer, 0, 128);
    human_size(mapi_gc_max_allocated(I), buffer, 127);
    printf("%s\n", buffer);

    mapi_close(I);

    return 0;
}
