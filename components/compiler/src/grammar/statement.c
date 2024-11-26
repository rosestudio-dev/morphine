//
// Created by why-iskra on 08.08.2024.
//

#include "controller.h"

struct mc_ast_node *rule_statement(struct parse_controller *C) {
    ml_line line = parser_get_line(C);
    ml_size token_from = parser_index(C);

    if (parser_match(C, et_predef_word(pass))) {
        ml_size token_to = parser_index(C);
        struct mc_ast_statement_pass *pass =
            mcapi_ast_create_statement_pass(parser_U(C), parser_A(C), token_from, token_to, line);

        return mcapi_ast_statement_pass2node(pass);
    }

    if (parser_match(C, et_predef_word(yield))) {
        ml_size token_to = parser_index(C);
        struct mc_ast_statement_yield *yield =
            mcapi_ast_create_statement_yield(parser_U(C), parser_A(C), token_from, token_to, line);

        return mcapi_ast_statement_yield2node(yield);
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

    if (parser_look(C, et_predef_word(val)) ||
        parser_look(C, et_predef_word(var)) ||
        parser_look(C, et_predef_word(fun))) {
        return parser_reduce(C, rule_declaration);
    }

    return parser_reduce(C, rule_assigment);
}
