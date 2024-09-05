//
// Created by why-iskra on 01.04.2024.
//

#pragma once

#include "morphine/core/value.h"

enum iterator_type {
    ITERATOR_TYPE_TABLE,
    ITERATOR_TYPE_VECTOR,
    ITERATOR_TYPE_STRING,
};

struct iterator {
    struct object header;

    struct {
        struct value key;
        struct value value;
    } name;

    enum iterator_type type;
    union {
        struct object *object;
        struct table *table;
        struct vector *vector;
        struct string *string;
    } iterable;

    struct {
        bool has;
        struct value key;
    } next;

    struct pair result;
};

struct iterator *iteratorI_create(morphine_instance_t, struct value);
void iteratorI_free(morphine_instance_t, struct iterator *);

void iteratorI_init(morphine_instance_t, struct iterator *, struct value key_name, struct value value_name);
bool iteratorI_has(morphine_instance_t, struct iterator *);
struct pair iteratorI_next(morphine_instance_t, struct iterator *);
struct table *iteratorI_next_table(morphine_instance_t, struct iterator *);
