//
// Created by why-iskra on 01.04.2024.
//

#pragma once

#include "morphine/core/value.h"

enum iterator_type {
    ITERATOR_TYPE_TABLE,
    ITERATOR_TYPE_VECTOR,
};

struct iterator {
    struct object header;

    enum iterator_type type;
    union {
        struct object *object;
        struct table *table;
        struct vector *vector;
    } iterable;

    struct {
        bool has;
        struct value key;
    } next;
};

struct iterator *iteratorI_create(morphine_instance_t, struct value);
void iteratorI_free(morphine_instance_t, struct iterator *);

void iteratorI_init(morphine_instance_t, struct iterator *);
bool iteratorI_has(morphine_instance_t, struct iterator *);
struct pair iteratorI_next(morphine_instance_t, struct iterator *);
