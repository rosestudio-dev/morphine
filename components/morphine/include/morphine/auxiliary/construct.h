//
// Created by why-iskra on 02.11.2024.
//

#pragma once

#include "morphine/platform.h"

#define MAUX_CONSTRUCT_FUNCTION(n, v)       ((maux_construct_element_t) { .type = MAUX_CONSTRUCT_TYPE_FUNCTION, .name = (n), .value.function = (v) })
#define MAUX_CONSTRUCT_NAMED_FUNCTION(n, v, f) ((maux_construct_element_t) { .type = MAUX_CONSTRUCT_TYPE_NAMED_FUNCTION, .name = (n), .value.named.name = (v), .value.named.function = (f) })
#define MAUX_CONSTRUCT_STRING(n, v)         ((maux_construct_element_t) { .type = MAUX_CONSTRUCT_TYPE_STRING, .name = (n), .value.string = (v) })
#define MAUX_CONSTRUCT_INTEGER(n, v)        ((maux_construct_element_t) { .type = MAUX_CONSTRUCT_TYPE_INTEGER, .name = (n), .value.integer = (v) })
#define MAUX_CONSTRUCT_DECIMAL(n, v)        ((maux_construct_element_t) { .type = MAUX_CONSTRUCT_TYPE_DECIMAL, .name = (n), .value.decimal = (v) })
#define MAUX_CONSTRUCT_BOOLEAN(n, v)        ((maux_construct_element_t) { .type = MAUX_CONSTRUCT_TYPE_BOOLEAN, .name = (n), .value.boolean = (v) })
#define MAUX_CONSTRUCT_NIL(n)               ((maux_construct_element_t) { .type = MAUX_CONSTRUCT_TYPE_NIL, .name = (n), .value.stub = NULL })
#define MAUX_CONSTRUCT_EMPTY(n)             ((maux_construct_element_t) { .type = MAUX_CONSTRUCT_TYPE_EMPTY, .name = (n), .value.stub = NULL })
#define MAUX_CONSTRUCT_END                  ((maux_construct_element_t) { .type = MAUX_CONSTRUCT_TYPE_END, .name = NULL, .value.stub = NULL })

typedef enum {
    MAUX_CONSTRUCT_TYPE_END,
    MAUX_CONSTRUCT_TYPE_STRING,
    MAUX_CONSTRUCT_TYPE_INTEGER,
    MAUX_CONSTRUCT_TYPE_DECIMAL,
    MAUX_CONSTRUCT_TYPE_BOOLEAN,
    MAUX_CONSTRUCT_TYPE_FUNCTION,
    MAUX_CONSTRUCT_TYPE_NAMED_FUNCTION,
    MAUX_CONSTRUCT_TYPE_NIL,
    MAUX_CONSTRUCT_TYPE_EMPTY
} maux_construct_type_t;

typedef struct {
    maux_construct_type_t type;
    const char *name;

    union {
        void *stub;
        const char *string;
        ml_integer integer;
        ml_decimal decimal;
        bool boolean;
        morphine_native_t function;

        struct {
            const char *name;
            morphine_native_t function;
        } named;
    } value;
} maux_construct_element_t;

MORPHINE_AUX void maux_construct(morphine_coroutine_t, maux_construct_element_t *);
