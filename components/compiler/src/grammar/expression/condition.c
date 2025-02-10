//
// Created by why-iskra on 06.08.2024.
//

#include "../controller.h"

struct mc_ast_node *rule_condition(struct parse_controller *C) {
    ml_size token_from = parser_index(C);
    {
        parser_reduce(C, rule_concat);

        while (true) {
            bool matched = parser_match(C, et_operator(LT)) || parser_match(C, et_operator(GT))
                           || parser_match(C, et_operator(LTEQ)) || parser_match(C, et_operator(GTEQ));

            if (matched) {
                parser_reduce(C, rule_concat);
            } else {
                break;
            }
        }
    }

    parser_reset(C);

    struct mc_ast_expression *expression = mcapi_ast_node2expression(parser_U(C), parser_reduce(C, rule_concat));

    while (true) {
        ml_line line = parser_get_line(C);

        enum mc_expression_binary_type type;
        if (parser_match(C, et_operator(LT))) {
            type = MCEXPR_BINARY_TYPE_LT;
        } else if (parser_match(C, et_operator(GT))) {
            type = MCEXPR_BINARY_TYPE_GT;
        } else if (parser_match(C, et_operator(LTEQ))) {
            type = MCEXPR_BINARY_TYPE_LE;
        } else if (parser_match(C, et_operator(GTEQ))) {
            type = MCEXPR_BINARY_TYPE_GE;
        } else {
            break;
        }

        ml_size token_to = parser_index(C);
        struct mc_ast_expression *expressionB = mcapi_ast_node2expression(parser_U(C), parser_reduce(C, rule_concat));

        struct mc_ast_expression_binary *binary =
            mcapi_ast_create_expression_binary(parser_U(C), parser_A(C), token_from, token_to, line);

        binary->type = type;
        binary->a = expression;
        binary->b = expressionB;

        expression = mcapi_ast_binary2expression(binary);
    }

    return mcapi_ast_expression2node(expression);
}
