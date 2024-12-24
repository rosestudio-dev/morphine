//
// Created by why-iskra on 24.12.2024.
//

#pragma once

#include "morphine/core/value.h"

struct value isolateI_call(
    morphine_instance_t,
    morphine_isolate_config_t,
    struct value,
    struct value *,
    ml_size
);
