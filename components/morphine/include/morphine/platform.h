//
// Created by whyiskra on 28.04.23.
//

#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include "config.h"
#include "limits.h"

typedef struct coroutine *morphine_coroutine_t;
typedef struct instance *morphine_instance_t;

typedef uint8_t priority_t;

typedef void (*morphine_native_t)(morphine_coroutine_t);

typedef void (*morphine_userdata_mark_t)(morphine_instance_t, void *);
typedef void (*morphine_userdata_free_t)(morphine_instance_t, void *);

typedef void *(*morphine_loader_init_t)(morphine_coroutine_t, void *args);
typedef uint8_t (*morphine_loader_read_t)(morphine_coroutine_t, void *data, const char **error);
typedef void (*morphine_loader_finish_t)(morphine_coroutine_t, void *data);

struct platform {
    struct {
        void *(*malloc)(size_t);
        void *(*realloc)(void *, size_t);
        void (*free)(void *);
        void (*signal)(morphine_instance_t) morphine_noret;
    } functions;

    struct {
        FILE *in;
        FILE *out;
        FILE *stacktrace;
    } io;
};

struct gc_settings {
    size_t limit_bytes;
    size_t threshold;
    uint16_t grow;
    uint16_t deal;
    uint8_t pause;
};

struct coroutine_settings {
    size_t stack_limit;
    size_t stack_grow;
};

struct settings {
    struct gc_settings gc;
    struct coroutine_settings states;
    struct coroutine_settings finalizer;
};

struct require_loader {
    const char *name;
    void (*loader)(morphine_coroutine_t);
};

bool platformI_string2integer(const char *string, ml_integer *container);
bool platformI_string2decimal(const char *string, ml_decimal *container);
