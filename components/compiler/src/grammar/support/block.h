//
// Created by why-iskra on 02.06.2024.
//

#pragma once

#include "elements.h"
#include "morphinec/ast.h"

size_t match_sblock(
    struct matcher *,
    size_t closes_size,
    struct matcher_symbol *closes
);

size_t get_sblock(
    morphine_coroutine_t,
    struct ast *,
    struct elements *,
    size_t closes_size,
    struct matcher_symbol *closes,
    size_t start_index,
    struct statement **,
    size_t *end_index
);

size_t match_eblock(
    struct matcher *,
    size_t closes_size,
    struct matcher_symbol *closes
);

size_t get_eblock(
    morphine_coroutine_t,
    struct ast *,
    struct elements *,
    size_t closes_size,
    struct matcher_symbol *closes,
    size_t start_index,
    struct expression **,
    size_t *end_index
);