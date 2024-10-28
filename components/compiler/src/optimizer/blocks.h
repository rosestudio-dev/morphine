//
// Created by why-iskra on 06.10.2024.
//

#pragma once

#include <morphine.h>
#include "instructions.h"

struct block {
    ml_size from;
    ml_size to;
};

struct blockedges {
    size_t count;
    ml_size edges[2];
};

struct blocks;

struct blocks *blocks_alloc(morphine_coroutine_t);
void blocks_free(morphine_instance_t, struct blocks *);
void blocks_build(morphine_coroutine_t, struct instructions *, struct blocks *);
void blocks_reformat(morphine_coroutine_t, struct blocks *);
ml_size blocks_size(struct blocks *);
struct block blocks_get(morphine_coroutine_t, struct blocks *, ml_size);
struct blockedges blocks_edges(morphine_coroutine_t, struct instructions *, struct blocks *, ml_size);
