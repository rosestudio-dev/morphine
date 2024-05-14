//
// Created by whyiskra on 3/16/24.
//

#pragma once

#include <stdbool.h>
#include "allocator.h"

struct vmdata {
    struct libcompiler *libcompiler;
};

void execute(
    struct libcompiler *libcompiler,
    struct allocator *allocator,
    const char *path,
    bool binary,
    size_t alloc_limit,
    size_t argc,
    char **args
);
