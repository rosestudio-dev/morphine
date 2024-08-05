//
// Created by why-iskra on 06.08.2024.
//

#include "controller.h"

struct mc_ast_node *rule_multiplicative(struct parse_controller *C) {
    {
        parser_reduce(C, rule_prefix);

        while (true) {
            bool matched = parser_match(C, et_operator(STAR)) ||
                           parser_match(C, et_operator(SLASH)) ||
                           parser_match(C, et_operator(PERCENT));

            if (matched) {
                parser_reduce(C, rule_prefix);
            } else {
                break;
            }
        }
    }

    parser_reset(C);

    struct mc_ast_expression *expression =
        mcapi_ast_node2expression(parser_U(C), parser_reduce(C, rule_prefix));

    while (true) {
        ml_line line = parser_get_line(C);

        enum mc_expression_binary_type type;
        if (parser_match(C, et_operator(STAR))) {
            type = MCEXPR_BINARY_TYPE_MUL;
        } else if (parser_match(C, et_operator(SLASH))) {
            type = MCEXPR_BINARY_TYPE_DIV;
        } else if (parser_match(C, et_operator(PERCENT))) {
            type = MCEXPR_BINARY_TYPE_MOD;
        } else {
            break;
        }

        struct mc_ast_expression *expressionB =
            mcapi_ast_node2expression(parser_U(C), parser_reduce(C, rule_prefix));

        struct mc_ast_expression_binary *binary =
            mcapi_ast_create_expression_binary(parser_U(C), parser_A(C), line);

        binary->type = type;
        binary->a = expression;
        binary->b = expressionB;

        expression = mcapi_ast_binary2expression(binary);
    }

    return mcapi_ast_expression2node(expression);
}
