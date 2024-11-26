//
// Created by why-iskra on 08.08.2024.
//

#include "../controller.h"
#include "../extra/arguments.h"

struct mc_ast_node *rule_when(struct parse_controller *C) {
    size_t count;
    bool has_else = false;
    ml_size token_from = parser_index(C);
    {
        parser_consume(C, et_predef_word(when));

        if (parser_match(C, et_operator(LPAREN))) {
            parser_reduce(C, rule_expression);
            parser_consume(C, et_operator(RPAREN));
        }

        struct arguments A = extra_arguments_init_full(
            C, true, true, et_operator(LBRACE), et_operator(RBRACE), et_operator(COMMA)
        );

        while (extra_arguments_next(C, &A)) {
            if (parser_match(C, et_predef_word(else))) {
                if (has_else) {
                    parser_errorf(C, "when already has else condition");
                }

                has_else = true;
            } else {
                parser_reduce(C, rule_expression);
            }

            parser_consume(C, et_operator(RARROW));
            parser_reduce(C, rule_statement);
        }

        count = extra_arguments_finish(C, &A);
    }
    ml_size token_to = parser_index(C);

    parser_reset(C);

    if (has_else) {
        count--;
    }

    ml_line line = parser_get_line(C);
    struct mc_ast_expression_when *when =
        mcapi_ast_create_expression_when(parser_U(C), parser_A(C), token_from, token_to, line, count);

    parser_consume(C, et_predef_word(when));

    if (parser_match(C, et_operator(LPAREN))) {
        when->expression = mcapi_ast_node2expression(parser_U(C), parser_reduce(C, rule_expression));
        parser_consume(C, et_operator(RPAREN));
    } else {
        when->expression = NULL;
    }

    struct arguments A = extra_arguments_init_full(
        C, true, true, et_operator(LBRACE), et_operator(RBRACE), et_operator(COMMA)
    );

    size_t index = 0;
    while (extra_arguments_next(C, &A)) {
        if (parser_match(C, et_predef_word(else))) {
            parser_consume(C, et_operator(RARROW));
            when->else_statement = mcapi_ast_node2statement(parser_U(C), parser_reduce(C, rule_statement));
        } else {
            when->if_conditions[index] = mcapi_ast_node2expression(
                parser_U(C), parser_reduce(C, rule_expression)
            );
            parser_consume(C, et_operator(RARROW));
            when->if_statements[index] = mcapi_ast_node2statement(
                parser_U(C), parser_reduce(C, rule_statement)
            );

            index++;
        }
    }

    extra_arguments_finish(C, &A);

    return mcapi_ast_expression_when2node(when);
}
