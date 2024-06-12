//
// Created by whyiskra on 3/16/24.
//

#pragma once

#include <stdbool.h>
#include "allocator.h"

void execute(
    struct allocator *allocator,
    const char *path,
    const char *export_path,
    bool binary,
    bool run,
    bool disassembly,
    size_t alloc_limit,
    size_t argc,
    char **args
);
