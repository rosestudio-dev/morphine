//
// Created by why-iskra on 05.08.2024.
//

#include "controller.h"
#include "extra/arguments.h"

static size_t function_arguments(struct parse_controller *C, struct mc_ast_expression **args) {
    if (parser_look(C, et_operator(LBRACE))) {
        struct mc_ast_expression *expression =
            mcapi_ast_node2expression(parser_U(C), parser_reduce(C, rule_table));

        if (args != NULL) {
            args[0] = expression;
        }

        return 1;
    }

    struct arguments A = extra_arguments_init_full(
        C, true, et_operator(LPAREN), et_operator(RPAREN), et_operator(COMMA)
    );

    size_t index = 0;
    for (; extra_arguments_next(C, &A); index++) {
        struct mc_ast_expression *expression =
            mcapi_ast_node2expression(parser_U(C), parser_reduce(C, rule_expression));

        if (args != NULL) {
            args[index] = expression;
        }
    }

    size_t count = extra_arguments_finish(C, &A);

    if (parser_look(C, et_operator(LBRACE))) {
        struct mc_ast_expression *expression =
            mcapi_ast_node2expression(parser_U(C), parser_reduce(C, rule_table));

        if (args != NULL) {
            args[index] = expression;
        }

        return count + 1;
    }

    return count;
}

static struct mc_ast_node *rule_variable_call(struct parse_controller *C) {
    size_t count = function_arguments(C, NULL);

    parser_reset(C);

    ml_line line = parser_get_line(C);

    struct mc_ast_expression_value *self_value =
        mcapi_ast_create_expression_value(parser_U(C), parser_A(C), line);

    self_value->type = MCEXPR_VALUE_TYPE_NIL;

    struct mc_ast_expression_call *call =
        mcapi_ast_create_expression_call(parser_U(C), parser_A(C), line, count);

    function_arguments(C, call->arguments);

    call->extract_callable = false;
    call->self = mcapi_ast_value2expression(self_value);
    call->callable = NULL;

    return mcapi_ast_expression_call2node(call);
}

static struct mc_ast_node *rule_variable_call_self(struct parse_controller *C) {
    size_t count;
    {
        bool extract_callable = parser_match(C, et_operator(COLON));
        if (!extract_callable) {
            parser_consume(C, et_operator(RARROW));
        }

        if (!parser_match(C, et_word())) {
            parser_consume(C, et_operator(LBRACKET));
            parser_reduce(C, rule_expression);
            parser_consume(C, et_operator(RBRACKET));
        }

        count = function_arguments(C, NULL);
    }

    parser_reset(C);

    ml_line line = parser_get_line(C);

    bool extract_callable = parser_match(C, et_operator(COLON));
    if (!extract_callable) {
        parser_consume(C, et_operator(RARROW));
    }

    struct mc_ast_expression *callable;
    if (parser_look(C, et_word())) {
        struct mc_lex_token token = parser_consume(C, et_word());

        if (extract_callable) {
            struct mc_ast_expression_value *value =
                mcapi_ast_create_expression_value(parser_U(C), parser_A(C), token.line);

            value->type = MCEXPR_VALUE_TYPE_STR;
            value->value.string = token.word;

            callable = mcapi_ast_value2expression(value);
        } else {
            struct mc_ast_expression_variable *variable =
                mcapi_ast_create_expression_variable(parser_U(C), parser_A(C), token.line);

            variable->index = token.word;
            variable->ignore_mutable = false;

            callable = mcapi_ast_variable2expression(variable);
        }
    } else {
        parser_consume(C, et_operator(LBRACKET));
        callable = mcapi_ast_node2expression(parser_U(C), parser_reduce(C, rule_expression));
        parser_consume(C, et_operator(RBRACKET));
    }

    struct mc_ast_expression_call *call =
        mcapi_ast_create_expression_call(parser_U(C), parser_A(C), line, count);

    function_arguments(C, call->arguments);

    call->extract_callable = extract_callable;
    call->self = NULL;
    call->callable = callable;

    return mcapi_ast_expression_call2node(call);
}

struct mc_ast_node *rule_variable(struct parse_controller *C) {
    {
        parser_reduce(C, rule_value);

        while (true) {
            if (parser_match(C, et_operator(LBRACKET))) {
                parser_reduce(C, rule_expression);
                parser_consume(C, et_operator(RBRACKET));
            } else if (parser_match(C, et_operator(DOT))) {
                parser_consume(C, et_word());
            } else if (parser_look(C, et_operator(LPAREN)) || parser_look(C, et_operator(LBRACE))) {
                parser_reduce(C, rule_variable_call);
            } else if (parser_look(C, et_operator(COLON)) || parser_look(C, et_operator(RARROW))) {
                parser_reduce(C, rule_variable_call_self);
            } else {
                break;
            }
        }
    }

    parser_reset(C);

    struct mc_ast_expression *expression =
        mcapi_ast_node2expression(parser_U(C), parser_reduce(C, rule_value));

    while (true) {
        ml_line line = parser_get_line(C);

        if (parser_match(C, et_operator(LBRACKET))) {
            struct mc_ast_expression *key =
                mcapi_ast_node2expression(parser_U(C), parser_reduce(C, rule_expression));
            parser_consume(C, et_operator(RBRACKET));

            struct mc_ast_expression_access *access =
                mcapi_ast_create_expression_access(parser_U(C), parser_A(C), line);

            access->container = expression;
            access->key = key;

            expression = mcapi_ast_access2expression(access);
        } else if (parser_match(C, et_operator(DOT))) {
            struct mc_lex_token token = parser_consume(C, et_word());

            struct mc_ast_expression_value *key_value =
                mcapi_ast_create_expression_value(parser_U(C), parser_A(C), token.line);

            key_value->type = MCEXPR_VALUE_TYPE_STR;
            key_value->value.string = token.word;

            struct mc_ast_expression_access *access =
                mcapi_ast_create_expression_access(parser_U(C), parser_A(C), line);

            access->container = expression;
            access->key = mcapi_ast_value2expression(key_value);

            expression = mcapi_ast_access2expression(access);
        } else if (parser_look(C, et_operator(LPAREN)) || parser_look(C, et_operator(LBRACE))) {
            struct mc_ast_expression *next_expression =
                mcapi_ast_node2expression(parser_U(C), parser_reduce(C, rule_variable_call));

            struct mc_ast_expression_call *call =
                mcapi_ast_expression2call(parser_U(C), next_expression);

            call->callable = expression;

            expression = next_expression;
        } else if (parser_look(C, et_operator(COLON)) || parser_look(C, et_operator(RARROW))) {
            struct mc_ast_expression *next_expression =
                mcapi_ast_node2expression(parser_U(C), parser_reduce(C, rule_variable_call_self));

            struct mc_ast_expression_call *call =
                mcapi_ast_expression2call(parser_U(C), next_expression);

            call->self = expression;

            expression = next_expression;
        } else {
            break;
        }
    }

    return mcapi_ast_expression2node(expression);
}
