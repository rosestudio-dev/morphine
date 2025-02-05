//
// Created by whyiskra on 16.12.23.
//

#pragma once

#include "morphine/core/value.h"

struct native {
    struct object header;
    struct string *name;

    mfunc_native_t function;
};

struct native *nativeI_create(morphine_instance_t, struct string *name, mfunc_native_t function);
void nativeI_free(morphine_instance_t, struct native *);
