//
// Created by whyiskra on 3/16/24.
//

#pragma once

#include "allocator.h"

void execute(
    struct allocator *allocator,
    const char *path,
    size_t alloc_limit
);
