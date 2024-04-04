//
// Created by whyiskra on 16.12.23.
//

#pragma once

#include <setjmp.h>
#include "morphine/core/value.h"
#include "morphine/utils/semicolon.h"

#define throwI_errorf(I, ...) morphinem_blk_start morphine_instance_t P_I = (I); throwI_errorv(P_I, valueI_object(stringI_createf(P_I, __VA_ARGS__))); morphinem_blk_end
#define throwI_panicf(I, ...) morphinem_blk_start morphine_instance_t P_I = (I); throwI_panicv(P_I, valueI_object(stringI_createf(P_I, __VA_ARGS__))); morphinem_blk_end

struct throw {
    bool inited;
    jmp_buf handler;

    bool is_message;
    union {
        struct value value;
        const char *message;
    } error;

    morphine_state_t context_state;
};

struct throw throwI_prototype(void);

void throwI_handler(morphine_instance_t);

morphine_noret void throwI_errorv(morphine_instance_t, struct value value);
morphine_noret void throwI_panicv(morphine_instance_t, struct value value);

morphine_noret void throwI_error(morphine_instance_t, const char *message);
morphine_noret void throwI_panic(morphine_instance_t, const char *message);

const char *throwI_message(morphine_instance_t);

void throwI_catchable(morphine_state_t, size_t callstate);
