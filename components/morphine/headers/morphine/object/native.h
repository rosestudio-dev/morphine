//
// Created by whyiskra on 16.12.23.
//

#pragma once

#include "morphine/core/value.h"

struct native {
    struct object header;

    morphine_native_t function;
    char *name;
    size_t name_len;

    struct value registry_key;
};

struct native *nativeI_create(morphine_instance_t, const char *name, morphine_native_t function);
void nativeI_free(morphine_instance_t, struct native *);
