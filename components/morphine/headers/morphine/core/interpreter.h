//
// Created by whyiskra on 25.12.23.
//

#pragma once

#include "morphine/platform.h"
#include "morphine/core/throw.h"

struct interpreter {
    morphine_state_t states;
    morphine_state_t candidates;

    uint16_t circle;
    struct throw throw;
};

struct interpreter interpreterI_prototype(void);

void interpreterI_run(morphine_instance_t);
