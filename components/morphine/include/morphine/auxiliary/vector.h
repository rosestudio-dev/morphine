//
// Created by why-iskra on 31.03.2024.
//

#pragma once

#include "morphine/platform.h"

MORPHINE_AUX void maux_vector_clear(morphine_coroutine_t);

MORPHINE_AUX void maux_vector_push(morphine_coroutine_t);
MORPHINE_AUX void maux_vector_pop(morphine_coroutine_t);
MORPHINE_AUX void maux_vector_peek(morphine_coroutine_t);

MORPHINE_AUX void maux_vector_push_front(morphine_coroutine_t);
MORPHINE_AUX void maux_vector_pop_front(morphine_coroutine_t);
MORPHINE_AUX void maux_vector_peek_front(morphine_coroutine_t);
