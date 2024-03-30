//
// Created by whyiskra on 28.04.23.
//

#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include "config.h"

// numbers

typedef int32_t morphine_integer_t;
#define MORPHINE_INTEGER_PRINT "d"
#define MORPHINE_INTEGER_MAX   INT32_MAX
#define MORPHINE_INTEGER_MIN   (INT32_MIN + 1)

typedef double morphine_decimal_t;
#define MORPHINE_DECIMAL_PRINT        "g"
#define morphine_decimal_tostrc(s, p) strtod((s), (p))

// other

typedef struct state *morphine_state_t;
typedef struct instance *morphine_instance_t;

typedef uint8_t priority_t;

typedef void (*morphine_native_t)(morphine_state_t);

typedef void (*morphine_userdata_mark_t)(morphine_instance_t, void *);
typedef void (*morphine_userdata_free_t)(morphine_instance_t, void *);

typedef void *(*morphine_loader_init_t)(morphine_state_t, void *args);
typedef uint8_t (*morphine_loader_read_t)(morphine_state_t, void *data, const char **error);
typedef void (*morphine_loader_finish_t)(morphine_state_t, void *data);

#ifdef MORPHINE_ENABLE_DEBUGGER
struct debugger {
    struct {
        void (*gc_step_enter)(morphine_instance_t);
        void (*gc_step_middle)(morphine_instance_t);
        void (*gc_step_exit)(morphine_instance_t);
        void (*gc_full_enter)(morphine_instance_t);
        void (*gc_full_exit)(morphine_instance_t);
        void (*interpreter_step)(morphine_instance_t, morphine_state_t);
    } hooks;
};
#endif

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

#ifdef MORPHINE_ENABLE_DEBUGGER
    struct debugger debugger;
#endif
};

struct params {
    struct {
        size_t limit_bytes;
        size_t start;
        size_t grow;
        size_t deal;

        struct {
            size_t stack_limit;
            size_t stack_grow;
        } finalizer;
    } gc;

    struct {
        size_t stack_limit;
        size_t stack_grow;
    } states;
};

struct require_loader {
    const char *name;
    void (*loader)(morphine_state_t);
};

bool platformI_string2integer(const char *string, morphine_integer_t *container, uint8_t base);
bool platformI_string2decimal(const char *string, morphine_decimal_t *container);
