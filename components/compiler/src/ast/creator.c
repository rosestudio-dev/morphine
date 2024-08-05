//
// Created by why-iskra on 12.08.2024.
//

#include "creator.h"

// statements

ast_standard_impl_stmt(eval)
ast_standard_impl_stmt(simple)
ast_standard_impl_stmt(leave)
ast_standard_impl_stmt(while)
ast_standard_impl_stmt(for)
ast_standard_impl_stmt(iterator)
ast_standard_impl_stmt(if)

ast_impl_stmt(block, ast_args(size_t count)) {
    size_t size = sizeof(struct mc_ast_statement_block) +
                  sizeof(struct mc_ast_statement *) * count;

    struct mc_ast_statement *statement = ast_create_statement(
        U, A, MCSTMTT_block, line, size
    );

    struct mc_ast_statement_block *block =
        mcapi_ast_statement2block(U, statement);

    block->count = count;
    block->statements = ((void *) block) + sizeof(struct mc_ast_statement_block);

    return block;
}

ast_impl_stmt(assigment, ast_args(size_t count)) {
    size_t size = sizeof(struct mc_ast_statement_assigment) +
                  sizeof(struct mc_ast_expression *) * count +
                  sizeof(struct mc_ast_expression *) * count;

    struct mc_ast_statement *statement = ast_create_statement(
        U, A, MCSTMTT_assigment, line, size
    );

    struct mc_ast_statement_assigment *assigment =
        mcapi_ast_statement2assigment(U, statement);

    assigment->is_decompose = count > 0;
    assigment->decompose.size = count;
    assigment->decompose.keys = ((void *) assigment) + sizeof(struct mc_ast_statement_assigment);
    assigment->decompose.values = ((void *) assigment) +
                              sizeof(struct mc_ast_statement_assigment) +
                              sizeof(struct mc_ast_expression *) * count;

    return assigment;
}

ast_impl_stmt(declaration, ast_args(size_t count)) {
    size_t size = sizeof(struct mc_ast_statement_declaration) +
                  sizeof(struct mc_ast_expression_variable *) * count +
                  sizeof(struct mc_ast_expression *) * count;

    struct mc_ast_statement *statement = ast_create_statement(
        U, A, MCSTMTT_declaration, line, size
    );

    struct mc_ast_statement_declaration *declaration =
        mcapi_ast_statement2declaration(U, statement);

    declaration->is_decompose = count > 0;
    declaration->decompose.size = count;
    declaration->decompose.keys = ((void *) declaration) + sizeof(struct mc_ast_statement_declaration);
    declaration->decompose.values = ((void *) declaration) +
                              sizeof(struct mc_ast_statement_declaration) +
                              sizeof(struct mc_ast_expression *) * count;

    return declaration;
}

// expressions

ast_standard_impl_expr(value)
ast_standard_impl_expr(function)
ast_standard_impl_expr(global)
ast_standard_impl_expr(variable)
ast_standard_impl_expr(binary)
ast_standard_impl_expr(unary)
ast_standard_impl_expr(increment)
ast_standard_impl_expr(access)
ast_standard_impl_expr(if)

ast_impl_expr(vector, ast_args(size_t count)) {
    size_t size = sizeof(struct mc_ast_expression_vector) +
                  sizeof(struct mc_ast_expression *) * count;

    struct mc_ast_expression *expression = ast_create_expression(
        U, A, MCEXPRT_vector, line, size
    );

    struct mc_ast_expression_vector *vector =
        mcapi_ast_expression2vector(U, expression);

    vector->count = count;
    vector->expressions = ((void *) vector) + sizeof(struct mc_ast_expression_vector);

    return vector;
}

ast_impl_expr(table, ast_args(size_t count)) {
    size_t size = sizeof(struct mc_ast_expression_table) +
                  sizeof(struct mc_ast_expression *) * count * 2;

    struct mc_ast_expression *expression = ast_create_expression(
        U, A, MCEXPRT_table, line, size
    );

    struct mc_ast_expression_table *table =
        mcapi_ast_expression2table(U, expression);

    table->count = count;

    table->keys = ((void *) table) +
                  sizeof(struct mc_ast_expression_table);

    table->values = ((void *) table) +
                    sizeof(struct mc_ast_expression_table) +
                    sizeof(struct mc_ast_expression *) * count;

    return table;
}

ast_impl_expr(call, ast_args(size_t args_count)) {
    size_t size = sizeof(struct mc_ast_expression_call) +
                  sizeof(struct mc_ast_expression *) * args_count;

    struct mc_ast_expression *expression = ast_create_expression(
        U, A, MCEXPRT_call, line, size
    );

    struct mc_ast_expression_call *call =
        mcapi_ast_expression2call(U, expression);

    call->args_count = args_count;
    call->arguments = ((void *) call) + sizeof(struct mc_ast_expression_call);

    return call;
}

ast_impl_expr(block, ast_args(size_t count)) {
    size_t size = sizeof(struct mc_ast_expression_block) +
                  sizeof(struct mc_ast_statement *) * count;

    struct mc_ast_expression *expression = ast_create_expression(
        U, A, MCEXPRT_block, line, size
    );

    struct mc_ast_expression_block *block =
        mcapi_ast_expression2block(U, expression);

    block->count = count;
    block->statements = ((void *) block) + sizeof(struct mc_ast_expression_block);

    return block;
}
