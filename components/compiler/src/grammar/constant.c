//
// Created by why-iskra on 05.08.2024.
//

#include "controller.h"

struct mc_ast_node *rule_constant(struct parse_controller *C) {
    ml_line line = parser_get_line(C);

    if (parser_look(C, et_integer())) {
        struct mc_lex_token token = parser_consume(C, et_integer());
        struct mc_ast_expression_value *value =
            mcapi_ast_create_expression_value(parser_U(C), parser_A(C), line);

        value->type = MCEXPR_VALUE_TYPE_INT;
        value->value.integer = token.integer;

        return mcapi_ast_expression_value2node(value);
    }

    if (parser_look(C, et_decimal())) {
        struct mc_lex_token token = parser_consume(C, et_decimal());
        struct mc_ast_expression_value *value =
            mcapi_ast_create_expression_value(parser_U(C), parser_A(C), line);

        value->type = MCEXPR_VALUE_TYPE_DEC;
        value->value.decimal = token.decimal;

        return mcapi_ast_expression_value2node(value);
    }

    if (parser_look(C, et_string())) {
        struct mc_lex_token token = parser_consume(C, et_string());
        struct mc_ast_expression_value *value =
            mcapi_ast_create_expression_value(parser_U(C), parser_A(C), line);

        value->type = MCEXPR_VALUE_TYPE_STR;
        value->value.string = token.string;

        return mcapi_ast_expression_value2node(value);
    }

    if (parser_look(C, et_word())) {
        struct mc_lex_token token = parser_consume(C, et_word());
        struct mc_ast_expression_variable *variable =
            mcapi_ast_create_expression_variable(parser_U(C), parser_A(C), line);

        variable->index = token.word;
        variable->ignore_mutable = false;

        return mcapi_ast_expression_variable2node(variable);
    }

    if (parser_match(C, et_predef_word(nil))) {
        struct mc_ast_expression_value *value =
            mcapi_ast_create_expression_value(parser_U(C), parser_A(C), line);

        value->type = MCEXPR_VALUE_TYPE_NIL;

        return mcapi_ast_expression_value2node(value);
    }

    if (parser_match(C, et_predef_word(true))) {
        struct mc_ast_expression_value *value =
            mcapi_ast_create_expression_value(parser_U(C), parser_A(C), line);

        value->type = MCEXPR_VALUE_TYPE_BOOL;
        value->value.boolean = true;

        return mcapi_ast_expression_value2node(value);
    }

    if (parser_match(C, et_predef_word(false))) {
        struct mc_ast_expression_value *value =
            mcapi_ast_create_expression_value(parser_U(C), parser_A(C), line);

        value->type = MCEXPR_VALUE_TYPE_BOOL;
        value->value.boolean = false;

        return mcapi_ast_expression_value2node(value);
    }

    if (parser_match(C, et_predef_word(env))) {
        struct mc_ast_expression_global *global =
            mcapi_ast_create_expression_global(parser_U(C), parser_A(C), line);

        global->type = MCEXPR_GLOBAL_TYPE_ENV;

        return mcapi_ast_expression_global2node(global);
    }

    if (parser_match(C, et_predef_word(self))) {
        struct mc_ast_expression_global *global =
            mcapi_ast_create_expression_global(parser_U(C), parser_A(C), line);

        global->type = MCEXPR_GLOBAL_TYPE_SELF;

        return mcapi_ast_expression_global2node(global);
    }

    if (parser_match(C, et_predef_word(invoked))) {
        struct mc_ast_expression_global *global =
            mcapi_ast_create_expression_global(parser_U(C), parser_A(C), line);

        global->type = MCEXPR_GLOBAL_TYPE_INVOKED;

        return mcapi_ast_expression_global2node(global);
    }

    parser_error(C, "unexpected token");
}
