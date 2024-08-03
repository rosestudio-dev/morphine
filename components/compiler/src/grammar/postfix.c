//
// Created by why-iskra on 31.05.2024.
//

#include "impl.h"

bool match_postfix(struct matcher *M, bool is_wrapped) {
    if (!is_wrapped) {
        matcher_reduce(M, REDUCE_TYPE_PRIMARY);
    }

    bool matched = matcher_match(M, symbol_operator(MCLTOP_PLUSPLUS)) ||
                   matcher_match(M, symbol_operator(MCLTOP_MINUSMINUS));

    return matched;
}

struct ast_node *assemble_postfix(morphine_coroutine_t U, struct ast *A, struct elements *E) {
    struct reduce reduce = elements_get_reduce(E, 0);
    if (elements_size(E) == 1) {
        return ast_as_node(ast_node_as_expression(U, reduce.node));
    }

    struct mc_lex_token watch_token = elements_get_token(E, 1);

    struct expression_increment *result = ast_create_expression_increment(U, A, watch_token.line);
    result->is_postfix = true;
    result->container = ast_node_as_expression(U, reduce.node);

    if (matcher_symbol(symbol_operator(MCLTOP_PLUSPLUS), watch_token)) {
        result->is_decrement = false;
    } else if (matcher_symbol(symbol_operator(MCLTOP_MINUSMINUS), watch_token)) {
        result->is_decrement = true;
    } else {
        return NULL;
    }

    return ast_as_node(result);
}
