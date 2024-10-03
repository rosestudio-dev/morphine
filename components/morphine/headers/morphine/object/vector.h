//
// Created by why-iskra on 22.04.2024.
//

#pragma once

#include "morphine/gc/object.h"

struct vector {
    struct object header;

    struct {
        bool fixed;
        bool mutable;
        bool locked;
    } mode;

    struct {
        ml_size real;
        ml_size accessible;
    } size;

    struct value *values;
};

struct vector *vectorI_create(morphine_instance_t, ml_size size);
void vectorI_free(morphine_instance_t, struct vector *);

void vectorI_mode_fixed(morphine_instance_t, struct vector *, bool is_fixed);
void vectorI_mode_mutable(morphine_instance_t, struct vector *, bool is_mutable);
void vectorI_mode_lock(morphine_instance_t, struct vector *);

ml_size vectorI_size(morphine_instance_t, struct vector *);

void vectorI_set(morphine_instance_t, struct vector *, ml_size, struct value);
struct value vectorI_get(morphine_instance_t, struct vector *, ml_size);
void vectorI_add(morphine_instance_t, struct vector *, ml_size, struct value);
struct value vectorI_remove(morphine_instance_t, struct vector *, ml_size);

void vectorI_resize(morphine_instance_t, struct vector *, ml_size);
struct vector *vectorI_copy(morphine_instance_t, struct vector *);
struct vector *vectorI_concat(morphine_instance_t, struct vector *, struct vector *);

struct value vectorI_iterator_first(morphine_instance_t, struct vector *, bool *has);
struct pair vectorI_iterator_next(morphine_instance_t, struct vector *, struct value *key, bool *next);
