//
// Created by why-iskra on 01.04.2024.
//

#pragma once

#include "morphine/core/value.h"

struct iterator {
    struct object header;

    struct table *iterable;

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
