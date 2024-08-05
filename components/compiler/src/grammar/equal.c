//
// Created by why-iskra on 06.08.2024.
//

#include "controller.h"

struct mc_ast_node *rule_equal(struct parse_controller *C) {
    {
        parser_reduce(C, rule_condition);

        while (true) {
            bool matched = parser_match(C, et_operator(EQEQ)) ||
                           parser_match(C, et_operator(EXCLEQ));

            if (matched) {
                parser_reduce(C, rule_condition);
            } else {
                break;
            }
        }
    }

    parser_reset(C);

    struct mc_ast_expression *expression =
        mcapi_ast_node2expression(parser_U(C), parser_reduce(C, rule_condition));

    while (true) {
        ml_line line = parser_get_line(C);

        if (parser_match(C, et_operator(EQEQ))) {
            struct mc_ast_expression *expressionB =
                mcapi_ast_node2expression(parser_U(C), parser_reduce(C, rule_condition));

            struct mc_ast_expression_binary *binary =
                mcapi_ast_create_expression_binary(parser_U(C), parser_A(C), line);

            binary->type = MCEXPR_BINARY_TYPE_EQUAL;
            binary->a = expression;
            binary->b = expressionB;

            expression = mcapi_ast_binary2expression(binary);
        } else if (parser_match(C, et_operator(EXCLEQ))) {
            struct mc_ast_expression *expressionB =
                mcapi_ast_node2expression(parser_U(C), parser_reduce(C, rule_condition));

            struct mc_ast_expression_binary *binary =
                mcapi_ast_create_expression_binary(parser_U(C), parser_A(C), line);

            binary->type = MCEXPR_BINARY_TYPE_EQUAL;
            binary->a = expression;
            binary->b = expressionB;

            struct mc_ast_expression_unary *unary =
                mcapi_ast_create_expression_unary(parser_U(C), parser_A(C), line);

            unary->type = MCEXPR_UNARY_TYPE_NOT;
            unary->expression = mcapi_ast_binary2expression(binary);

            expression = mcapi_ast_unary2expression(unary);
        } else {
            break;
        }
    }

    return mcapi_ast_expression2node(expression);
}
