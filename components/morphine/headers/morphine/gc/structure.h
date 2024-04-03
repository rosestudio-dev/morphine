//
// Created by whyiskra on 3/23/24.
//

#pragma once

#include "morphine/core/value.h"

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

    bool enabled;

    struct gc_settings settings;

    struct {
        size_t debt;
    } stats;

    struct {
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

    struct {
        struct value value;
    } safe;

    struct callinfo *callinfo_trash;
};

struct garbage_collector gcI_prototype(struct gc_settings, size_t inited_bytes);
void gcI_destruct(morphine_instance_t, struct garbage_collector);
