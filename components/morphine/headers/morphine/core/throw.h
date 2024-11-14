//
// Created by whyiskra on 16.12.23.
//

#pragma once

#include <setjmp.h>
#include "morphine/core/value.h"
#include "morphine/utils/semicolon.h"

#define throwI_errorf(I, ...) semicolon_blk(morphine_instance_t _instance = (I); throwI_errorv(_instance, valueI_object(stringI_createf(_instance, __VA_ARGS__)));)

typedef enum {
    THROW_TYPE_VALUE,
    THROW_TYPE_MESSAGE,
    THROW_TYPE_OFM,
} throw_type_t;

struct protect_frame {
    bool entered;
    jmp_buf handler;
};

struct throw {
    morphine_coroutine_t context;
    size_t signal_entered;

    struct protect_frame protect;

    throw_type_t type;
    union {
        struct value value;
        const char *message;
    } error;

    struct {
        struct exception *ofm;
    } special;
};

struct throw throwI_prototype(void);
void throwI_special(morphine_instance_t);

void throwI_handler(morphine_instance_t);
void throwI_protect(morphine_instance_t, morphine_try_t, morphine_catch_t, void *, void *);

morphine_noret void throwI_error(morphine_instance_t, const char *message);
morphine_noret void throwI_errorv(morphine_instance_t, struct value value);
morphine_noret void throwI_panic(morphine_instance_t, const char *message);
morphine_noret void throwI_ofm(morphine_instance_t);

const char *throwI_message(morphine_instance_t);
bool throwI_is_nested_signal(morphine_instance_t);

void throwI_catchable(morphine_coroutine_t, size_t callstate);
void throwI_crashable(morphine_coroutine_t);
void throwI_uncatch(morphine_coroutine_t);

struct value throwI_thrown(morphine_coroutine_t);
