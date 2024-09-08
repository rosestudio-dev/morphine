//
// Created by why-iskra on 08.08.2024.
//

#include "controller.h"

static struct mc_ast_node *rule_statement(struct parse_controller *C) {
    ml_line line = parser_get_line(C);
    if (parser_match(C, et_predef_word(pass))) {
        struct mc_ast_statement_pass *pass =
            mcapi_ast_create_statement_pass(parser_U(C), parser_A(C), line);

        return mcapi_ast_statement_pass2node(pass);
    }

    if (parser_match(C, et_predef_word(yield))) {
        struct mc_ast_statement_yield *yield =
            mcapi_ast_create_statement_yield(parser_U(C), parser_A(C), line);

        return mcapi_ast_statement_yield2node(yield);
    }

    if (parser_match(C, et_predef_word(eval))) {
        struct mc_ast_expression *expression =
            mcapi_ast_node2expression(parser_U(C), parser_reduce(C, rule_expression));

        struct mc_ast_statement_eval *eval =
            mcapi_ast_create_statement_eval(parser_U(C), parser_A(C), line);

        eval->implicit = false;
        eval->expression = expression;

        return mcapi_ast_statement_eval2node(eval);
    }

    if (parser_look(C, et_predef_word(while))) {
        return parser_reduce(C, rule_while);
    }

    if (parser_look(C, et_predef_word(do))) {
        return parser_reduce(C, rule_dowhile);
    }

    if (parser_look(C, et_predef_word(for))) {
        return parser_reduce(C, rule_for);
    }

    if (parser_look(C, et_predef_word(iterator))) {
        return parser_reduce(C, rule_iterator);
    }

    if (parser_look(C, et_predef_word(if))) {
        return parser_reduce(C, rule_statement_if);
    }

    if (parser_look(C, et_predef_word(val)) ||
        parser_look(C, et_predef_word(var)) ||
        parser_look(C, et_predef_word(fun))) {
        return parser_reduce(C, rule_declaration);
    }

    return parser_reduce(C, rule_assigment);
}

struct mc_ast_node *rule_statement_explicit_without_semicolon(struct parse_controller *C) {
    struct mc_ast_node *node = parser_reduce(C, rule_statement);
    struct mc_ast_statement *statement = mcapi_ast_node2statement(parser_U(C), node);

    if (statement->type == MCSTMTT_eval) {
        struct mc_ast_statement_eval *eval = mcapi_ast_statement2eval(parser_U(C), statement);

        if (eval->implicit) {
            parser_error(C, "implicit statement");
        }
    }

    return node;
}

struct mc_ast_node *rule_statement_explicit(struct parse_controller *C) {
    struct mc_ast_node *node = rule_statement_explicit_without_semicolon(C);
    parser_match(C, et_operator(SEMICOLON));
    return node;
}

struct mc_ast_node *rule_statement_implicit(struct parse_controller *C) {
    struct mc_ast_node *node = parser_reduce(C, rule_statement);
    parser_match(C, et_operator(SEMICOLON));
    mcapi_ast_node2statement(parser_U(C), node);
    return node;
}
