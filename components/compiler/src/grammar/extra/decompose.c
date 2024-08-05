//
// Created by why-iskra on 12.08.2024.
//

#include "decompose.h"
#include "arguments.h"

size_t extra_consume_decompose(struct parse_controller *C, bool is_word) {
    if (!parser_match(C, et_predef_word(decompose))) {
        if (is_word) {
            parser_consume(C, et_word());
        } else {
            parser_reduce(C, rule_expression);
        }

        return 0;
    }

    struct arguments A = extra_arguments_init_simple(C, et_operator(COMMA));

    while (extra_arguments_next(C, &A)) {
        if (is_word) {
            parser_consume(C, et_word());
        } else {
            parser_reduce(C, rule_expression);
        }

        if (parser_match(C, et_predef_word(as))) {
            parser_reduce(C, rule_expression);
        }
    }

    return extra_arguments_finish(C, &A);
}

void extra_extract_decompose(
    struct parse_controller *C,
    bool is_word,
    struct mc_ast_expression_variable **variables,
    struct mc_ast_expression **expressions,
    struct mc_ast_expression **keys
) {
    if (!parser_match(C, et_predef_word(decompose))) {
        if (is_word) {
            struct mc_lex_token token = parser_consume(C, et_word());
            struct mc_ast_expression_variable *variable =
                mcapi_ast_create_expression_variable(parser_U(C), parser_A(C), token.line);

            variable->ignore_mutable = true;
            variable->index = token.word;

            *variables = variable;
        } else {
            *expressions = mcapi_ast_node2expression(
                parser_U(C), parser_reduce(C, rule_expression)
            );
        }

        return;
    }

    struct arguments A = extra_arguments_init_simple(C, et_operator(COMMA));

    for (size_t i = 0; extra_arguments_next(C, &A); i ++) {
        if (is_word) {
            struct mc_lex_token token = parser_consume(C, et_word());
            struct mc_ast_expression_variable *variable =
                mcapi_ast_create_expression_variable(parser_U(C), parser_A(C), token.line);

            variable->ignore_mutable = true;
            variable->index = token.word;

            variables[i] = variable;
        } else {
            expressions[i] = mcapi_ast_node2expression(
                parser_U(C), parser_reduce(C, rule_expression)
            );
        }

        if (parser_match(C, et_predef_word(as))) {
            keys[i] = mcapi_ast_node2expression(
                parser_U(C), parser_reduce(C, rule_expression)
            );
        }
    }

    extra_arguments_finish(C, &A);
}
