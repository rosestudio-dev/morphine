//
// Created by why-iskra on 05.08.2024.
//

#include "../controller.h"
#include "../extra/arguments.h"

struct mc_ast_node *rule_table(struct parse_controller *C) {
    size_t count;
    ml_size token_from = parser_index(C);
    {
        struct arguments A = extra_arguments_init_full(
            C, true, true, et_operator(LBRACE), et_operator(RBRACE), et_operator(COMMA)
        );

        while (extra_arguments_next(C, &A)) {
            bool is_word = parser_match(C, et_word());
            if (!is_word) {
                if (parser_look(C, et_predef_word(fun))) {
                    parser_reduce(C, rule_function);
                    if (parser_match(C, et_operator(EQ))) {
                        parser_reduce(C, rule_expression);
                    }
                } else {
                    parser_reduce(C, rule_expression);
                    parser_consume(C, et_operator(EQ));
                    parser_reduce(C, rule_expression);
                }
            } else if (parser_match(C, et_operator(EQ))) {
                parser_reduce(C, rule_expression);
            }
        }

        count = extra_arguments_finish(C, &A);
    }
    ml_size token_to = parser_index(C);

    parser_reset(C);

    ml_line line = parser_get_line(C);

    struct mc_ast_expression_table *table =
        mcapi_ast_create_expression_table(parser_U(C), parser_A(C), token_from, token_to, line, count);

    struct arguments A = extra_arguments_init_full(
        C, true, true, et_operator(LBRACE), et_operator(RBRACE), et_operator(COMMA)
    );

    for (size_t i = 0; extra_arguments_next(C, &A); i++) {
        ml_line intermediate_line = parser_get_line(C);

        struct mc_ast_expression *key;
        struct mc_ast_expression *value;

        bool is_word = parser_look(C, et_word());
        if (!is_word) {
            ml_line fun_line = parser_get_line(C);
            if (parser_look(C, et_predef_word(fun))) {
                value = mcapi_ast_node2expression(parser_U(C), parser_reduce(C, rule_function));
                if (parser_match(C, et_operator(EQ))) {
                    key = value;
                    value = mcapi_ast_node2expression(parser_U(C), parser_reduce(C, rule_expression));
                } else {
                    struct mc_ast_expression_function *function =
                        mcapi_ast_expression2function(parser_U(C), value);

                    if (function->ref->anonymous) {
                        parser_errorf(C, "unable to set anonymous function");
                    }

                    struct mc_ast_expression_value *expr_value = mcapi_ast_create_expression_value(
                        parser_U(C), parser_A(C),
                        function->header.node.from,
                        function->header.node.to,
                        fun_line
                    );

                    expr_value->type = MCEXPR_VALUE_TYPE_STR;
                    expr_value->value.string = function->ref->name;

                    key = mcapi_ast_value2expression(expr_value);
                }
            } else {
                key = mcapi_ast_node2expression(parser_U(C), parser_reduce(C, rule_expression));
                parser_consume(C, et_operator(EQ));
                value = mcapi_ast_node2expression(parser_U(C), parser_reduce(C, rule_expression));
            }
        } else {
            ml_size word_token_from = parser_index(C);
            struct mc_lex_token token = parser_consume(C, et_word());
            ml_size word_token_to = parser_index(C);

            struct mc_ast_expression_value *expr_value = mcapi_ast_create_expression_value(
                parser_U(C), parser_A(C), word_token_from, word_token_to, token.line
            );

            expr_value->type = MCEXPR_VALUE_TYPE_STR;
            expr_value->value.string = token.word;

            key = mcapi_ast_value2expression(expr_value);

            if (parser_match(C, et_operator(EQ))) {
                value = mcapi_ast_node2expression(parser_U(C), parser_reduce(C, rule_expression));
            } else {
                struct mc_ast_expression_variable *variable_value =
                    mcapi_ast_create_expression_variable(
                        parser_U(C), parser_A(C), word_token_from, word_token_to, intermediate_line
                    );

                variable_value->index = token.word;
                variable_value->ignore_mutable = false;

                value = mcapi_ast_variable2expression(variable_value);
            }
        }

        table->keys[i] = key;
        table->values[i] = value;
    }

    extra_arguments_finish(C, &A);

    return mcapi_ast_expression_table2node(table);
}
