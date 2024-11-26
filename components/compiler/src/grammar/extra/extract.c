//
// Created by why-iskra on 12.08.2024.
//

#include "extract.h"
#include "arguments.h"

size_t extra_consume_extract(struct parse_controller *C, bool is_word, bool simple) {
    if (simple && !parser_look(C, et_operator(LBRACE))) {
        if (is_word) {
            parser_consume(C, et_word());
        } else {
            parser_reduce(C, rule_expression);
        }

        return 0;
    }

    struct arguments A = extra_arguments_init_full(
        C, true, false,
        et_operator(LBRACE),
        et_operator(RBRACE),
        et_operator(COMMA)
    );

    while (extra_arguments_next(C, &A)) {
        if (is_word) {
            parser_consume(C, et_word());

            if (parser_match(C, et_predef_word(as))) {
                parser_reduce(C, rule_expression);
            }
        } else {
            parser_reduce(C, rule_expression);
            parser_consume(C, et_predef_word(as));
            parser_reduce(C, rule_expression);
        }
    }

    return extra_arguments_finish(C, &A);
}

void extra_get_extract(
    struct parse_controller *C,
    bool simple,
    bool ignore_mutable,
    struct mc_ast_expression_variable **variables,
    struct mc_ast_expression **expressions,
    struct mc_ast_expression **keys
) {
    ml_line line = parser_get_line(C);
    if (simple && !parser_look(C, et_operator(LBRACE))) {
        if (variables != NULL) {
            ml_size token_from = parser_index(C);
            struct mc_lex_token token = parser_consume(C, et_word());
            ml_size token_to = parser_index(C);
            struct mc_ast_expression_variable *variable = mcapi_ast_create_expression_variable(
                parser_U(C), parser_A(C), token_from, token_to, token.line
            );

            variable->ignore_mutable = ignore_mutable;
            variable->index = token.word;

            *variables = variable;
        } else {
            *expressions = mcapi_ast_node2expression(
                parser_U(C), parser_reduce(C, rule_expression)
            );
        }

        return;
    }

    struct arguments A = extra_arguments_init_full(
        C, true, false,
        et_operator(LBRACE),
        et_operator(RBRACE),
        et_operator(COMMA)
    );

    for (size_t i = 0; extra_arguments_next(C, &A); i++) {
        if (variables != NULL) {
            ml_size token_from = parser_index(C);
            struct mc_lex_token token = parser_consume(C, et_word());
            ml_size token_to = parser_index(C);
            struct mc_ast_expression_variable *variable = mcapi_ast_create_expression_variable(
                parser_U(C), parser_A(C), token_from, token_to, token.line
            );

            variable->ignore_mutable = ignore_mutable;
            variable->index = token.word;

            mc_strtable_index_t string = token.word;
            variables[i] = variable;

            if (parser_match(C, et_predef_word(as))) {
                keys[i] = mcapi_ast_node2expression(
                    parser_U(C), parser_reduce(C, rule_expression)
                );
            } else {
                struct mc_ast_expression_value *value = mcapi_ast_create_expression_value(
                    parser_U(C), parser_A(C), token_from, token_to, line
                );

                value->type = MCEXPR_VALUE_TYPE_STR;
                value->value.string = string;

                keys[i] = mcapi_ast_value2expression(value);
            }
        } else {
            expressions[i] = mcapi_ast_node2expression(
                parser_U(C), parser_reduce(C, rule_expression)
            );

            parser_consume(C, et_predef_word(as));

            keys[i] = mcapi_ast_node2expression(
                parser_U(C), parser_reduce(C, rule_expression)
            );
        }
    }

    extra_arguments_finish(C, &A);
}
