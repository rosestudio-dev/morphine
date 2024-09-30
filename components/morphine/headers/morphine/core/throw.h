//
// Created by whyiskra on 16.12.23.
//

#pragma once

#include <setjmp.h>
#include "morphine/core/value.h"
#include "morphine/utils/semicolon.h"

#define throwI_errorf(I, ...) semicolon_blk(morphine_instance_t _instance = (I); throwI_errorv(_instance, valueI_object(stringI_createf(_instance, __VA_ARGS__)));)

struct throw {
    bool inited;
    jmp_buf handler;
    morphine_coroutine_t context;
    size_t signal_entered;

    bool is_message;
    union {
        struct value value;
        const char *message;
    } error;
};

struct throw throwI_prototype(void);

void throwI_handler(morphine_instance_t);

morphine_noret void throwI_error(morphine_instance_t, const char *message);
morphine_noret void throwI_errorv(morphine_instance_t, struct value value);

morphine_noret void throwI_panic(morphine_instance_t, const char *message);
morphine_noret void throwI_panicv(morphine_instance_t, struct value value);

const char *throwI_message(morphine_instance_t);
bool throwI_is_nested_signal(morphine_instance_t);

void throwI_catchable(morphine_coroutine_t, size_t callstate);
