//
// Created by why on 3/23/24.
//

#pragma once

#include "morphine/platform.h"

typedef enum {
    GC_STATUS_IDLE,
    GC_STATUS_PREPARE,
    GC_STATUS_INCREMENT,
    GC_STATUS_FINALIZE_PREPARE,
    GC_STATUS_FINALIZE_INCREMENT,
    GC_STATUS_SWEEP,
} gc_status_t;

struct garbage_collector {
    gc_status_t status;
    size_t cycle;

    bool enabled;

    struct {
        size_t limit_bytes;
        size_t start;
        size_t grow;
        size_t deal;
    } settings;

    struct {
        size_t started;
        size_t allocated;
        size_t prev_allocated;
        size_t max_allocated;
    } bytes;

    struct {
        struct object *allocated;
        struct object *gray;
        struct object *white;

        struct object *finalize;
    } pools;

    struct {
        bool work;
        struct object *candidate;
        morphine_state_t state;
    } finalizer;

    struct callinfo *callinfo_trash;
};

struct garbage_collector gcI_init(struct params, size_t inited_size);
