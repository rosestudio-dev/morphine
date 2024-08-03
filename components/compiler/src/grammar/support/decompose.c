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
                } else if (!matcher_match(M, symbol_word)) {
                    matcher_reduce(M, REDUCE_TYPE_EXPRESSION);
                }

                if (matcher_match(M, symbol_predef_word(TPW_as))) {
                    matcher_reduce(M, REDUCE_TYPE_EXPRESSION);
                }
            } while (argument_matcher_next(&R));
        }
        size_t size = argument_matcher_close(&R);

        if (size == 0) {
            matcher_error(M, "empty decomposition");
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
                } else if (!argument_matcher_match(&R, symbol_word)) {
                    argument_matcher_reduce(&R, REDUCE_TYPE_EXPRESSION);
                }

                if (argument_matcher_match(&R, symbol_predef_word(TPW_as))) {
                    argument_matcher_reduce(&R, REDUCE_TYPE_EXPRESSION);
                }
            } while (argument_matcher_next(&R));
        }
        size_t size = argument_matcher_close(&R);

        if (size == 0) {
            elements_error(E, 0, "empty decomposition");
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
    morphinec_strtable_index_t *names,
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
                struct token token;
                bool required_key = true;
                if (is_word) {
                    required_key = false;
                    token = argument_matcher_consume(&R, symbol_word);

                    names[R.count] = token.word;
                } else if (argument_matcher_look(&R, symbol_word)) {
                    required_key = false;
                    token = argument_matcher_consume(&R, symbol_word);
                    struct expression_variable *variable = ast_create_expression_variable(U, A, token.line);
                    variable->index = token.word;

                    expressions[R.count] = ast_as_expression(variable);
                } else {
                    struct ast_node *node = argument_matcher_reduce(&R, REDUCE_TYPE_EXPRESSION).node;
                    expressions[R.count] = ast_node_as_expression(U, node);
                }

                bool parse_key;
                if (required_key) {
                    argument_matcher_consume(&R, symbol_predef_word(TPW_as));
                    parse_key = true;
                } else {
                    parse_key = argument_matcher_match(&R, symbol_predef_word(TPW_as));
                }

                if (parse_key) {
                    struct ast_node *node = argument_matcher_reduce(&R, REDUCE_TYPE_EXPRESSION).node;
                    keys[R.count] = ast_node_as_expression(U, node);
                } else {
                    struct expression_value *value = ast_create_expression_value(
                        U, A, elements_line(E, start_index)
                    );

                    value->type = EXPRESSION_VALUE_TYPE_STR;
                    value->value.string = token.word;

                    keys[R.count] = ast_as_expression(value);
                }
            } while (argument_matcher_next(&R));
        }
        argument_matcher_close(&R);
    } else if (is_word) {
        *names = elements_get_token(E, start_index).word;
    } else {
        struct ast_node *node = elements_get_reduce(E, start_index).node;
        *expressions = ast_node_as_expression(U, node);
    }
}
