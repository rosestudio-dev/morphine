//
// Created by why-iskra on 31.05.2024.
//

#include "impl.h"
#include "support/arguments.h"

void match_vector(struct matcher *M) {
    struct argument_matcher R = {
        .assemble = false,
        .M = M,
        .separator = symbol_operator(MCLTOP_COMMA),
        .has_open_close = true,
        .consume_open = true,
        .open_symbol = symbol_operator(MCLTOP_LBRACKET),
        .close_symbol = symbol_operator(MCLTOP_RBRACKET),
    };

    if (argument_matcher_init(&R, 0)) {
        do {
            matcher_reduce(M, REDUCE_TYPE_EXPRESSION);
        } while (argument_matcher_next(&R));
    }
    argument_matcher_close(&R);
}


struct ast_node *assemble_vector(morphine_coroutine_t U, struct ast *A, struct elements *E) {
    struct argument_matcher R = {
        .assemble = true,
        .E = E,
        .U = U,
        .separator = symbol_operator(MCLTOP_COMMA),
        .has_open_close = true,
        .consume_open = true,
        .open_symbol = symbol_operator(MCLTOP_LBRACKET),
        .close_symbol = symbol_operator(MCLTOP_RBRACKET),
    };

    if (argument_matcher_init(&R, 0)) {
        do {
            argument_matcher_reduce(&R, REDUCE_TYPE_EXPRESSION);
        } while (argument_matcher_next(&R));
    }
    size_t size = argument_matcher_close(&R);

    struct expression_vector *result = ast_create_expression_vector(
        U, A, elements_get_token(E, 0).line, size
    );

    if (argument_matcher_init(&R, 0)) {
        do {
            struct reduce reduce = argument_matcher_reduce(&R, REDUCE_TYPE_EXPRESSION);
            result->values[R.count] = ast_node_as_expression(U, reduce.node);
        } while (argument_matcher_next(&R));
    }
    argument_matcher_close(&R);

    return ast_as_node(result);
}