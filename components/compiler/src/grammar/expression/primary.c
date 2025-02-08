//
// Created by why-iskra on 05.08.2024.
//

#include "../controller.h"

struct mc_ast_node *rule_primary(struct parse_controller *C) {
    ml_line line = parser_get_line(C);
    ml_size token_from = parser_index(C);

    if (parser_look(C, et_operator(LBRACE))) {
        return parser_reduce(C, rule_table);
    }

    if (parser_look(C, et_operator(LBRACKET))) {
        return parser_reduce(C, rule_vector);
    }

    if (parser_look(C, et_predef_word(fun))) {
        return parser_reduce(C, rule_function);
    }

    if (parser_look(C, et_predef_word(if))) {
        return parser_reduce(C, rule_if);
    }

    if (parser_match(C, et_predef_word(do))) {
        return parser_reduce(C, rule_expression_block);
    }

    if (parser_match(C, et_predef_word(leave))) {
        ml_size token_to = parser_index(C);
        struct mc_ast_expression_leave *leave =
            mcapi_ast_create_expression_leave(parser_U(C), parser_A(C), token_from, token_to, line);

        leave->expression = NULL;

        return mcapi_ast_expression_leave2node(leave);
    }

    if (parser_match(C, et_predef_word(return))) {
        ml_size token_to = parser_index(C);
        struct mc_ast_expression *expression =
            mcapi_ast_node2expression(parser_U(C), parser_reduce(C, rule_expression));

        struct mc_ast_expression_leave *leave =
            mcapi_ast_create_expression_leave(parser_U(C), parser_A(C), token_from, token_to, line);

        leave->expression = expression;

        return mcapi_ast_expression_leave2node(leave);
    }

    if (parser_match(C, et_predef_word(break))) {
        ml_size token_to = parser_index(C);
        struct mc_ast_expression_break *expr =
            mcapi_ast_create_expression_break(parser_U(C), parser_A(C), token_from, token_to, line);

        return mcapi_ast_expression_break2node(expr);
    }

    if (parser_match(C, et_predef_word(continue))) {
        ml_size token_to = parser_index(C);
        struct mc_ast_expression_continue *expr =
            mcapi_ast_create_expression_continue(parser_U(C), parser_A(C), token_from, token_to, line);

        return mcapi_ast_expression_continue2node(expr);
    }

    return parser_reduce(C, rule_variable);
}
