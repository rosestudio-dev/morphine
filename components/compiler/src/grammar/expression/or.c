//
// Created by why-iskra on 06.08.2024.
//

#include "../controller.h"

struct mc_ast_node *rule_or(struct parse_controller *C) {
    ml_size token_from = parser_index(C);
    {
        parser_reduce(C, rule_and);

        while (true) {
            bool matched = parser_match(C, et_predef_word(or));

            if (matched) {
                parser_reduce(C, rule_and);
            } else {
                break;
            }
        }
    }

    parser_reset(C);

    struct mc_ast_expression *expression =
        mcapi_ast_node2expression(parser_U(C), parser_reduce(C, rule_and));

    while (true) {
        ml_line line = parser_get_line(C);

        if (!parser_match(C, et_predef_word(or))) {
            break;
        }

        ml_size token_to = parser_index(C);
        struct mc_ast_expression *expressionB =
            mcapi_ast_node2expression(parser_U(C), parser_reduce(C, rule_and));

        struct mc_ast_expression_binary *binary =
            mcapi_ast_create_expression_binary(parser_U(C), parser_A(C), token_from, token_to, line);

        binary->type = MCEXPR_BINARY_TYPE_OR;
        binary->a = expression;
        binary->b = expressionB;

        expression = mcapi_ast_binary2expression(binary);
    }

    return mcapi_ast_expression2node(expression);
}
