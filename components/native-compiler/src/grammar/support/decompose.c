//
// Created by why-iskra on 01.06.2024.
//

#include "decompose.h"
#include "arguments.h"

bool match_decompose(struct matcher *M, bool is_word) {
    if (matcher_match(M, symbol_predef_word(TPW_decompose))) {
        struct argument_matcher A = {
            .assemble = false,
            .M = M,
            .separator = symbol_operator(TOP_COMMA),
            .has_open_close = true,
            .consume_open = false,
            .open_symbol = symbol_operator(TOP_LPAREN),
            .close_symbol = symbol_operator(TOP_RPAREN),
        };

        if (argument_matcher_init(&A, 0)) {
            do {
                if (is_word) {
                    matcher_consume(M, symbol_word);
                } else {
                    matcher_reduce(M, REDUCE_TYPE_EXPRESSION);
                }
            } while (argument_matcher_next(&A));
        }
        size_t size = argument_matcher_close(&A);

        if (size == 0) {
            matcher_error(M, "empty decomposition");
        }

        if (matcher_match(M, symbol_predef_word(TPW_as))) {
            if (argument_matcher_init(&A, 0)) {
                do {
                    matcher_reduce(M, REDUCE_TYPE_EXPRESSION);
                } while (argument_matcher_next(&A));
            }
            size_t as_size = argument_matcher_close(&A);

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
        struct argument_matcher A = {
            .assemble = true,
            .E = E,
            .U = U,
            .separator = symbol_operator(TOP_COMMA),
            .has_open_close = true,
            .consume_open = false,
            .open_symbol = symbol_operator(TOP_LPAREN),
            .close_symbol = symbol_operator(TOP_RPAREN),
        };

        if (argument_matcher_init(&A, start_index + 1)) {
            do {
                if (is_word) {
                    argument_matcher_consume(&A, symbol_word);
                } else {
                    argument_matcher_reduce(&A, REDUCE_TYPE_EXPRESSION);
                }
            } while (argument_matcher_next(&A));
        }
        size_t size = argument_matcher_close(&A);

        if (elements_look(E, A.pos, symbol_predef_word(TPW_as))) {
            if (argument_matcher_init(&A, A.pos + 1)) {
                do {
                    argument_matcher_reduce(&A, REDUCE_TYPE_EXPRESSION);
                } while (argument_matcher_next(&A));
            }
            size_t as_size = argument_matcher_close(&A);

            if (size != as_size) {
                elements_error(E, 0, "keys for decomposition aren't defined correctly");
            }
        }

        if (end_index != NULL) {
            *end_index = A.pos;
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
    struct elements *E,
    bool is_word,
    size_t start_index,
    strtable_index_t *names,
    struct expression **expressions,
    struct expression **keys
) {
    if (elements_look(E, start_index, symbol_predef_word(TPW_decompose))) {
        struct argument_matcher A = {
            .assemble = true,
            .E = E,
            .U = U,
            .separator = symbol_operator(TOP_COMMA),
            .has_open_close = true,
            .consume_open = false,
            .open_symbol = symbol_operator(TOP_LPAREN),
            .close_symbol = symbol_operator(TOP_RPAREN),
        };

        if (argument_matcher_init(&A, start_index + 1)) {
            do {
                if (is_word) {
                    struct token token = argument_matcher_consume(&A, symbol_word);
                    names[A.count] = token.word;
                } else {
                    struct ast_node *node = argument_matcher_reduce(&A, REDUCE_TYPE_EXPRESSION).node;
                    expressions[A.count] = ast_node_as_expression(U, node);
                }
            } while (argument_matcher_next(&A));
        }
        size_t size = argument_matcher_close(&A);

        if (elements_look(E, A.pos, symbol_predef_word(TPW_as))) {
            if (argument_matcher_init(&A, A.pos + 1)) {
                do {
                    struct ast_node *node = argument_matcher_reduce(&A, REDUCE_TYPE_EXPRESSION).node;
                    keys[A.count] = ast_node_as_expression(U, node);
                } while (argument_matcher_next(&A));
            }
            size_t as_size = argument_matcher_close(&A);

            if (size != as_size) {
                elements_error(E, 0, "keys for decomposition aren't defined correctly");
            }
        }
    } else if (is_word) {
        *names = elements_get_token(E, start_index).word;
    } else {
        struct ast_node *node = elements_get_reduce(E, start_index).node;
        *expressions = ast_node_as_expression(U, node);
    }
}
