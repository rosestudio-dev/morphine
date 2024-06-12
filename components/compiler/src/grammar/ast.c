//
// Created by why-iskra on 31.05.2024.
//

#include "impl.h"

void match_ast(struct matcher *M) {
    while (!matcher_match(M, symbol_eos)) {
        matcher_reduce(M, REDUCE_TYPE_BLOCK_ELEM);
    }
}

struct ast_node *assemble_ast(morphine_coroutine_t U, struct ast *A, struct elements *E) {
    if (elements_size(E) == 0) {
        return NULL;
    }

    size_t size = elements_size(E) - 1;
    struct statement_block *block = ast_create_statement_block(U, A, 0, size);

    for (size_t i = 0; i < size; i++) {
        struct reduce reduce = elements_get_reduce(E, i);
        block->statements[i] = ast_node_as_statement(U, reduce.node);
    }

    struct ast_function *function = ast_create_function(U, A, 0, 0, 0);
    function->line = 1;
    function->recursive = false;
    function->anonymous = false;
    function->name = ast_get_main_name(A);
    function->auto_closure = false;
    function->body = ast_as_statement(block);

    struct expression_function *result = ast_create_expression_function(U, A, 1);
    result->ref = function;

    return ast_as_node(result);
}
