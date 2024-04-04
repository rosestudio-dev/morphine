//
// Created by whyiskra on 3/24/24.
//

#pragma once

#include "morphine/core/value.h"

size_t stackI_space_size(morphine_coroutine_t);
void stackI_push(morphine_coroutine_t, struct value value);
struct value stackI_peek(morphine_coroutine_t, size_t offset);
struct value *stackI_vector(morphine_coroutine_t, size_t offset, size_t size);
void stackI_pop(morphine_coroutine_t, size_t count);
