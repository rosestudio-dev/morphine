//
// Created by why-iskra on 04.04.2024.
//

#pragma once

#include "morphine/platform.h"

#define callstackI_info(U) ((U)->callstack.callinfo)
#define callstackI_info_or_error(U) ({ morphine_coroutine_t _U = (U); struct callinfo *c = callstackI_info(_U); if(unlikely(c == NULL)) throwI_error(_U->I, "Require callable"); c; })

struct callinfo {
    struct {
        union {
            struct value *base;
            struct value *callable;
        };

        struct value *source;
        struct value *env;
        struct value *self;
        struct value *result;
        struct value *thrown;
        struct value *args;
        struct value *slots;
        struct value *params;
        struct value *space;
        struct value *top;
    } s;

    size_t pop_size;

    size_t arguments_count;

    struct {
        size_t position;
        size_t state;
    } pc;

    struct {
        bool enable;
        size_t state;
        size_t space_size;
    } catch;

    bool exit;

    struct callinfo *prev;
};

struct callstack {
    size_t size;
    struct callinfo *callinfo;
    struct callinfo *uninit_callinfo;
};

struct callstack callstackI_prototype(void);
void callstackI_destruct(morphine_instance_t, struct callstack *);
void callstackI_callinfo_free(morphine_instance_t, struct callinfo *);

void callstackI_call_unsafe(
    morphine_coroutine_t U,
    struct value callable,
    struct value self,
    struct value *args,
    size_t argc,
    size_t pop_size
);

void callstackI_call_stack(
    morphine_coroutine_t U,
    struct value callable,
    struct value self,
    size_t offset,
    size_t argc,
    size_t pop_size
);

void callstackI_call_params(
    morphine_coroutine_t U,
    struct value callable,
    struct value self,
    size_t argc,
    size_t pop_size
);

void callstackI_pop(morphine_coroutine_t);
void callstackI_fix_uninit(morphine_coroutine_t);

struct value callstackI_extract_callable(morphine_instance_t, struct value callable);
bool callstackI_is_callable(morphine_instance_t I, struct value callable);

struct value callstackI_result(morphine_coroutine_t);
void callstackI_return(morphine_coroutine_t, struct value);
void callstackI_leave(morphine_coroutine_t);
void callstackI_continue(morphine_coroutine_t, size_t state);
size_t callstackI_state(morphine_coroutine_t);
