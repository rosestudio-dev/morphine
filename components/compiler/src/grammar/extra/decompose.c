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
    ml_line line = parser_get_line(C);
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

    for (size_t i = 0; extra_arguments_next(C, &A); i++) {
        mc_strtable_index_t string;
        if (is_word) {
            struct mc_lex_token token = parser_consume(C, et_word());
            struct mc_ast_expression_variable *variable =
                mcapi_ast_create_expression_variable(parser_U(C), parser_A(C), token.line);

            variable->ignore_mutable = true;
            variable->index = token.word;

            string = token.word;
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
        } else {
            struct mc_ast_expression_value *value =
                mcapi_ast_create_expression_value(parser_U(C), parser_A(C), line);

            if (is_word) {
                value->type = MCEXPR_VALUE_TYPE_STR;
                value->value.string = string;
            } else {
                value->type = MCEXPR_VALUE_TYPE_INT;
                value->value.integer = mapi_csize2size(parser_U(C), i, "index");
            }

            keys[i] = mcapi_ast_value2expression(value);
        }
    }

    extra_arguments_finish(C, &A);
}
