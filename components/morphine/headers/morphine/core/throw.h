//
// Created by whyiskra on 16.12.23.
//

#pragma once

#include <setjmp.h>
#include "value.h"
#include "morphine/utils/semicolon.h"

#define throwI_errorf(S, ...) morphinem_blk_start morphine_state_t P_S = (S); throwI_error(P_S, valueI_object(stringI_createf(P_S->I, __VA_ARGS__))); morphinem_blk_end

struct throw {
    bool inited;
    jmp_buf handler;
    bool is_message;
    union {
        struct value value;
        const char *message;
    } result;
    morphine_state_t cause_state;
};

struct throw throwI_prototype(void);

void throwI_handler(morphine_instance_t I);

morphine_noret void throwI_error(morphine_state_t, struct value value);
morphine_noret void throwI_message_error(morphine_state_t, const char *message);

morphine_noret void throwI_panic(morphine_instance_t, morphine_state_t cause_state, struct value value);
morphine_noret void throwI_message_panic(morphine_instance_t, morphine_state_t cause_state, const char *message);

void throwI_catchable(morphine_state_t, size_t callstate);

const char *throwI_get_panic_message(morphine_instance_t);
