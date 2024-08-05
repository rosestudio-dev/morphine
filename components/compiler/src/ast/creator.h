//
// Created by why-iskra on 12.08.2024.
//

#pragma once

#include "morphinec/ast.h"

#define ast_args(args...) , args
#define ast_noargs
#define ast_impl_node(ntype, name, args...) MORPHINE_API struct mc_ast_##ntype##_##name *mcapi_ast_create_##ntype##_##name(morphine_coroutine_t U, struct mc_ast *A, ml_line line args)

#define ast_impl_expr(name, args) ast_impl_node(expression, name, args)
#define ast_impl_stmt(name, args) ast_impl_node(statement, name, args)

#define ast_standard_impl_expr(name) ast_impl_expr(name, ast_noargs) { return (struct mc_ast_expression_##name *) ast_create_expression(U, A, MCEXPRT_##name, line, sizeof(struct mc_ast_expression_##name)); }
#define ast_standard_impl_stmt(name) ast_impl_stmt(name, ast_noargs) { return (struct mc_ast_statement_##name *) ast_create_statement(U, A, MCSTMTT_##name, line, sizeof(struct mc_ast_statement_##name)); }

struct mc_ast_expression *ast_create_expression(
    morphine_coroutine_t,
    struct mc_ast *,
    enum mc_expression_type,
    ml_line,
    size_t size
);

struct mc_ast_statement *ast_create_statement(
    morphine_coroutine_t,
    struct mc_ast *,
    enum mc_statement_type,
    ml_line,
    size_t size
);
