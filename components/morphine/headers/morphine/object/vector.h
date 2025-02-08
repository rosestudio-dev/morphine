//
// Created by why-iskra on 22.04.2024.
//

#pragma once

#include "morphine/core/value.h"

struct vector {
    struct object header;
    bool dynamic;

    struct {
        ml_size real;
        ml_size accessible;
    } size;

    struct value *values;
};

struct vector *vectorI_create(morphine_instance_t, ml_size size, bool dynamic);
void vectorI_free(morphine_instance_t, struct vector *);

ml_size vectorI_size(struct vector *);
bool vectorI_has(struct vector *, struct value);

struct value vectorI_get(morphine_instance_t, struct vector *, ml_size);
void vectorI_set(morphine_instance_t, struct vector *, ml_size, struct value);
void vectorI_add(morphine_instance_t, struct vector *, ml_size, struct value);
struct value vectorI_remove(morphine_instance_t, struct vector *, ml_size);

void vectorI_resize(morphine_instance_t, struct vector *, ml_size);
struct vector *vectorI_copy(morphine_instance_t, struct vector *);
struct vector *vectorI_concat(morphine_instance_t, struct vector *, struct vector *);
