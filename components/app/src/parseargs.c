//
// Created by whyiskra on 3/16/24.
//

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include <parseargs.h>
#include <ctype.h>

static void help(const char *program, const char *message, bool disable_usage) {
    printf("Morphine Language\n");
    if (message != NULL) {
        printf("Message: %s\n\n", message);
    }

    if (!disable_usage) {
        printf("Usage: %s [options] [program [args]]\n", program);
        printf("Positional arguments:\n");
        printf("    program         path to program\n");
        printf("    args            program arguments\n");
        printf("Optional arguments:\n");
        printf("    -b              Binary program\n");
        printf("    -r              Run program\n");
        printf("    -e              Export program\n");
        printf("    -d              Disassembly program\n");
        printf("    -l bytes [256M] Limit of allocation in bytes (suffixes: K, M, G)\n");
        printf("    -m, -M          Enable executing time measure (Use uppercase for pretty printing)\n");
        printf("    -a, -A          Use debug allocator (Use uppercase for pretty printing)\n");
        printf("    -L bytes [256M] Limit of allocation for debug allocator in bytes (suffixes: K, M, G)\n");
        printf("    -v              Print version\n");
        printf("    -h              Usage info\n");
        printf("    --              Stop handling options\n");
    }

    exit(1);
}

static size_t read_bytes(const char *str, const char *program) {
    char *end;
    size_t bytes = strtoumax(str, &end, 10);

    switch (tolower(*end)) {
        case '\0':
            return bytes;
        case 'g':
            bytes *= 1024 * 1024 * 1024;
            break;
        case 'm':
            bytes *= 1024 * 1024;
            break;
        case 'k':
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
        .alloc_limit = 1024 * 1024 * 256,
        .custom_alloc_limit = 1024 * 1024 * 256,
        .measure_time = false,
        .measure_time_pretty = false,
        .custom_alloc = false,
        .custom_alloc_pretty = false,
        .binary = false,
        .run = false,
        .disassembly = false,
        .export_path = NULL,
        .program_path = NULL,
        .argc = 0,
        .args = NULL,
        .version = false
    };

    const char *program;
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
        if (arg_size < 1) {
            help(program, "empty argument", true);
        }

        if (arg[0] != '-') {
            break;
        }

        int offset = 0;
        bool stop = false;
        for (size_t i = 1; i < arg_size; i++) {
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
                case 'L':
                    offset++;
                    if (parsed + offset >= argc) {
                        help(program, "bytes are expected", true);
                    }

                    args.custom_alloc_limit = read_bytes(argv[parsed + offset], program);
                    break;
                case 'a':
                    args.custom_alloc = true;
                    args.custom_alloc_pretty = false;
                    break;
                case 'A':
                    args.custom_alloc = true;
                    args.custom_alloc_pretty = true;
                    break;
                case 'm':
                    args.measure_time = true;
                    args.measure_time_pretty = false;
                    break;
                case 'M':
                    args.measure_time = true;
                    args.measure_time_pretty = true;
                    break;
                case 'b':
                    args.binary = true;
                    break;
                case 'r':
                    args.run = true;
                    break;
                case 'e':
                    offset++;
                    if (parsed + offset >= argc) {
                        help(program, "export path expected", true);
                    }

                    args.export_path = argv[parsed + offset];
                    break;
                case 'd':
                    args.disassembly = true;
                    break;
                case 'v':
                    args.version = true;
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
        args.argc = (size_t) (argc - (parsed + 1));
    }

    return args;
}
