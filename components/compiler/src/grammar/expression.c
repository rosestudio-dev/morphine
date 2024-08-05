//
// Created by why-iskra on 05.08.2024.
//

#include "controller.h"

struct mc_ast_node *rule_expression(struct parse_controller *C) {
    return parser_reduce(C, rule_or);
}
