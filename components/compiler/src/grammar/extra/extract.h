//
// Created by why-iskra on 12.08.2024.
//

#pragma once

#include "../controller.h"

size_t extra_consume_extract(struct parse_controller *, bool is_word);

void extra_get_extract(
    struct parse_controller *,
    bool is_word,
    struct mc_ast_expression_variable **variables,
    struct mc_ast_expression **expressions,
    struct mc_ast_expression **keys
);
