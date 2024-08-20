//
// Created by why-iskra on 06.08.2024.
//

#include "controller.h"

struct mc_ast_node *rule_condition(struct parse_controller *C) {
    {
        parser_reduce(C, rule_concat);

        while (true) {
            bool matched = parser_match(C, et_operator(LT)) ||
                           parser_match(C, et_operator(GT)) ||
                           parser_match(C, et_operator(LTEQ)) ||
                           parser_match(C, et_operator(GTEQ));

            if (matched) {
                parser_reduce(C, rule_concat);
            } else {
                break;
            }
        }
    }

    parser_reset(C);

    struct mc_ast_expression *expression =
        mcapi_ast_node2expression(parser_U(C), parser_reduce(C, rule_concat));

    while (true) {
        ml_line line = parser_get_line(C);

        if (parser_look(C, et_operator(LT)) || parser_look(C, et_operator(GT))) {
            bool swap = parser_match(C, et_operator(GT));
            if (!swap) {
                parser_consume(C, et_operator(LT));
            }

            struct mc_ast_expression *expressionB =
                mcapi_ast_node2expression(parser_U(C), parser_reduce(C, rule_concat));

            struct mc_ast_expression_binary *binary =
                mcapi_ast_create_expression_binary(parser_U(C), parser_A(C), line);

            binary->type = MCEXPR_BINARY_TYPE_LESS;

            if (swap) {
                binary->a = expressionB;
                binary->b = expression;
            } else {
                binary->a = expression;
                binary->b = expressionB;
            }

            expression = mcapi_ast_binary2expression(binary);
        } else if (parser_look(C, et_operator(LTEQ)) || parser_look(C, et_operator(GTEQ))) {
            bool swap = parser_match(C, et_operator(GTEQ));
            if (!swap) {
                parser_consume(C, et_operator(LTEQ));
            }

            struct mc_ast_expression *expressionB =
                mcapi_ast_node2expression(parser_U(C), parser_reduce(C, rule_concat));

            struct mc_ast_expression_binary *binary_cond =
                mcapi_ast_create_expression_binary(parser_U(C), parser_A(C), line);

            binary_cond->type = MCEXPR_BINARY_TYPE_LESS;

            if (swap) {
                binary_cond->a = expressionB;
                binary_cond->b = expression;
            } else {
                binary_cond->a = expression;
                binary_cond->b = expressionB;
            }

            struct mc_ast_expression_binary *binary_eq =
                mcapi_ast_create_expression_binary(parser_U(C), parser_A(C), line);

            binary_eq->type = MCEXPR_BINARY_TYPE_EQUAL;
            binary_eq->a = expression;
            binary_eq->b = expressionB;

            struct mc_ast_expression_binary *binary =
                mcapi_ast_create_expression_binary(parser_U(C), parser_A(C), line);

            binary->type = MCEXPR_BINARY_TYPE_OR;
            binary->a = mcapi_ast_binary2expression(binary_cond);
            binary->b = mcapi_ast_binary2expression(binary_eq);

            expression = mcapi_ast_binary2expression(binary);
        } else {
            break;
        }
    }

    return mcapi_ast_expression2node(expression);
}
