//
// Created by why-iskra on 04.04.2024.
//

#pragma once

#include "morphine/platform.h"

#define callstackI_info(U) ((U)->callstack.callinfo)
#define callstackI_info_or_error(U) ({ morphine_coroutine_t _U = (U); struct callinfo *c = callstackI_info(_U); if(mm_unlikely(c == NULL)) throwI_error(_U->I, "require callable"); c; })

struct callinfo {
    struct {
        struct {
            union {
                struct value *base;
                struct value *source;
            };
            struct value *space;
            struct value *top;
        } stack;

        struct {
            struct value *callable;
            struct value *env;
            struct value *args;
            struct value *self;
        } direct;
    } s;

    struct {
        ml_size arguments_count;
        size_t pop_size;
    } info;

    struct {
        size_t position;
        ml_callstate state;
    } pc;

    struct {
        bool enable;
        bool crash;
        ml_callstate state;
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

void callstackI_pop(morphine_coroutine_t);
void callstackI_throw_fix(morphine_coroutine_t);

struct value callstackI_extract_callable(morphine_instance_t, struct value callable);

struct value callstackI_result(morphine_coroutine_t);
void callstackI_set_result(morphine_coroutine_t, struct value);
void callstackI_return(morphine_coroutine_t, struct value);
void callstackI_continue(morphine_coroutine_t, ml_callstate);
ml_callstate callstackI_state(morphine_coroutine_t);

void callstackI_call_unsafe(
    morphine_coroutine_t U,
    struct value callable,
    struct value self,
    struct value *args,
    ml_size argc,
    size_t pop_size
);

void callstackI_call_from_api(
    morphine_coroutine_t U,
    bool has_env,
    bool has_self,
    ml_size argc
);

void callstackI_call_from_interpreter(
    morphine_coroutine_t U,
    struct value *callable,
    struct value *self,
    ml_size argc
);
