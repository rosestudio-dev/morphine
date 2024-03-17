//
// Created by whyiskra on 16.12.23.
//

#pragma once

#include "config/modifiers.h"
#include "platform.h"
#include "utils/nativebuilder.h"

struct maux_construct_field {
    const char *name;
    morphine_native_t value;
};

MORPHINE_AUX void maux_push_empty_callable(morphine_state_t S);

MORPHINE_AUX void maux_table_lock(morphine_state_t);

MORPHINE_AUX void maux_expect(morphine_state_t, const char *type);

MORPHINE_AUX void maux_checkargs_fixed(morphine_state_t, size_t count);
MORPHINE_AUX void maux_checkargs_pattern(morphine_state_t, size_t count, ...);
MORPHINE_AUX void maux_checkargs_self(morphine_state_t, size_t count);
MORPHINE_AUX size_t maux_checkargs_or(morphine_state_t, size_t count1, size_t count2);
MORPHINE_AUX size_t maux_checkargs_minimum(morphine_state_t, size_t minimum);

MORPHINE_AUX void maux_construct(morphine_state_t, struct maux_construct_field *table);

MORPHINE_AUX void maux_construct_call(
    morphine_state_t,
    struct maux_construct_field *table,
    const char *name,
    size_t argc
);
