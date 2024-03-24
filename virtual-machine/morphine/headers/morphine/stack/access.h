//
// Created by why on 3/24/24.
//

#pragma once

#include "morphine/core/value.h"

size_t stackI_space_size(morphine_state_t);
void stackI_push(morphine_state_t, struct value value);
struct value stackI_peek(morphine_state_t, size_t offset);
struct value *stackI_vector(morphine_state_t, size_t offset, size_t size);
void stackI_pop(morphine_state_t, size_t count);
