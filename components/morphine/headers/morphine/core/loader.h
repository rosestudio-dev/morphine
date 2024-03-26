//
// Created by whyiskra on 16.12.23.
//

#pragma once

#include <stdint.h>
#include "morphine/object/state.h"

struct proto *loaderI_load(
    morphine_state_t,
    morphine_loader_init_t init,
    morphine_loader_read_t read,
    morphine_loader_finish_t finish,
    void *args
);
