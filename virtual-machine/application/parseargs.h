//
// Created by whyiskra on 3/16/24.
//

#pragma once

#include <stddef.h>
#include <stdbool.h>

struct args {
    size_t alloc_limit;
    bool measure_time;
    bool custom_alloc;
    bool human_readable;
    const char *program_path;
    size_t argc;
    char **args;
};

struct args parseargs(int argc, char **argv);
