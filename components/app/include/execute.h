//
// Created by whyiskra on 3/16/24.
//

#pragma once

#include <stdbool.h>
#include "allocator.h"

void execute(
    struct allocator *allocator,
    const char *path,
    bool binary,
    bool run,
    bool export,
    bool decompile,
    size_t alloc_limit,
    size_t argc,
    char **args
);
