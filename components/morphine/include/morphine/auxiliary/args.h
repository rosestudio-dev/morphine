//
// Created by why-iskra on 31.03.2024.
//

#pragma once

#include "morphine/platform.h"
#include "morphine/config.h"

MORPHINE_AUX void maux_checkargs_fixed(morphine_state_t, size_t count);
MORPHINE_AUX size_t maux_checkargs_pattern(morphine_state_t, size_t count, ...);
MORPHINE_AUX void maux_checkargs_self(morphine_state_t, size_t count);
MORPHINE_AUX size_t maux_checkargs_or(morphine_state_t, size_t count1, size_t count2);
MORPHINE_AUX size_t maux_checkargs_minimum(morphine_state_t, size_t minimum);
