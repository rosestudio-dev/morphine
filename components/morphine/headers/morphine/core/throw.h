//
// Created by whyiskra on 16.12.23.
//

#pragma once

#include <setjmp.h>
#include "morphine/core/value.h"

typedef enum {
    THROW_TYPE_UNDEF,
    THROW_TYPE_VALUE,
    THROW_TYPE_MESSAGE,
    THROW_TYPE_OFM,
    THROW_TYPE_AF,
} throw_type_t;

struct protect_frame {
    bool entered;
    jmp_buf handler;
    size_t danger_entered;
    size_t safe_level;
};

struct throw {
    size_t signal_entered;

    struct protect_frame protect;

    throw_type_t type;
    union {
        struct value value;
        const char *message;
    } error;

    struct {
        struct exception *ofm;
        struct exception *af;
    } special;

    struct {
        morphine_coroutine_t stack;
        morphine_coroutine_t callstack;
    } fix;
};

struct throw throwI_prototype(void);
void throwI_destruct(morphine_instance_t);
void throwI_special(morphine_instance_t);

void throwI_handler(morphine_instance_t);
void throwI_protect(morphine_instance_t, morphine_try_t, morphine_catch_t, void *, void *, bool);

morphine_noret void throwI_error(morphine_instance_t, const char *);
morphine_noret void throwI_errorv(morphine_instance_t, struct value);
morphine_noret morphine_printf(2, 3) void throwI_errorf(morphine_instance_t, const char *, ...);
morphine_noret void throwI_panic(morphine_instance_t, const char *);
morphine_noret void throwI_panicv(morphine_instance_t, struct value);
morphine_noret morphine_printf(2, 3) void throwI_panicf(morphine_instance_t, const char *, ...);
morphine_noret void throwI_ofm(morphine_instance_t);
morphine_noret void throwI_af(morphine_instance_t);
morphine_noret void throwI_undef(morphine_instance_t);

morphine_noret void throwI_provide_error(morphine_coroutine_t);

void throwI_danger_enter(morphine_instance_t);
void throwI_danger_exit(morphine_instance_t);

const char *throwI_message(morphine_instance_t);
bool throwI_is_nested_signal(morphine_instance_t);
