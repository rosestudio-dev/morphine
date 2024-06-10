//
// Created by why-iskra on 01.06.2024.
//

#include "decompose.h"
#include "arguments.h"

bool match_decompose(struct matcher *M, bool is_word) {
    if (matcher_match(M, symbol_predef_word(TPW_decompose))) {
        struct argument_matcher R = {
            .assemble = false,
            .M = M,
            .separator = symbol_operator(TOP_COMMA),
            .has_open_close = true,
            .consume_open = false,
            .open_symbol = symbol_operator(TOP_LPAREN),
            .close_symbol = symbol_operator(TOP_RPAREN),
        };

        if (argument_matcher_init(&R, 0)) {
            do {
                if (is_word) {
                    matcher_consume(M, symbol_word);
                } else {
                    matcher_reduce(M, REDUCE_TYPE_EXPRESSION);
                }
            } while (argument_matcher_next(&R));
        }
        size_t size = argument_matcher_close(&R);

        if (size == 0) {
            matcher_error(M, "empty decomposition");
        }

        if (matcher_match(M, symbol_predef_word(TPW_as))) {
            if (argument_matcher_init(&R, 0)) {
                do {
                    matcher_reduce(M, REDUCE_TYPE_EXPRESSION);
                } while (argument_matcher_next(&R));
            }
            size_t as_size = argument_matcher_close(&R);

            if (size != as_size) {
                matcher_error(M, "keys for decomposition aren't defined correctly");
            }
        }

        return true;
    } else if (is_word) {
        matcher_consume(M, symbol_word);
    } else {
        matcher_reduce(M, REDUCE_TYPE_EXPRESSION);
    }

    return false;
}

size_t size_decompose(
    morphine_coroutine_t U,
    struct elements *E,
    bool is_word,
    size_t start_index,
    size_t *end_index
) {
    if (elements_look(E, start_index, symbol_predef_word(TPW_decompose))) {
        struct argument_matcher R = {
            .assemble = true,
            .E = E,
            .U = U,
            .separator = symbol_operator(TOP_COMMA),
            .has_open_close = true,
            .consume_open = false,
            .open_symbol = symbol_operator(TOP_LPAREN),
            .close_symbol = symbol_operator(TOP_RPAREN),
        };

        if (argument_matcher_init(&R, start_index + 1)) {
            do {
                if (is_word) {
                    argument_matcher_consume(&R, symbol_word);
                } else {
                    argument_matcher_reduce(&R, REDUCE_TYPE_EXPRESSION);
                }
            } while (argument_matcher_next(&R));
        }
        size_t size = argument_matcher_close(&R);

        if (elements_look(E, R.pos, symbol_predef_word(TPW_as))) {
            if (argument_matcher_init(&R, R.pos + 1)) {
                do {
                    argument_matcher_reduce(&R, REDUCE_TYPE_EXPRESSION);
                } while (argument_matcher_next(&R));
            }
            size_t as_size = argument_matcher_close(&R);

            if (size != as_size) {
                elements_error(E, 0, "keys for decomposition aren't defined correctly");
            }
        }

        if (end_index != NULL) {
            *end_index = R.pos;
        }

        return size;
    }

    if (end_index != NULL) {
        *end_index = start_index + 1;
    }

    return 0;
}

void insert_decompose(
    morphine_coroutine_t U,
    struct ast *A,
    struct elements *E,
    bool is_word,
    size_t start_index,
    strtable_index_t *names,
    struct expression **expressions,
    struct expression **keys
) {
    if (elements_look(E, start_index, symbol_predef_word(TPW_decompose))) {
        struct argument_matcher R = {
            .assemble = true,
            .E = E,
            .U = U,
            .separator = symbol_operator(TOP_COMMA),
            .has_open_close = true,
            .consume_open = false,
            .open_symbol = symbol_operator(TOP_LPAREN),
            .close_symbol = symbol_operator(TOP_RPAREN),
        };

        if (argument_matcher_init(&R, start_index + 1)) {
            do {
                if (is_word) {
                    struct token token = argument_matcher_consume(&R, symbol_word);
                    names[R.count] = token.word;
                } else {
                    struct ast_node *node = argument_matcher_reduce(&R, REDUCE_TYPE_EXPRESSION).node;
                    expressions[R.count] = ast_node_as_expression(U, node);
                }
            } while (argument_matcher_next(&R));
        }
        size_t size = argument_matcher_close(&R);

        if (elements_look(E, R.pos, symbol_predef_word(TPW_as))) {
            if (argument_matcher_init(&R, R.pos + 1)) {
                do {
                    struct ast_node *node = argument_matcher_reduce(&R, REDUCE_TYPE_EXPRESSION).node;
                    keys[R.count] = ast_node_as_expression(U, node);
                } while (argument_matcher_next(&R));
            }
            size_t as_size = argument_matcher_close(&R);

            if (size != as_size) {
                elements_error(E, 0, "keys for decomposition aren't defined correctly");
            }
        } else {
            for (size_t i = 0; i < size; i++) {
                struct expression_value *value = ast_create_expression_value(
                    U, A, elements_line(E, start_index)
                );

                value->type = EXPRESSION_VALUE_TYPE_INT;
                value->value.integer = (ml_integer) i;

                keys[i] = ast_as_expression(value);
            }
        }
    } else if (is_word) {
        *names = elements_get_token(E, start_index).word;
    } else {
        struct ast_node *node = elements_get_reduce(E, start_index).node;
        *expressions = ast_node_as_expression(U, node);
    }
}
