//
// Created by why-iskra on 13.08.2024.
//

#include "../controller.h"

struct mc_ast_node *rule_declaration(struct parse_controller *C) {
    ml_size token_from = parser_index(C);
    {
        if (parser_look(C, et_predef_word(fun))) {
            parser_reduce(C, rule_function);
        } else {
            if (!parser_match(C, et_predef_word(val)) && !parser_match(C, et_predef_word(var))) {
                parser_errorf(C, "expect val, var or fun");
            }

            parser_consume(C, et_word());
            parser_consume(C, et_operator(EQ));
            parser_reduce(C, rule_expression);
        }
    }
    ml_size token_to = parser_index(C);

    parser_reset(C);

    ml_line line = parser_get_line(C);

    struct mc_ast_statement_declaration *declaration =
        mcapi_ast_create_statement_declaration(parser_U(C), parser_A(C), token_from, token_to, line);

    if (parser_look(C, et_predef_word(fun))) {
        struct mc_ast_expression_function *function =
            mcapi_ast_node2function_expression(parser_U(C), parser_reduce(C, rule_function));

        if (function->ref->anonymous) {
            parser_errorf(C, "unable to declare anonymous function");
        }

        struct mc_ast_expression_variable *variable =
            mcapi_ast_create_expression_variable(parser_U(C), parser_A(C), token_from, token_to, line);

        variable->ignore_mutable = true;
        variable->index = function->ref->name;

        declaration->mutable = false;
        declaration->variable = variable;
        declaration->expression = mcapi_ast_function2expression(function);
    } else {
        bool is_var = parser_match(C, et_predef_word(var));
        if (!is_var) {
            parser_consume(C, et_predef_word(val));
        }
        declaration->mutable = is_var;

        struct mc_lex_token token = parser_consume(C, et_word());
        struct mc_ast_expression_variable *variable =
            mcapi_ast_create_expression_variable(parser_U(C), parser_A(C), token_from, token_to, line);

        variable->ignore_mutable = true;
        variable->index = token.word;

        parser_consume(C, et_operator(EQ));
        declaration->variable = variable;
        declaration->expression = mcapi_ast_node2expression(parser_U(C), parser_reduce(C, rule_expression));
    }

    return mcapi_ast_statement_declaration2node(declaration);
}
