//
// Created by whyiskra on 16.12.23.
//

#pragma once

#include "morphine/platform.h"
#include "morphine/utils/cast.h"
#include "morphine/type.h"

#define objectI_cast(x) cast(struct object *, (x))

struct object {
    struct object *prev;
    enum obj_type type;
    struct {
        bool mark;
        bool finalized;
    } flags;
};

void objectI_init(morphine_instance_t, struct object *, enum obj_type);
void objectI_free(morphine_instance_t, struct object *);
