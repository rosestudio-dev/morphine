//
// Created by whyiskra on 3/16/24.
//

#include <execute.h>
#include <morphinec/lib.h>
#include <loaders.h>
#include <stdlib.h>
#include <setjmp.h>
#include <stdio.h>
#include "morphinec/decompiler.h"

struct require_loader userlibs[] = {
    { "compiler", mclib_compiler_loader },
    { NULL, NULL }
};

static struct allocator *pallocator;
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

static void *dalloc(size_t size) {
    return allocator_alloc(pallocator, size);
}

static void *drealloc(void *ptr, size_t size) {
    return allocator_realloc(pallocator, ptr, size);
}

static void dfree(void *ptr) {
    allocator_free(pallocator, ptr);
}

static void load_program(morphine_coroutine_t U, const char *path, bool binary) {
    if (path == NULL) {
        mapi_errorf(U, "Empty file path");
    } else if (binary) {
        loader_binary_file(U, path);
    } else {
        loader_source_file(U, path);
    }
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

void execute(
    struct allocator *allocator,
    const char *path,
    bool binary,
    bool run,
    bool export,
    bool decompile,
    size_t alloc_limit,
    size_t argc,
    char **args
) {
    (void) export;
    if (setjmp(abort_jmp) != 0) {
        return;
    }

    struct settings settings = {
        .gc.limit_bytes = alloc_limit,
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

    struct platform instance_platform = {
        .functions.malloc = malloc,
        .functions.realloc = realloc,
        .functions.free = free,
        .functions.signal = signal,
        .sio_io_interface = io_interface,
        .sio_error_interface = error_interface,
    };

    if (allocator != NULL) {
        pallocator = allocator;
        instance_platform.functions.malloc = dalloc;
        instance_platform.functions.realloc = drealloc;
        instance_platform.functions.free = dfree;
    }

    morphine_instance_t I = mapi_open(instance_platform, settings, NULL);
    mapi_userlibs(I, userlibs);

    morphine_coroutine_t U = mapi_coroutine(I);

    init_args(U, argc, args);
    load_program(U, path, binary);

    if (decompile) {
        ml_size count = mapi_vector_len(U);
        for (ml_size i = 0; i < count; i++) {
            mapi_vector_get(U, i);
            mapi_push_sio_io(U);
            mapi_rotate(U, 2);

            mcapi_decompile(U);

            mapi_pop(U, 1);
            mapi_sio_print(U, "\n");
            mapi_pop(U, 1);
        }
    }

    if (run) {
        mapi_vector_peek(U);

        mapi_rotate(U, 2);
        mapi_pop(U, 1);

        mapi_call(U, 0);
    }

    mapi_interpreter(I);
    mapi_close(I);
}