//
// Created by why-iskra on 02.11.2024.
//

#pragma once

#include "morphine/platform.h"

#define MAUX_CONSTRUCT_FUNCTION(n, v) MAUX_CONSTRUCT_NFUNCTION(n, n, v)
#define MAUX_CONSTRUCT_SIZE(n, v)     MAUX_CONSTRUCT_NSIZE(n, n, v)

#define MAUX_CONSTRUCT_NFUNCTION(n, v, f) ((maux_construct_element_t) { .type = MAUX_CONSTRUCT_TYPE_FUNCTION, .name = (n), .value.function.name = (v), .value.function.value = (f) })
#define MAUX_CONSTRUCT_NSIZE(n, t, v)     ((maux_construct_element_t) { .type = MAUX_CONSTRUCT_TYPE_SIZE, .name = (n), .value.size.name = (t), .value.size.value = (v) })
#define MAUX_CONSTRUCT_STRING(n, v)       ((maux_construct_element_t) { .type = MAUX_CONSTRUCT_TYPE_STRING, .name = (n), .value.string = (v) })
#define MAUX_CONSTRUCT_INTEGER(n, v)      ((maux_construct_element_t) { .type = MAUX_CONSTRUCT_TYPE_INTEGER, .name = (n), .value.integer = (v) })
#define MAUX_CONSTRUCT_DECIMAL(n, v)      ((maux_construct_element_t) { .type = MAUX_CONSTRUCT_TYPE_DECIMAL, .name = (n), .value.decimal = (v) })
#define MAUX_CONSTRUCT_BOOLEAN(n, v)      ((maux_construct_element_t) { .type = MAUX_CONSTRUCT_TYPE_BOOLEAN, .name = (n), .value.boolean = (v) })
#define MAUX_CONSTRUCT_NIL(n)             ((maux_construct_element_t) { .type = MAUX_CONSTRUCT_TYPE_NIL, .name = (n), .value.stub = NULL })
#define MAUX_CONSTRUCT_EMPTY(n)           ((maux_construct_element_t) { .type = MAUX_CONSTRUCT_TYPE_EMPTY, .name = (n), .value.stub = NULL })
#define MAUX_CONSTRUCT_END                ((maux_construct_element_t) { .type = MAUX_CONSTRUCT_TYPE_END, .name = NULL, .value.stub = NULL })

typedef enum {
    MAUX_CONSTRUCT_TYPE_END,
    MAUX_CONSTRUCT_TYPE_STRING,
    MAUX_CONSTRUCT_TYPE_INTEGER,
    MAUX_CONSTRUCT_TYPE_DECIMAL,
    MAUX_CONSTRUCT_TYPE_BOOLEAN,
    MAUX_CONSTRUCT_TYPE_FUNCTION,
    MAUX_CONSTRUCT_TYPE_SIZE,
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

        struct {
            const char *name;
            mfunc_native_t value;
        } function;

        struct {
            const char *name;
            size_t value;
        } size;
    } value;
} maux_construct_element_t;

MORPHINE_AUX void maux_construct(morphine_coroutine_t, maux_construct_element_t *);
