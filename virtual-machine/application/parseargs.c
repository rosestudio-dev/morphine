//
// Created by whyiskra on 3/16/24.
//

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include "parseargs.h"

static void help(const char *program, const char *message, bool disable_usage) {
    printf("Morphine virtual-machine\n");
    if (message != NULL) {
        printf("Message: %s\n\n", message);
    }

    if (!disable_usage) {
        printf("Usage: %s [options] [program [args]]\n", program);
        printf("Positional arguments:\n");
        printf("    program       path to program\n");
        printf("    args          program arguments\n");
        printf("Optional arguments:\n");
        printf("    -l bytes [8M] Limit of allocation in bytes (suffixes: K, M, G, T)\n");
        printf("    -a            Use custom debug allocator\n");
        printf("    -m            Enable executing time measure\n");
        printf("    -H            Human-readable output\n");
        printf("    -h            Usage info\n");
        printf("    --            Stop handling options\n");
        printf("    -             Stop handling options and execute from stdin\n");
    }

    exit(1);
}

static size_t read_bytes(const char *str, const char *program) {
    char *end;
    size_t bytes = strtoumax(str, &end, 10);

    switch (*end) {
        case '\0':
            return bytes;
        case 'G':
            bytes *= 1024 * 1024 * 1024;
            break;
        case 'M':
            bytes *= 1024 * 1024;
            break;
        case 'K':
            bytes *= 1024;
            break;
    }

    if (end[1] != '\0') {
        help(program, "wrong bytes suffix", true);
    }

    return bytes;
}

struct args parseargs(int argc, char **argv) {
    struct args args = {
        .alloc_limit = 1024 * 1024 * 8,
        .measure_time = false,
        .custom_alloc = false,
        .program_path = NULL,
        .argc = 0,
        .args = NULL
    };

    const char *program = 0;
    if (argc >= 1) {
        program = argv[0];
    } else {
        program = "morphine";
    }

    if (argc < 2) {
        help(program, NULL, false);
    }

    int parsed = 1;
    for (; parsed < argc; parsed++) {
        const char *arg = argv[parsed];
        size_t arg_size = strlen(arg);
        if (arg_size == 0) {
            help(program, "empty argument", true);
        }

        if (arg[0] != '-') {
            break;
        }

        if (arg_size == 1) {
            return args;
        }

        int offset = 0;
        bool stop = false;
        for (int i = 1; i < arg_size; i++) {
            if (arg[i] == '-') {
                stop = true;
                offset++;
                break;
            }

            switch (arg[i]) {
                case 'l':
                    offset++;
                    if (parsed + offset >= argc) {
                        help(program, "bytes are expected", true);
                    }

                    args.alloc_limit = read_bytes(argv[parsed + offset], program);
                    break;
                case 'a':
                    args.custom_alloc = true;
                    break;
                case 'm':
                    args.measure_time = true;
                    break;
                case 'H':
                    args.human_readable = true;
                    break;
                case 'h':
                    help(program, NULL, false);
            }
        }

        parsed += offset;

        if (stop) {
            break;
        }
    }

    if (parsed >= argc) {
        help(program, "program path is expected", true);
    }

    args.program_path = argv[parsed];
    args.args = argv + parsed + 1;

    if (argc - (parsed + 1) < 0) {
        args.argc = 0;
    } else {
        args.argc = argc - (parsed + 1);
    }

    return args;
}
