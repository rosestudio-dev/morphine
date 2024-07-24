//
// Created by why-iskra on 31.05.2024.
//

#include "impl.h"
#include "support/arguments.h"

void match_table(struct matcher *M) {
    struct argument_matcher R = {
        .assemble = false,
        .M = M,
        .separator = symbol_operator(TOP_COMMA),
        .has_open_close = true,
        .consume_open = true,
        .open_symbol = symbol_operator(TOP_LBRACE),
        .close_symbol = symbol_operator(TOP_RBRACE),
    };

    if (argument_matcher_init(&R, 0)) {
        do {
            if (!matcher_match(M, symbol_word)) {
                matcher_reduce(M, REDUCE_TYPE_EXPRESSION);
            }

            if (matcher_match(M, symbol_operator(TOP_EQ))) {
                matcher_reduce(M, REDUCE_TYPE_EXPRESSION);
            }
        } while (argument_matcher_next(&R));
    }
    argument_matcher_close(&R);
}

struct ast_node *assemble_table(morphine_coroutine_t U, struct ast *A, struct elements *E) {
    struct argument_matcher R = {
        .assemble = true,
        .E = E,
        .U = U,
        .separator = symbol_operator(TOP_COMMA),
        .has_open_close = true,
        .consume_open = true,
        .open_symbol = symbol_operator(TOP_LBRACE),
        .close_symbol = symbol_operator(TOP_RBRACE),
    };

    if (argument_matcher_init(&R, 0)) {
        do {
            if (!argument_matcher_match(&R, symbol_word)) {
                argument_matcher_reduce(&R, REDUCE_TYPE_EXPRESSION);
            }

            if (argument_matcher_match(&R, symbol_operator(TOP_EQ))) {
                argument_matcher_reduce(&R, REDUCE_TYPE_EXPRESSION);
            }
        } while (argument_matcher_next(&R));
    }
    size_t size = argument_matcher_close(&R);

    struct expression_table *result = ast_create_expression_table(U, A, elements_get_token(E, 0).line, size);

    if (argument_matcher_init(&R, 0)) {
        do {
            struct expression *expression;

            if (argument_matcher_look(&R, symbol_word)) {
                struct token token = argument_matcher_consume(&R, symbol_word);
                struct expression_value *key = ast_create_expression_value(U, A, token.line);
                key->type = EXPRESSION_VALUE_TYPE_STR;
                key->value.string = token.word;

                expression = ast_as_expression(key);
            } else {
                struct reduce reduce = argument_matcher_reduce(&R, REDUCE_TYPE_EXPRESSION);
                expression = ast_node_as_expression(U, reduce.node);
            }

            if (argument_matcher_match(&R, symbol_operator(TOP_EQ))) {
                struct reduce reduce_value = argument_matcher_reduce(&R, REDUCE_TYPE_EXPRESSION);
                result->keys[R.count] = expression;
                result->values[R.count] = ast_node_as_expression(U, reduce_value.node);
            } else {
                struct expression_value *index = ast_create_expression_value(
                    U, A, ast_node_line(expression)
                );

                index->type = EXPRESSION_VALUE_TYPE_INT;
                index->value.integer = (ml_integer) R.count;

                result->keys[R.count] = ast_as_expression(index);
                result->values[R.count] = expression;
            }
        } while (argument_matcher_next(&R));
    }
    argument_matcher_close(&R);

    return ast_as_node(result);
}
