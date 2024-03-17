//
// Created by whyiskra on 16.12.23.
//

#pragma once

#include "morphine/core/value.h"

struct native {
    struct object header;

    morphine_native_t function;
    const char *name;

    struct value registry_key;
};

struct native *nativeI_create(morphine_instance_t, const char *name, morphine_native_t function);
void nativeI_free(morphine_instance_t, struct native *);

size_t nativeI_allocated_size(struct native *);
