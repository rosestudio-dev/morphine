//
// Created by why-iskra on 05.08.2024.
//

#include "../controller.h"

struct mc_ast_node *rule_constant(struct parse_controller *C) {
    ml_line line = parser_get_line(C);
    ml_size token_from = parser_index(C);

    if (parser_look(C, et_integer())) {
        struct mc_lex_token token = parser_consume(C, et_integer());
        ml_size token_to = parser_index(C);
        struct mc_ast_expression_value *value =
            mcapi_ast_create_expression_value(parser_U(C), parser_A(C), token_from, token_to, line);

        value->type = MCEXPR_VALUE_TYPE_INT;
        value->value.integer = token.integer;

        return mcapi_ast_expression_value2node(value);
    }

    if (parser_look(C, et_decimal())) {
        struct mc_lex_token token = parser_consume(C, et_decimal());
        ml_size token_to = parser_index(C);
        struct mc_ast_expression_value *value =
            mcapi_ast_create_expression_value(parser_U(C), parser_A(C), token_from, token_to, line);

        value->type = MCEXPR_VALUE_TYPE_DEC;
        value->value.decimal = token.decimal;

        return mcapi_ast_expression_value2node(value);
    }

    if (parser_look(C, et_string())) {
        struct mc_lex_token token = parser_consume(C, et_string());
        ml_size token_to = parser_index(C);
        struct mc_ast_expression_value *value =
            mcapi_ast_create_expression_value(parser_U(C), parser_A(C), token_from, token_to, line);

        value->type = MCEXPR_VALUE_TYPE_STR;
        value->value.string = token.string;

        return mcapi_ast_expression_value2node(value);
    }

    if (parser_look(C, et_word())) {
        struct mc_lex_token token = parser_consume(C, et_word());
        ml_size token_to = parser_index(C);
        struct mc_ast_expression_variable *variable =
            mcapi_ast_create_expression_variable(parser_U(C), parser_A(C), token_from, token_to, line);

        variable->index = token.word;
        variable->ignore_mutable = false;

        return mcapi_ast_expression_variable2node(variable);
    }

    if (parser_match(C, et_predef_word(nil))) {
        ml_size token_to = parser_index(C);
        struct mc_ast_expression_value *value =
            mcapi_ast_create_expression_value(parser_U(C), parser_A(C), token_from, token_to, line);

        value->type = MCEXPR_VALUE_TYPE_NIL;

        return mcapi_ast_expression_value2node(value);
    }

    if (parser_match(C, et_predef_word(true))) {
        ml_size token_to = parser_index(C);
        struct mc_ast_expression_value *value =
            mcapi_ast_create_expression_value(parser_U(C), parser_A(C), token_from, token_to, line);

        value->type = MCEXPR_VALUE_TYPE_BOOL;
        value->value.boolean = true;

        return mcapi_ast_expression_value2node(value);
    }

    if (parser_match(C, et_predef_word(false))) {
        ml_size token_to = parser_index(C);
        struct mc_ast_expression_value *value =
            mcapi_ast_create_expression_value(parser_U(C), parser_A(C), token_from, token_to, line);

        value->type = MCEXPR_VALUE_TYPE_BOOL;
        value->value.boolean = false;

        return mcapi_ast_expression_value2node(value);
    }

    if (parser_match(C, et_predef_word(env))) {
        ml_size token_to = parser_index(C);
        struct mc_ast_expression_global *global =
            mcapi_ast_create_expression_global(parser_U(C), parser_A(C), token_from, token_to, line);

        global->type = MCEXPR_GLOBAL_TYPE_ENV;

        return mcapi_ast_expression_global2node(global);
    }

    if (parser_match(C, et_predef_word(self))) {
        ml_size token_to = parser_index(C);
        struct mc_ast_expression_global *global =
            mcapi_ast_create_expression_global(parser_U(C), parser_A(C), token_from, token_to, line);

        global->type = MCEXPR_GLOBAL_TYPE_SELF;

        return mcapi_ast_expression_global2node(global);
    }

    if (parser_match(C, et_predef_word(invoked))) {
        ml_size token_to = parser_index(C);
        struct mc_ast_expression_global *global =
            mcapi_ast_create_expression_global(parser_U(C), parser_A(C), token_from, token_to, line);

        global->type = MCEXPR_GLOBAL_TYPE_INVOKED;

        return mcapi_ast_expression_global2node(global);
    }

    parser_errorf(C, "unexpected token");
}
