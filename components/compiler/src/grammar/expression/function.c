//
// Created by why-iskra on 06.08.2024.
//

#include "../controller.h"
#include "../extra/arguments.h"

struct mc_ast_node *rule_function(struct parse_controller *C) {
    size_t args;
    size_t closures = 0;
    ml_size token_from = parser_index(C);
    {
        parser_consume(C, et_predef_word(fun));
        parser_match(C, et_predef_word(recursive));
        parser_match(C, et_word());

        if (parser_look(C, et_operator(LT))) {
            struct arguments A = extra_arguments_init_full(
                C, true, true, et_operator(LT), et_operator(GT), et_operator(COMMA)
            );

            if (!parser_match(C, et_predef_word(auto))) {
                while (extra_arguments_next(C, &A)) {
                    parser_consume(C, et_word());
                }
            }

            closures = extra_arguments_finish(C, &A);
        }

        {
            struct arguments A = extra_arguments_init_full(
                C, true, true, et_operator(LPAREN), et_operator(RPAREN), et_operator(COMMA)
            );

            while (extra_arguments_next(C, &A)) {
                parser_consume(C, et_word());
            }

            args = extra_arguments_finish(C, &A);
        }

        if (parser_match(C, et_operator(EQ))) {
            parser_reduce(C, rule_expression);
        } else {
            parser_reduce(C, rule_statement_block);
        }
    }
    ml_size token_to = parser_index(C);

    parser_reset(C);

    struct mc_ast_function *function =
        mcapi_ast_create_function(parser_U(C), parser_A(C), closures, args);

    ml_line line = parser_get_line(C);
    function->line = line;

    parser_consume(C, et_predef_word(fun));
    function->recursive = parser_match(C, et_predef_word(recursive));
    if (parser_look(C, et_word())) {
        function->name = parser_consume(C, et_word()).word;
        function->anonymous = false;
    } else {
        function->anonymous = true;
    }

    if (parser_look(C, et_operator(LT))) {
        struct arguments A = extra_arguments_init_full(
            C, true, true, et_operator(LT), et_operator(GT), et_operator(COMMA)
        );

        if (parser_match(C, et_predef_word(auto))) {
            function->auto_closure = true;
        } else {
            function->auto_closure = false;
            for (size_t i = 0; extra_arguments_next(C, &A); i++) {
                function->closures[i] = parser_consume(C, et_word()).word;
            }
        }

        extra_arguments_finish(C, &A);
    }

    {
        struct arguments A = extra_arguments_init_full(
            C, true, true, et_operator(LPAREN), et_operator(RPAREN), et_operator(COMMA)
        );

        for (size_t i = 0; extra_arguments_next(C, &A); i++) {
            function->arguments[i] = parser_consume(C, et_word()).word;
        }

        extra_arguments_finish(C, &A);
    }

    struct mc_ast_statement *statement;
    ml_line eq_line = parser_get_line(C);
    ml_size eq_token_from = parser_index(C);
    if (parser_match(C, et_operator(EQ))) {
        struct mc_ast_expression_leave *leave =
            mcapi_ast_create_expression_leave(parser_U(C), parser_A(C), eq_token_from, token_to, eq_line);

        leave->expression = mcapi_ast_node2expression(parser_U(C), parser_reduce(C, rule_expression));

        struct mc_ast_statement_eval *eval =
            mcapi_ast_create_statement_eval(parser_U(C), parser_A(C), eq_token_from, token_to, eq_line);

        eval->expression = mcapi_ast_leave2expression(leave);

        statement = mcapi_ast_eval2statement(eval);
    } else {
        statement = mcapi_ast_node2statement(parser_U(C), parser_reduce(C, rule_statement_block));
    }

    function->body = statement;

    struct mc_ast_expression_function *function_expr =
        mcapi_ast_create_expression_function(parser_U(C), parser_A(C), token_from, token_to, line);

    function_expr->ref = function;

    return mcapi_ast_expression_function2node(function_expr);
}
