//
// Created by why-iskra on 05.08.2024.
//

#include "controller.h"

struct mc_ast_node *rule_primary(struct parse_controller *C) {
    ml_line line = parser_get_line(C);

    if (parser_look(C, et_operator(LBRACE))) {
        return parser_reduce(C, rule_table);
    } else if (parser_look(C, et_operator(LBRACKET))) {
        return parser_reduce(C, rule_vector);
    } else if (parser_look(C, et_predef_word(fun))) {
        return parser_reduce(C, rule_function);
    } else if (parser_look(C, et_predef_word(if))) {
        return parser_reduce(C, rule_expression_if);
    } else if (parser_match(C, et_predef_word(do))) {
        return parser_reduce(C, rule_expression_block);
    } else if (parser_match(C, et_predef_word(leave))) {
        struct mc_ast_expression_leave *leave =
            mcapi_ast_create_expression_leave(parser_U(C), parser_A(C), line);

        leave->expression = NULL;

        return mcapi_ast_expression_leave2node(leave);
    }  else if (parser_match(C, et_predef_word(return))) {
        struct mc_ast_expression *expression =
            mcapi_ast_node2expression(parser_U(C), parser_reduce(C, rule_expression));

        struct mc_ast_expression_leave *leave =
            mcapi_ast_create_expression_leave(parser_U(C), parser_A(C), line);

        leave->expression = expression;

        return mcapi_ast_expression_leave2node(leave);
    } else {
        return parser_reduce(C, rule_variable);
    }
}
