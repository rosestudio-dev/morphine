//
// Created by why-iskra on 31.03.2024.
//

#pragma once

#include "morphine/platform.h"

MORPHINE_AUX void maux_expect(morphine_coroutine_t, const char *type);
MORPHINE_AUX void maux_expect_args(morphine_coroutine_t U, size_t count);
MORPHINE_AUX void maux_expect_args_minimum(morphine_coroutine_t U, size_t count);
