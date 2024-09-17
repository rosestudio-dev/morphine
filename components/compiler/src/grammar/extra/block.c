//
// Created by why-iskra on 06.08.2024.
//

#include "block.h"

static size_t consume_block(
    struct parse_controller *C,
    size_t closes_size,
    struct expected_token *closes,
    size_t *close_index,
    parse_function_t function
) {
    size_t size = 0;
    while (true) {
        for (size_t i = 0; i < closes_size; i++) {
            if (parser_match(C, closes[i])) {
                if (close_index != NULL) {
                    *close_index = i;
                }

                return size;
            }
        }

        parser_reduce(C, function);
        size++;
    }
}

size_t extra_consume_statement_block(
    struct parse_controller *C,
    size_t closes_size,
    struct expected_token *closes,
    size_t *close_index
) {
    return consume_block(
        C, closes_size, closes, close_index, rule_statement_explicit
    );
}

size_t extra_consume_expression_block(
    struct parse_controller *C,
    size_t closes_size,
    struct expected_token *closes,
    size_t *close_index
) {
    size_t size = consume_block(
        C, closes_size, closes, close_index, rule_statement_implicit
    );

    if (size == 0) {
        parser_errorf(C, "empty expression block");
    }

    return size - 1;
}

void extra_extract_statement_block(
    struct parse_controller *C,
    size_t closes_size,
    struct expected_token *closes,
    size_t size,
    struct mc_ast_statement **statements
) {
    for (size_t i = 0; i < size; i++) {
        struct mc_ast_node *node = parser_reduce(C, rule_statement_explicit);
        statements[i] = mcapi_ast_node2statement(parser_U(C), node);
    }

    for (size_t i = 0; i < closes_size; i++) {
        if (parser_match(C, closes[i])) {
            return;
        }
    }

    parser_errorf(C, "unclosed block");
}

void extra_extract_expression_block(
    struct parse_controller *C,
    size_t closes_size,
    struct expected_token *closes,
    size_t size,
    struct mc_ast_statement **statements,
    struct mc_ast_expression **expression
) {
    for (size_t i = 0; i < size; i++) {
        struct mc_ast_node *node = parser_reduce(C, rule_statement_implicit);
        struct mc_ast_statement *statement = mcapi_ast_node2statement(parser_U(C), node);

        if (statement->type == MCSTMTT_eval) {
            struct mc_ast_statement_eval *eval = mcapi_ast_statement2eval(parser_U(C), statement);

            if (eval->implicit) {
                parser_errorf(C, "implicit expression");
            }
        }

        statements[i] = statement;
    }

    struct mc_ast_statement *statement =
        mcapi_ast_node2statement(parser_U(C), parser_reduce(C, rule_statement_implicit));

    if (statement->type != MCSTMTT_eval) {
        parser_errorf(C, "expression block must contain eval expression at the end");
    }

    struct mc_ast_statement_eval *eval = mcapi_ast_statement2eval(parser_U(C), statement);
    *expression = eval->expression;

    for (size_t i = 0; i < closes_size; i++) {
        if (parser_match(C, closes[i])) {
            return;
        }
    }

    parser_errorf(C, "unclosed block");
}
