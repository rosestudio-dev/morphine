//
// Created by why-iskra on 31.05.2024.
//

#include "impl.h"
#include "support/arguments.h"

void match_table(struct matcher *M) {
    struct argument_matcher A = {
        .assemble = false,
        .M = M,
        .separator = symbol_operator(TOP_COMMA),
        .has_open_close = true,
        .consume_open = true,
        .open_symbol = symbol_operator(TOP_LBRACE),
        .close_symbol = symbol_operator(TOP_RBRACE),
    };

    if (argument_matcher_init(&A, 0)) {
        do {
            matcher_reduce(M, REDUCE_TYPE_EXPRESSION);
            if (matcher_match(M, symbol_predef_word(TPW_to))) {
                matcher_reduce(M, REDUCE_TYPE_EXPRESSION);
            }
        } while (argument_matcher_next(&A));
    }
    argument_matcher_close(&A);
}

struct ast_node *assemble_table(morphine_coroutine_t U, struct elements *E) {
    struct argument_matcher A = {
        .assemble = true,
        .E = E,
        .U = U,
        .separator = symbol_operator(TOP_COMMA),
        .has_open_close = true,
        .consume_open = true,
        .open_symbol = symbol_operator(TOP_LBRACE),
        .close_symbol = symbol_operator(TOP_RBRACE),
    };

    if (argument_matcher_init(&A, 0)) {
        do {
            argument_matcher_reduce(&A, REDUCE_TYPE_EXPRESSION);
            if (argument_matcher_match(&A, symbol_predef_word(TPW_to))) {
                argument_matcher_reduce(&A, REDUCE_TYPE_EXPRESSION);
            }
        } while (argument_matcher_next(&A));
    }
    size_t size = argument_matcher_close(&A);

    struct expression_table *result = ast_create_expression_table(U, elements_get_token(E, 0).line, size);

    if (argument_matcher_init(&A, 0)) {
        do {
            struct reduce reduce = argument_matcher_reduce(&A, REDUCE_TYPE_EXPRESSION);

            if (argument_matcher_match(&A, symbol_predef_word(TPW_to))) {
                struct reduce reduce_value = argument_matcher_reduce(&A, REDUCE_TYPE_EXPRESSION);
                result->keys[A.count] = ast_node_as_expression(U, reduce.node);
                result->values[A.count] = ast_node_as_expression(U, reduce_value.node);
            } else {
                struct expression_value *index = ast_create_expression_value(
                    U, ast_node_line(reduce.node)
                );

                index->type = EXPRESSION_VALUE_TYPE_INT;
                index->value.integer = (ml_integer) A.count;

                result->keys[A.count] = ast_as_expression(index);
                result->values[A.count] = ast_node_as_expression(U, reduce.node);
            }
        } while (argument_matcher_next(&A));
    }
    argument_matcher_close(&A);

    return ast_as_node(result);
}
