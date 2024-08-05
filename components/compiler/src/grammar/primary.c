//
// Created by why-iskra on 05.08.2024.
//

#include "controller.h"

struct mc_ast_node *rule_primary(struct parse_controller *C) {
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
    } else {
        return parser_reduce(C, rule_variable);
    }
}
