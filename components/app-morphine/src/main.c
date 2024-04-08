//
// Created by whyiskra on 30.01.24.
//

#include <stdio.h>
#include <parseargs.h>
#include <execute.h>
#include <millis.h>
#include <human.h>
#include "morphine/api.h"

int main(int argc, char **argv) {
    struct args args = parseargs(argc, argv);

    if(args.version) {
        printf("Morphine version: %s\n", mapi_version());
        return 0;
    }

    struct allocator allocator;
    struct allocator *pallocator = NULL;
    if (args.custom_alloc) {
        allocator_init(&allocator, args.custom_alloc_limit);
        pallocator = &allocator;
    }

    uint64_t start_ms = millis();

    execute(
        pallocator,
        args.program_path,
        args.binary,
        args.alloc_limit
    );

    uint64_t end_ms = millis();

    if (args.measure_time) {
        char buffer[32];

        uint64_t time = end_ms - start_ms;
        if (args.measure_time_pretty) {
            human_time(time, buffer, 32);
            printf("\nExecuted in %s\n", buffer);
        } else {
            printf("\nExecuted in %lums\n", time);
        }
    }

    if (args.custom_alloc) {
        char buffer[32];
        printf("Allocator:\n");

        if (allocator.allocated_bytes > 0) {
            if (args.custom_alloc_pretty) {
                human_size(allocator.allocated_bytes, buffer, 32);
                printf("  lost:       \x1b[31m%s\x1b[0m\n", buffer);
            } else {
                printf("  lost:       \x1b[31m%zuB\x1b[0m\n", allocator.allocated_bytes);
            }
        }

        if (args.custom_alloc_pretty) {
            human_size(allocator.peak_allocated_bytes, buffer, 32);
            printf("  peak:       %s\n", buffer);
        } else {
            printf("  peak:       %zuB\n", allocator.peak_allocated_bytes);
        }

        printf("  call count: %zu\n", allocator.allocations_count);

        allocator_clear(&allocator);
    }

    return 0;
}