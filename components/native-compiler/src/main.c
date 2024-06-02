//
// Created by why-iskra on 19.05.2024.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <morphine.h>
#include <sys/time.h>
#include "lex.h"
#include "parser.h"
#include "printer.h"

static uint64_t millis(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);

    return (((uint64_t) tv.tv_sec) * 1000) + (((uint64_t) tv.tv_usec) / 1000);
}

static void test(morphine_coroutine_t U) {
    mapi_push_arg(U, 0);
    const char *text = mapi_get_string(U);
    uint64_t start = millis();

    strtable(U, 32);
    lex(U, text, strlen(text));
    parser(U);
    while (parser_next(U)) { }
    mapi_rotate(U, 5);
    mapi_pop(U, 2);
    mapi_rotate(U, 2);
    mapi_pop(U, 1);
    mapi_rotate(U, 2);

    uint64_t end = millis();

    mapi_rotate(U, 2);
    printer_strtable(U);

    mapi_rotate(U, 2);
    printer_ast(U);

    mapi_gc_full(mapi_instance(U));
    printf("\nmillis: %zu ms\n", end - start);
    printf("allocated: %zu KB\n", mapi_gc_allocated(mapi_instance(U)) / 1024);
    printf("allocated peak: %zu KB\n", mapi_gc_max_allocated(mapi_instance(U)) / 1024);
}

static char *file(const char *path) {
    FILE *file = fopen(path, "rb");

    if (file == NULL) {
        abort();
    }

    // get size

    int fseek_end_res = fseek(file, 0, SEEK_END);

    long tell_result = ftell(file);

    if (tell_result < 0 || fseek_end_res != 0) {
        abort();
    }

    int fseek_set_res = fseek(file, 0, SEEK_SET);

    if (fseek_set_res != 0) {
        abort();
    }

    size_t size = (size_t) tell_result + 1;

    // read

    char *result = malloc(size);
    memset(result, 0, size);

    size_t count = fread(result, 1, size, file);

    if (size - 1 != count) {
        abort();
    }

    // close

    fclose(file);

    return result;
}

morphine_noret static void signal(morphine_instance_t I) {
    const char *message = mapi_signal_message(I);
    printf("morphine panic: %s\n", message);

    if (I != NULL && !mapi_is_nested_signal(I)) {
        mapi_close(I);
    }

    abort();
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

static void vm(const char *text) {
    struct settings settings = {
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

    morphine_instance_t I = mapi_open(instance_platform, settings, NULL);
    morphine_coroutine_t U = mapi_coroutine(I);

    mapi_push_native(U, "test", test);
    mapi_push_string(U, text);
    mapi_call(U, 1);

    mapi_interpreter(I);
    mapi_close(I);
}

int main(int argc, const char **argv) {
    if (argc < 2) {
        return 1;
    }

    char *text = file(argv[1]);
    vm(text);
    free(text);

    return 0;
}
