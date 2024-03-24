//
// Created by why on 3/24/24.
//

#pragma once

#include "morphine/platform.h"

void stackI_shrink(morphine_state_t);

void stackI_set_grow(morphine_state_t, size_t grow);
void stackI_set_limit(morphine_state_t, size_t limit);
