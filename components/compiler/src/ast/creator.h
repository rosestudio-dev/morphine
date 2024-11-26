//
// Created by why-iskra on 12.08.2024.
//

#pragma once

#include "morphinec/ast.h"

struct mc_ast_expression *ast_create_expression(
    morphine_coroutine_t,
    struct mc_ast *,
    enum mc_expression_type,
    ml_line,
    ml_size from,
    ml_size to,
    size_t size
);

struct mc_ast_statement *ast_create_statement(
    morphine_coroutine_t,
    struct mc_ast *,
    enum mc_statement_type,
    ml_line,
    ml_size from,
    ml_size to,
    size_t size
);
