//
// Created by why-iskra on 30.03.2024.
//

#pragma once

#include "morphine/core/value.h"

struct value *gcI_safe(morphine_instance_t, size_t *rollback);
size_t gcI_safe_value(morphine_instance_t, struct value);
size_t gcI_safe_obj(morphine_instance_t, struct object *);
void gcI_reset_safe(morphine_instance_t, size_t rollback);
