//
// Created by whyiskra on 3/16/24.
//

#pragma once

#include <stddef.h>
#include <stdbool.h>

struct args {
    size_t alloc_limit;
    bool measure_time;
    bool measure_time_pretty;
    bool custom_alloc;
    bool custom_alloc_pretty;
    bool binary;
    const char *program_path;
    size_t argc;
    char **args;

    bool version;
};

struct args parseargs(int argc, char **argv);
