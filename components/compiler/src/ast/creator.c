//
// Created by why-iskra on 12.08.2024.
//

#include "creator.h"
#include "morphine/utils/overflow.h"

#define ast_args(args...) , args
#define ast_noargs
#define ast_impl_node(ntype, name, args...) MORPHINE_API struct mc_ast_##ntype##_##name *mcapi_ast_create_##ntype##_##name(morphine_coroutine_t U, struct mc_ast *A, ml_size from, ml_size to, ml_line line args)

#define ast_impl_expr(name, args) ast_impl_node(expression, name, args)
#define ast_impl_stmt(name, args) ast_impl_node(statement, name, args)

#define ast_standard_impl_expr(name) ast_impl_expr(name, ast_noargs) { return (struct mc_ast_expression_##name *) ast_create_expression(U, A, MCEXPRT_##name, line, from, to, sizeof(struct mc_ast_expression_##name)); }
#define ast_standard_impl_stmt(name) ast_impl_stmt(name, ast_noargs) { return (struct mc_ast_statement_##name *) ast_create_statement(U, A, MCSTMTT_##name, line, from, to, sizeof(struct mc_ast_statement_##name)); }

#define ast_overflow_error(name) mapi_errorf(U, "line %"MLIMIT_LINE_PR": "name" overflow", line)

// statements

ast_standard_impl_stmt(eval)
ast_standard_impl_stmt(pass)
ast_standard_impl_stmt(yield)
ast_standard_impl_stmt(while)
ast_standard_impl_stmt(for)
ast_standard_impl_stmt(assigment)
ast_standard_impl_stmt(declaration)

// expressions

ast_standard_impl_expr(value)
ast_standard_impl_expr(function)
ast_standard_impl_expr(env)
ast_standard_impl_expr(leave)
ast_standard_impl_expr(break)
ast_standard_impl_expr(continue)
ast_standard_impl_expr(variable)
ast_standard_impl_expr(binary)
ast_standard_impl_expr(unary)
ast_standard_impl_expr(increment)
ast_standard_impl_expr(access)
ast_standard_impl_expr(if)

ast_impl_expr(vector, ast_args(size_t count)) {
    size_t alloc_size_values = mm_overflow_opc_mul(
        sizeof(struct mc_ast_expression *),
        count,
        ast_overflow_error("values")
    );

    size_t alloc_size = mm_overflow_opc_add(
        sizeof(struct mc_ast_expression_vector),
        alloc_size_values,
        ast_overflow_error("vector")
    );

    struct mc_ast_expression *expression = ast_create_expression(
        U, A, MCEXPRT_vector, line, from, to, alloc_size
    );

    struct mc_ast_expression_vector *vector =
        mcapi_ast_expression2vector(U, expression);

    vector->count = count;
    vector->expressions = ((void *) vector) + sizeof(struct mc_ast_expression_vector);

    return vector;
}

ast_impl_expr(table, ast_args(size_t count)) {
    size_t alloc_size_keys = mm_overflow_opc_mul(
        sizeof(struct mc_ast_expression *),
        count,
        ast_overflow_error("keys")
    );

    size_t alloc_size_values = mm_overflow_opc_mul(
        sizeof(struct mc_ast_expression *),
        count,
        ast_overflow_error("values")
    );

    size_t alloc_size = sizeof(struct mc_ast_expression_table);
    alloc_size = mm_overflow_opc_add(alloc_size, alloc_size_keys, ast_overflow_error("table"));
    alloc_size = mm_overflow_opc_add(alloc_size, alloc_size_values, ast_overflow_error("table"));

    struct mc_ast_expression *expression = ast_create_expression(
        U, A, MCEXPRT_table, line, from, to, alloc_size
    );

    struct mc_ast_expression_table *table =
        mcapi_ast_expression2table(U, expression);

    table->count = count;

    table->keys = ((void *) table) + sizeof(struct mc_ast_expression_table);
    table->values = ((void *) table->keys) + alloc_size_keys;

    return table;
}

ast_impl_expr(call, ast_args(size_t args_count)) {
    size_t alloc_size_statements = mm_overflow_opc_mul(
        sizeof(struct mc_ast_expression *),
        args_count,
        ast_overflow_error("args")
    );

    size_t alloc_size = mm_overflow_opc_add(
        sizeof(struct mc_ast_expression_call),
        alloc_size_statements,
        ast_overflow_error("call")
    );

    struct mc_ast_expression *expression = ast_create_expression(
        U, A, MCEXPRT_call, line, from, to, alloc_size
    );

    struct mc_ast_expression_call *call =
        mcapi_ast_expression2call(U, expression);

    call->args_count = args_count;
    call->arguments = ((void *) call) + sizeof(struct mc_ast_expression_call);

    return call;
}

ast_impl_expr(block, ast_args(size_t count)) {
    size_t alloc_size_statements = mm_overflow_opc_mul(
        sizeof(struct mc_ast_statement *),
        count,
        ast_overflow_error("statements")
    );

    size_t alloc_size = mm_overflow_opc_add(
        sizeof(struct mc_ast_expression_block),
        alloc_size_statements,
        ast_overflow_error("block")
    );

    struct mc_ast_expression *expression = ast_create_expression(
        U, A, MCEXPRT_block, line, from, to, alloc_size
    );

    struct mc_ast_expression_block *block =
        mcapi_ast_expression2block(U, expression);

    block->count = count;
    block->inlined = false;
    block->statements = ((void *) block) + sizeof(struct mc_ast_expression_block);

    return block;
}
