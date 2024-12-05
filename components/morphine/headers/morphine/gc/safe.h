//
// Created by why-iskra on 30.03.2024.
//

#pragma once

#include "morphine/core/value.h"

#define gcI_safe_obj(I, n, x) (valueI_as_##n(gcI_safe((I), valueI_object(x))))

struct value gcI_safe(morphine_instance_t, struct value);

void gcI_safe_enter(morphine_instance_t);
void gcI_safe_exit(morphine_instance_t);

size_t gcI_safe_level(morphine_instance_t);
void gcI_safe_reset(morphine_instance_t, size_t level);
