//
// Created by why-iskra on 01.06.2024.
//

#pragma once

#include "elements.h"
#include "morphinec/ast.h"

bool match_decompose(struct matcher *, bool is_word);

size_t size_decompose(
    morphine_coroutine_t,
    struct elements *,
    bool is_word,
    size_t start_index,
    size_t *end_index
);

void insert_decompose(
    morphine_coroutine_t,
    struct ast *,
    struct elements *,
    bool is_word,
    size_t start_index,
    mc_strtable_index_t *names,
    struct expression **expressions,
    struct expression **keys
);
