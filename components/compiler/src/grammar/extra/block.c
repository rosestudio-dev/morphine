//
// Created by why-iskra on 06.08.2024.
//

#include "block.h"

static size_t consume_block(
    struct parse_controller *C,
    size_t closes_size,
    struct expected_token *closes,
    size_t *close_index,
    parse_function_t function,
    struct mc_ast_node **last
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

        struct mc_ast_node *node = parser_reduce(C, function);
        if (last != NULL) {
            *last = node;
        }

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
        C, closes_size, closes, close_index, rule_statement_explicit, NULL
    );
}

size_t extra_consume_expression_block(
    struct parse_controller *C,
    size_t closes_size,
    struct expected_token *closes,
    size_t *close_index,
    bool safe
) {
    struct mc_ast_node *last = NULL;
    size_t size = consume_block(
        C, closes_size, closes, close_index, rule_statement_implicit, &last
    );

    if (safe) {
        if (last == NULL) {
            return 0;
        }

        struct mc_ast_statement *statement = mcapi_ast_node2statement(parser_U(C), last);

        if (statement->type == MCSTMTT_eval) {
            struct mc_ast_statement_eval *eval = mcapi_ast_statement2eval(parser_U(C), statement);

            if (eval->implicit) {
                return size - 1;
            }
        }

        return size;
    }

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
    struct mc_ast_expression **expression,
    bool safe
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

    if (safe) {
        ml_line line = parser_get_line(C);
        bool insert_nil = false;
        for (size_t i = 0; i < closes_size; i++) {
            if (parser_match(C, closes[i])) {
                insert_nil = true;
                break;
            }
        }

        if (insert_nil) {
            struct mc_ast_expression_value *value =
                mcapi_ast_create_expression_value(parser_U(C), parser_A(C), line);

            value->type = MCEXPR_VALUE_TYPE_NIL;

            *expression = mcapi_ast_value2expression(value);
            return;
        }
    }

    ml_line line = parser_get_line(C);
    struct mc_ast_statement *statement =
        mcapi_ast_node2statement(parser_U(C), parser_reduce(C, rule_statement_implicit));

    if (statement->type == MCSTMTT_eval) {
        struct mc_ast_statement_eval *eval = mcapi_ast_statement2eval(parser_U(C), statement);
        *expression = eval->expression;
    } else if (safe) {
        struct mc_ast_expression_value *value =
            mcapi_ast_create_expression_value(parser_U(C), parser_A(C), line);

        value->type = MCEXPR_VALUE_TYPE_NIL;

        *expression = mcapi_ast_value2expression(value);
    } else {
        parser_errorf(C, "expression block must contain eval expression at the end");
    }

    for (size_t i = 0; i < closes_size; i++) {
        if (parser_match(C, closes[i])) {
            return;
        }
    }

    parser_errorf(C, "unclosed block");
}
