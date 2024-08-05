//
// Created by why-iskra on 06.08.2024.
//

#include "controller.h"

struct mc_ast_node *rule_additive(struct parse_controller *C) {
    {
        parser_reduce(C, rule_multiplicative);

        while (true) {
            bool matched = parser_match(C, et_operator(PLUS)) ||
                           parser_match(C, et_operator(MINUS));

            if (matched) {
                parser_reduce(C, rule_multiplicative);
            } else {
                break;
            }
        }
    }

    parser_reset(C);

    struct mc_ast_expression *expression =
        mcapi_ast_node2expression(parser_U(C), parser_reduce(C, rule_multiplicative));

    while (true) {
        ml_line line = parser_get_line(C);

        enum mc_expression_binary_type type;
        if (parser_match(C, et_operator(PLUS))) {
            type = MCEXPR_BINARY_TYPE_ADD;
        } else if (parser_match(C, et_operator(MINUS))) {
            type = MCEXPR_BINARY_TYPE_SUB;
        } else {
            break;
        }

        struct mc_ast_expression *expressionB =
            mcapi_ast_node2expression(parser_U(C), parser_reduce(C, rule_multiplicative));

        struct mc_ast_expression_binary *binary =
            mcapi_ast_create_expression_binary(parser_U(C), parser_A(C), line);

        binary->type = type;
        binary->a = expression;
        binary->b = expressionB;

        expression = mcapi_ast_binary2expression(binary);
    }

    return mcapi_ast_expression2node(expression);
}
