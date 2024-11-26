//
// Created by why-iskra on 05.08.2024.
//

#include "../controller.h"

struct mc_ast_node *rule_postfix(struct parse_controller *C) {
    ml_size token_from = parser_index(C);
    {
        parser_reduce(C, rule_primary);

        bool matched;
        do {
            matched = parser_match(C, et_operator(PLUSPLUS)) ||
                      parser_match(C, et_operator(MINUSMINUS));
        } while (matched);
    }

    parser_reset(C);

    struct mc_ast_expression *expression =
        mcapi_ast_node2expression(parser_U(C), parser_reduce(C, rule_primary));

    while (true) {
        ml_line line = parser_get_line(C);

        bool is_decrement;
        if (parser_match(C, et_operator(PLUSPLUS))) {
            is_decrement = false;
        } else if (parser_match(C, et_operator(MINUSMINUS))) {
            is_decrement = true;
        } else {
            break;
        }

        ml_size token_to = parser_index(C);
        struct mc_ast_expression_increment *increment =
            mcapi_ast_create_expression_increment(parser_U(C), parser_A(C), token_from, token_to, line);

        increment->is_postfix = true;
        increment->is_decrement = is_decrement;
        increment->expression = expression;

        expression = mcapi_ast_increment2expression(increment);
    }

    return mcapi_ast_expression2node(expression);
}
