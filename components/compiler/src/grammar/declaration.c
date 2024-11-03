//
// Created by why-iskra on 13.08.2024.
//

#include "controller.h"
#include "extra/extract.h"

struct mc_ast_node *rule_declaration(struct parse_controller *C) {
    size_t extract_size = 0;
    {
        if (parser_look(C, et_predef_word(fun))) {
            parser_reduce(C, rule_function);
        } else {
            if (!parser_match(C, et_predef_word(val)) &&
                !parser_match(C, et_predef_word(var))) {
                parser_errorf(C, "expect val, var or fun");
            }

            extract_size = extra_consume_extract(C, true, true);

            parser_consume(C, et_operator(EQ));
            parser_reduce(C, rule_expression);
        }
    }

    parser_reset(C);

    ml_line line = parser_get_line(C);

    struct mc_ast_statement_declaration *declaration =
        mcapi_ast_create_statement_declaration(parser_U(C), parser_A(C), line, extract_size);

    declaration->is_extract = extract_size > 0;

    if (parser_look(C, et_predef_word(fun))) {
        struct mc_ast_expression_function *function =
            mcapi_ast_node2function_expression(parser_U(C), parser_reduce(C, rule_function));

        if (function->ref->anonymous) {
            parser_errorf(C, "unable to declare anonymous function");
        }

        struct mc_ast_expression_variable *variable =
            mcapi_ast_create_expression_variable(parser_U(C), parser_A(C), line);

        variable->ignore_mutable = true;

        variable->index = function->ref->name;

        declaration->mutable = false;
        declaration->value = variable;
        declaration->expression = mcapi_ast_function2expression(function);
    } else {
        bool is_var = parser_match(C, et_predef_word(var));
        if (!is_var) {
            parser_consume(C, et_predef_word(val));
        }

        declaration->mutable = is_var;

        if (extract_size > 0) {
            extra_get_extract(
                C,
                true,
                true,
                declaration->extract.values,
                NULL,
                declaration->extract.keys
            );
        } else {
            extra_get_extract(
                C,
                true,
                true,
                &declaration->value,
                NULL,
                NULL
            );
        }

        parser_consume(C, et_operator(EQ));

        declaration->expression =
            mcapi_ast_node2expression(parser_U(C), parser_reduce(C, rule_expression));
    }

    return mcapi_ast_statement_declaration2node(declaration);
}