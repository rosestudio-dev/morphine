//
// Created by whyiskra on 25.12.23.
//

#pragma once

#include "morphine/platform.h"
#include "morphine/core/throw.h"

struct interpreter {
    morphine_coroutine_t coroutines;
    morphine_coroutine_t running;
    morphine_coroutine_t next;

    ml_size circle;
};

struct interpreter interpreterI_prototype(void);

void interpreterI_run(morphine_instance_t);
bool interpreterI_step(morphine_instance_t);
