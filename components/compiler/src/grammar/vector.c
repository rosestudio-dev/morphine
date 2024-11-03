//
// Created by why-iskra on 05.08.2024.
//

#include "controller.h"
#include "extra/arguments.h"

struct mc_ast_node *rule_vector(struct parse_controller *C) {
    size_t count;
    {
        struct arguments A = extra_arguments_init_full(
            C, true, true, et_operator(LBRACKET), et_operator(RBRACKET), et_operator(COMMA)
        );

        while (extra_arguments_next(C, &A)) {
            parser_reduce(C, rule_expression);
        }

        count = extra_arguments_finish(C, &A);
    }

    parser_reset(C);

    ml_line line = parser_get_line(C);

    struct mc_ast_expression_vector *vector =
        mcapi_ast_create_expression_vector(parser_U(C), parser_A(C), line, count);

    struct arguments A = extra_arguments_init_full(
        C, true, true, et_operator(LBRACKET), et_operator(RBRACKET), et_operator(COMMA)
    );

    for (size_t i = 0; extra_arguments_next(C, &A); i++) {
        struct mc_ast_expression *expression =
            mcapi_ast_node2expression(parser_U(C), parser_reduce(C, rule_expression));

        vector->expressions[i] = expression;
    }

    extra_arguments_finish(C, &A);

    return mcapi_ast_expression_vector2node(vector);
}
