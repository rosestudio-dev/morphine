//
// Created by why-iskra on 31.03.2024.
//

#pragma once

#include "morphine/platform.h"

struct function *loaderI_load(
    morphine_coroutine_t,
    morphine_init_t,
    morphine_read_t,
    morphine_finish_t,
    void *
);
