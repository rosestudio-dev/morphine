//
// Created by why-iskra on 05.08.2024.
//

#include "controller.h"

struct mc_ast_node *rule_value(struct parse_controller *C) {
    if (parser_match(C, et_operator(LPAREN))) {
        struct mc_ast_node *node = parser_reduce(C, rule_expression);
        parser_consume(C, et_operator(RPAREN));
        return node;
    }

    return parser_reduce(C, rule_constant);
}
