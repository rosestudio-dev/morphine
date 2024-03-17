//
// Created by whyiskra on 3/16/24.
//

#include <stdio.h>
#include <stdlib.h>
#include "allocator.h"

#define max(a, b) ({ __typeof__ (a) _a = (a); __typeof__ (b) _b = (b); _a > _b ? _a : _b; })

struct allocated_block {
    void *pointer;
    size_t size;
    struct allocated_block *prev;
};

void allocator_init(struct allocator *allocator) {
    *allocator = (struct allocator) {
        .allocated = NULL,
        .allocated_bytes = 0,
        .allocations_count = 0,
        .peak_allocated_bytes = 0
    };
}

void *allocator_alloc(struct allocator *allocator, size_t size) {
    if (size == 0) {
        fprintf(stderr, "Allocator: Attempt to allocate zero size\n");
        abort();
    }

    void *pointer = malloc(size);
    struct allocated_block *block = (struct allocated_block *) malloc(sizeof(struct allocated_block));

    if (block == NULL || pointer == NULL) {
        fprintf(stderr, "Allocator: out of memory\n");
        abort();
    }

    (*block) = (struct allocated_block) {
        .pointer = pointer,
        .size = size,
        .prev = allocator->allocated
    };

    allocator->allocated = block;

    allocator->allocated_bytes += size;

    allocator->allocations_count++;
    allocator->peak_allocated_bytes = max(allocator->peak_allocated_bytes, allocator->allocated_bytes);

    return pointer;
}

void *allocator_realloc(struct allocator *allocator, void *pointer, size_t size) {
    if (size == 0) {
        fprintf(stderr, "Allocator: Attempt to reallocate to zero size\n");
        abort();
    }

    if (pointer == NULL) {
        fprintf(stderr, "Allocator: Attempt to reallocate null\n");
        abort();
    }

    struct allocated_block *current = allocator->allocated;
    while (current != NULL) {
        if (current->pointer == pointer) {
            break;
        }

        current = current->prev;
    }

    if (current == NULL) {
        fprintf(stderr, "Allocator: %p not found\n", pointer);
        abort();
    }

    current->pointer = realloc(pointer, size);

    if (current->pointer == NULL) {
        fprintf(stderr, "Allocator: out of memory\n");
        abort();
    }

    allocator->allocated_bytes += size - current->size;

    current->size = size;

    allocator->allocations_count++;
    allocator->peak_allocated_bytes = max(allocator->peak_allocated_bytes, allocator->allocated_bytes);

    return pointer;
}

void allocator_free(struct allocator *allocator, void *pointer) {
    if (pointer == NULL) {
        fprintf(stderr, "Allocator: Attempt to free null\n");
        abort();
    }

    struct allocated_block *found = NULL;

    {
        struct allocated_block *current = allocator->allocated;
        struct allocated_block *last = NULL;
        while (current != NULL) {
            if (current->pointer == pointer) {
                found = current;

                if (last == NULL) {
                    allocator->allocated = current->prev;
                } else {
                    last->prev = current->prev;
                }

                break;
            }

            last = current;
            current = current->prev;
        }
    }

    if (found == NULL) {
        fprintf(stderr, "Allocator: Attempt to free unallocated pointer %p\n", pointer);
        abort();
    }

    free(found->pointer);

    allocator->allocated_bytes -= found->size;

    free(found);
}

void allocator_clear(struct allocator *allocator) {
    struct allocated_block *current = allocator->allocated;
    while (current != NULL) {
        struct allocated_block *prev = current->prev;

        free(current->pointer);
        free(current);

        current = prev;
    }

    allocator->allocated = NULL;
}