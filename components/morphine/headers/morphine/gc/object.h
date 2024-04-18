//
// Created by whyiskra on 16.12.23.
//

#pragma once

#include "morphine/platform.h"
#include "morphine/utils/cast.h"
#include "morphine/type.h"

#define objectI_cast(x) cast(struct object *, (x))

enum obj_color {
    OBJ_COLOR_WHITE = 0,
    OBJ_COLOR_GREY = 1,
    OBJ_COLOR_BLACK = 2,
    OBJ_COLOR_RED = 3,
};

struct object {
    struct object *prev;
    struct object *next;
    enum obj_type type;
    enum obj_color color: 8;
    struct {
        bool finalized;
    } flags;
};

void objectI_init(morphine_instance_t, struct object *, enum obj_type);
void objectI_free(morphine_instance_t, struct object *);
