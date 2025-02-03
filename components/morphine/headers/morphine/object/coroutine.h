//
// Created by whyiskra on 16.12.23.
//

#pragma once

#include "morphine/core/value.h"
#include "morphine/core/throw.h"

typedef enum {
    COROUTINE_STATUS_DETACHED,
    COROUTINE_STATUS_RUNNING,
    COROUTINE_STATUS_SUSPENDED,
    COROUTINE_STATUS_KILLING,
} coroutine_status_t;

struct callframe {
    union {
        struct {
            bool pop_callable;
            bool pop_args;
            ml_size arguments_count;
        } uninited;

        struct {
            struct {
                struct {
                    struct value *base;
                    struct value *top;
                } stack;

                struct {
                    struct value *args;
                    struct value *callable;
                } direct;
            } s;

            struct {
                ml_size position;
                ml_size state;
            } pc;

            struct {
                bool pop_callable;
                bool pop_args;

                bool catch_enabled;
                bool catch_crash;
                ml_size catch_state;

                ml_size arguments_count;
                ml_size pop_size;
            } params;

            struct callframe *prev;
        };
    };
};

struct coroutine {
    struct object header;
    struct string *name;

    coroutine_status_t status;

    struct {
        struct value *allocated;
        ml_size size;
        ml_size top;

        struct {
            ml_size limit;
            bool allow_reduce_stack;
        } settings;
    } stack;

    struct {
        ml_size access;
        ml_size size;
        struct callframe *frame;
        struct callframe *uninited;
        struct value result;

        struct {
            size_t size;
            struct callframe *pool;
        } cache;
    } callstack;

    struct value env;
    struct exception *exception;

    morphine_coroutine_t prev;
    morphine_instance_t I;
};

morphine_coroutine_t coroutineI_create(morphine_instance_t, struct string *name, struct value env);
void coroutineI_free(morphine_instance_t, morphine_coroutine_t);

void coroutineI_fix_stack(morphine_coroutine_t);
void coroutineI_fix_callstack(morphine_coroutine_t);

void coroutineI_detach(morphine_coroutine_t);
void coroutineI_suspend(morphine_coroutine_t);
void coroutineI_resume(morphine_coroutine_t);
void coroutineI_kill(morphine_coroutine_t);
bool coroutineI_isalive(morphine_coroutine_t);

struct exception *coroutineI_exception(morphine_coroutine_t);

const char *coroutineI_status2string(morphine_coroutine_t, coroutine_status_t);

void stackI_set_limit(morphine_coroutine_t, ml_size);
void stackI_reduce_stack(morphine_coroutine_t, bool emergency);
void stackI_reduce_cache(morphine_coroutine_t, bool emergency);

ml_size stackI_space(morphine_coroutine_t);
void stackI_push(morphine_coroutine_t, struct value);
void stackI_pop(morphine_coroutine_t, ml_size count);
struct value stackI_peek(morphine_coroutine_t, ml_size offset);
void stackI_replace(morphine_coroutine_t, ml_size offset, struct value);
void stackI_rotate(morphine_coroutine_t, ml_size count);

void callstackI_check_access(morphine_coroutine_t);
void callstackI_update_access(morphine_coroutine_t);

void callstackI_call(morphine_coroutine_t, struct value *callable, struct value *args, ml_size argc, ml_size pop_size);
void callstackI_call_api(morphine_coroutine_t, ml_size argc);
void callstackI_pop(morphine_coroutine_t, struct value);
void callstackI_drop(morphine_coroutine_t, struct callframe *);

struct value callstackI_extract_callable(morphine_instance_t, struct value);

void callstackI_continue(morphine_coroutine_t, ml_size state);
ml_size callstackI_state(morphine_coroutine_t);

ml_size callstackI_args(morphine_coroutine_t);
struct value callstackI_get_arg(morphine_coroutine_t, ml_size);
struct value callstackI_callable(morphine_coroutine_t);
struct value callstackI_result(morphine_coroutine_t);
void callstackI_set_result(morphine_coroutine_t, struct value);

void callstackI_catchable(morphine_coroutine_t, ml_size);
void callstackI_crashable(morphine_coroutine_t);
void callstackI_uncatch(morphine_coroutine_t);
