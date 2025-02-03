//
// Created by whyiskra on 25.12.23.
//

#pragma once

#include "morphine/platform.h"

struct interpreter {
    bool entered;
    morphine_coroutine_t context;

    morphine_coroutine_t coroutines;
    morphine_coroutine_t running;
    morphine_coroutine_t next;
};

struct interpreter interpreterI_prototype(void);

void interpreterI_run(morphine_instance_t);
bool interpreterI_step(morphine_instance_t);
