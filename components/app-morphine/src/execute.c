//
// Created by whyiskra on 3/16/24.
//

#include <execute.h>
#include <loaders.h>
#include <stdlib.h>
#include <setjmp.h>
#include <stdio.h>

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

void execute(
    struct libcompiler *libcompiler,
    struct allocator *allocator,
    const char *path,
    bool binary,
    size_t alloc_limit,
    size_t argc,
    char **args
) {
    if (setjmp(abort_jmp) != 0) {
        return;
    }

    struct vmdata data = {
        .libcompiler = libcompiler
    };

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
        .close = NULL
    };

    morphine_sio_interface_t error_interface = {
        .write = io_error_write,
        .read = NULL,
        .flush = NULL,
        .open = NULL,
        .close = NULL
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

    morphine_instance_t I = mapi_open(instance_platform, settings, &data);
    morphine_coroutine_t U = mapi_coroutine(I);

    ml_size argc_size = mapi_csize2size(U, argc);

    mapi_push_env(U);
    mapi_push_string(U, "args");
    mapi_push_vector(U, argc_size);
    for (ml_size i = 0; i < argc_size; i++) {
        mapi_push_string(U, args[i]);
        mapi_vector_set(U, i);
    }
    mapi_table_set(U);

    load_program(U, path, binary);
    mapi_call(U, 0);

    mapi_interpreter(I);
    mapi_close(I);
}