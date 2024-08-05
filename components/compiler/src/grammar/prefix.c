//
// Created by why-iskra on 06.08.2024.
//

#include "controller.h"

struct mc_ast_node *rule_prefix(struct parse_controller *C) {
    {
        bool matched = parser_match(C, et_operator(MINUS)) ||
                       parser_match(C, et_operator(STAR)) ||
                       parser_match(C, et_predef_word(not)) ||
                       parser_match(C, et_predef_word(typeof)) ||
                       parser_match(C, et_predef_word(lenof)) ||
                       parser_match(C, et_predef_word(ref)) ||
                       parser_match(C, et_operator(PLUSPLUS)) ||
                       parser_match(C, et_operator(MINUSMINUS));

        if (matched) {
            parser_reduce(C, rule_prefix);
        } else {
            parser_reduce(C, rule_postfix);
        }
    }

    parser_reset(C);

    ml_line line = parser_get_line(C);

    if (parser_match(C, et_operator(MINUS))) {
        struct mc_ast_expression *expression =
            mcapi_ast_node2expression(parser_U(C), parser_reduce(C, rule_prefix));

        struct mc_ast_expression_unary *unary =
            mcapi_ast_create_expression_unary(parser_U(C), parser_A(C), line);

        unary->type = MCEXPR_UNARY_TYPE_NEGATE;
        unary->expression = expression;

        return mcapi_ast_expression_unary2node(unary);
    }

    if (parser_match(C, et_operator(STAR))) {
        struct mc_ast_expression *expression =
            mcapi_ast_node2expression(parser_U(C), parser_reduce(C, rule_prefix));

        struct mc_ast_expression_unary *unary =
            mcapi_ast_create_expression_unary(parser_U(C), parser_A(C), line);

        unary->type = MCEXPR_UNARY_TYPE_DEREF;
        unary->expression = expression;

        return mcapi_ast_expression_unary2node(unary);
    }

    if (parser_match(C, et_predef_word(ref))) {
        struct mc_ast_expression *expression =
            mcapi_ast_node2expression(parser_U(C), parser_reduce(C, rule_prefix));

        struct mc_ast_expression_unary *unary =
            mcapi_ast_create_expression_unary(parser_U(C), parser_A(C), line);

        unary->type = MCEXPR_UNARY_TYPE_REF;
        unary->expression = expression;

        return mcapi_ast_expression_unary2node(unary);
    }

    if (parser_match(C, et_predef_word(not))) {
        struct mc_ast_expression *expression =
            mcapi_ast_node2expression(parser_U(C), parser_reduce(C, rule_prefix));

        struct mc_ast_expression_unary *unary =
            mcapi_ast_create_expression_unary(parser_U(C), parser_A(C), line);

        unary->type = MCEXPR_UNARY_TYPE_NOT;
        unary->expression = expression;

        return mcapi_ast_expression_unary2node(unary);
    }

    if (parser_match(C, et_predef_word(typeof))) {
        struct mc_ast_expression *expression =
            mcapi_ast_node2expression(parser_U(C), parser_reduce(C, rule_prefix));

        struct mc_ast_expression_unary *unary =
            mcapi_ast_create_expression_unary(parser_U(C), parser_A(C), line);

        unary->type = MCEXPR_UNARY_TYPE_TYPE;
        unary->expression = expression;

        return mcapi_ast_expression_unary2node(unary);
    }

    if (parser_match(C, et_predef_word(lenof))) {
        struct mc_ast_expression *expression =
            mcapi_ast_node2expression(parser_U(C), parser_reduce(C, rule_prefix));

        struct mc_ast_expression_unary *unary =
            mcapi_ast_create_expression_unary(parser_U(C), parser_A(C), line);

        unary->type = MCEXPR_UNARY_TYPE_LEN;
        unary->expression = expression;

        return mcapi_ast_expression_unary2node(unary);
    }

    if (parser_match(C, et_operator(PLUSPLUS))) {
        struct mc_ast_expression *expression =
            mcapi_ast_node2expression(parser_U(C), parser_reduce(C, rule_prefix));

        struct mc_ast_expression_increment *increment =
            mcapi_ast_create_expression_increment(parser_U(C), parser_A(C), line);

        increment->is_postfix = false;
        increment->is_decrement = false;
        increment->expression = expression;

        return mcapi_ast_expression_increment2node(increment);
    }

    if (parser_match(C, et_operator(MINUSMINUS))) {
        struct mc_ast_expression *expression =
            mcapi_ast_node2expression(parser_U(C), parser_reduce(C, rule_prefix));

        struct mc_ast_expression_increment *increment =
            mcapi_ast_create_expression_increment(parser_U(C), parser_A(C), line);

        increment->is_postfix = false;
        increment->is_decrement = true;
        increment->expression = expression;

        return mcapi_ast_expression_increment2node(increment);
    }

    return parser_reduce(C, rule_postfix);
}
