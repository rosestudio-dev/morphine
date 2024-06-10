//
// Created by why-iskra on 02.06.2024.
//

#include "impl.h"

void match_primary(struct matcher *M) {
    if (matcher_look(M, symbol_operator(TOP_LBRACE)) || matcher_is_reduced(M, REDUCE_TYPE_TABLE)) {
        matcher_reduce(M, REDUCE_TYPE_TABLE);
        return;
    }

    if (matcher_look(M, symbol_operator(TOP_LBRACKET)) || matcher_is_reduced(M, REDUCE_TYPE_VECTOR)) {
        matcher_reduce(M, REDUCE_TYPE_VECTOR);
        return;
    }

    if (matcher_look(M, symbol_predef_word(TPW_fun)) || matcher_is_reduced(M, REDUCE_TYPE_FUNCTION)) {
        matcher_reduce(M, REDUCE_TYPE_FUNCTION);
        return;
    }

    if (matcher_look(M, symbol_predef_word(TPW_if)) || matcher_is_reduced(M, REDUCE_TYPE_EXPRESSION_IF)) {
        matcher_reduce(M, REDUCE_TYPE_EXPRESSION_IF);
        return;
    }

    if (matcher_match(M, symbol_predef_word(TPW_do))) {
        matcher_reduce(M, REDUCE_TYPE_EXPRESSION_BLOCK);
        return;
    }

    matcher_reduce(M, REDUCE_TYPE_VARIABLE);
}

struct ast_node *assemble_primary(morphine_coroutine_t U, struct ast *A, struct elements *E) {
    (void) A;

    if (!elements_is_token(E, 0)) {
        return elements_get_reduce(E, 0).node;
    }

    struct token watch_token = elements_get_token(E, 0);

    if (matcher_symbol(symbol_predef_word(TPW_do), watch_token)) {
        struct ast_node *node = elements_get_reduce(E, 1).node;
        return ast_as_node(ast_node_as_expression(U, node));
    }

    return NULL;
}
