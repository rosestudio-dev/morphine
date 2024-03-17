//
// Created by whyiskra on 16.12.23.
//

#pragma once

#include "morphine/platform.h"
#include "morphine/utils/cast.h"
#include "type.h"

#define objectI_cast(x) morphinem_cast(struct object *, (x))

struct object {
    struct object *prev;
    bool mark;
    enum obj_type type;
};

void objectI_init(morphine_instance_t, struct object *, enum obj_type);
void objectI_free(morphine_instance_t, struct object *);
