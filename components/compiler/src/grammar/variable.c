//
// Created by why-iskra on 31.05.2024.
//

#include "impl.h"
#include "support/arguments.h"

static void function_arguments(struct matcher *M) {
    if (matcher_look(M, symbol_operator(MCLTOP_LBRACE)) || matcher_is_reduced(M, REDUCE_TYPE_TABLE)) {
        matcher_reduce(M, REDUCE_TYPE_TABLE);
        return;
    }

    struct argument_matcher R = {
        .assemble = false,
        .M = M,
        .separator = symbol_operator(MCLTOP_COMMA),
        .has_open_close = true,
        .consume_open = true,
        .open_symbol = symbol_operator(MCLTOP_LPAREN),
        .close_symbol = symbol_operator(MCLTOP_RPAREN),
    };

    if (argument_matcher_init(&R, 0)) {
        do {
            matcher_reduce(M, REDUCE_TYPE_EXPRESSION);
        } while (argument_matcher_next(&R));
    }
    argument_matcher_close(&R);

    if (matcher_look(M, symbol_operator(MCLTOP_LBRACE)) || matcher_is_reduced(M, REDUCE_TYPE_TABLE)) {
        matcher_reduce(M, REDUCE_TYPE_TABLE);
    }
}

static size_t function_arguments_count(
    morphine_coroutine_t U,
    struct elements *E,
    size_t start
) {
    if (!elements_is_token(E, start)) {
        return 1;
    }

    struct argument_matcher R = {
        .assemble = true,
        .E = E,
        .U = U,
        .separator = symbol_operator(MCLTOP_COMMA),
        .has_open_close = true,
        .consume_open = true,
        .open_symbol = symbol_operator(MCLTOP_LPAREN),
        .close_symbol = symbol_operator(MCLTOP_RPAREN),
    };

    if (argument_matcher_init(&R, start)) {
        do {
            argument_matcher_reduce(&R, REDUCE_TYPE_EXPRESSION);
        } while (argument_matcher_next(&R));
    }
    size_t size = argument_matcher_close(&R);

    if (R.pos != elements_size(E)) {
        return size + 1;
    }

    return size;
}

static void function_arguments_insert(
    morphine_coroutine_t U,
    struct elements *E,
    size_t start,
    struct expression **expressions
) {
    if (!elements_is_token(E, start)) {
        struct reduce reduce_arg = elements_get_reduce(E, start);
        expressions[0] = ast_node_as_expression(U, reduce_arg.node);
        return;
    }

    struct argument_matcher R = {
        .assemble = true,
        .E = E,
        .U = U,
        .separator = symbol_operator(MCLTOP_COMMA),
        .has_open_close = true,
        .consume_open = true,
        .open_symbol = symbol_operator(MCLTOP_LPAREN),
        .close_symbol = symbol_operator(MCLTOP_RPAREN),
    };

    if (argument_matcher_init(&R, start)) {
        do {
            struct reduce arg_reduce = argument_matcher_reduce(&R, REDUCE_TYPE_EXPRESSION);
            expressions[R.count] = ast_node_as_expression(U, arg_reduce.node);
        } while (argument_matcher_next(&R));
    }
    argument_matcher_close(&R);

    if (R.pos != elements_size(E)) {
        struct reduce arg_reduce = elements_get_reduce(E, R.pos);
        expressions[R.count] = ast_node_as_expression(U, arg_reduce.node);
    }
}

bool match_variable(struct matcher *M, bool is_wrapped) {
    if (!is_wrapped) {
        matcher_reduce(M, REDUCE_TYPE_VALUE);
    }

    if (matcher_match(M, symbol_operator(MCLTOP_LBRACKET))) {
        matcher_reduce(M, REDUCE_TYPE_EXPRESSION);
        matcher_consume(M, symbol_operator(MCLTOP_RBRACKET));
        return true;
    }

    if (matcher_match(M, symbol_operator(MCLTOP_DOT))) {
        matcher_consume(M, symbol_word);
        return true;
    }

    if (matcher_look(M, symbol_operator(MCLTOP_LPAREN)) ||
        matcher_look(M, symbol_operator(MCLTOP_LBRACE)) ||
        matcher_is_reduced(M, REDUCE_TYPE_TABLE)) {
        function_arguments(M);
        return true;
    }

    if (matcher_match(M, symbol_operator(MCLTOP_COLON)) || matcher_match(M, symbol_operator(MCLTOP_RARROW))) {
        if (!matcher_match(M, symbol_word)) {
            matcher_consume(M, symbol_operator(MCLTOP_LBRACKET));
            matcher_reduce(M, REDUCE_TYPE_EXPRESSION);
            matcher_consume(M, symbol_operator(MCLTOP_RBRACKET));
        }

        function_arguments(M);
        return true;
    }

    return false;
}

struct ast_node *assemble_variable(morphine_coroutine_t U, struct ast *A, struct elements *E) {
    struct reduce reduce = elements_get_reduce(E, 0);
    if (elements_size(E) == 1) {
        return ast_as_node(ast_node_as_expression(U, reduce.node));
    }

    if (!elements_is_token(E, 1)) {
        struct reduce reduce_arg = elements_get_reduce(E, 1);

        struct expression_call *result = ast_create_expression_call(U, A, ast_node_line(reduce_arg.node), 1);
        result->expression = ast_node_as_expression(U, reduce.node);
        result->arguments[0] = ast_node_as_expression(U, reduce_arg.node);

        return ast_as_node(result);
    }

    struct mc_lex_token watch_token = elements_get_token(E, 1);

    if (matcher_symbol(symbol_operator(MCLTOP_LBRACKET), watch_token)) {
        struct reduce reduce_key = elements_get_reduce(E, 2);

        struct expression_access *result = ast_create_expression_access(U, A, watch_token.line);
        result->container = ast_node_as_expression(U, reduce.node);
        result->key = ast_node_as_expression(U, reduce_key.node);

        return ast_as_node(result);
    }

    if (matcher_symbol(symbol_operator(MCLTOP_DOT), watch_token)) {
        struct expression_value *key = ast_create_expression_value(U, A, watch_token.line);
        key->type = EXPRESSION_VALUE_TYPE_STR;
        key->value.string = elements_get_token(E, 2).word;

        struct expression_access *result = ast_create_expression_access(U, A, watch_token.line);
        result->container = ast_node_as_expression(U, reduce.node);
        result->key = ast_as_expression(key);

        return ast_as_node(result);
    }

    if (matcher_symbol(symbol_operator(MCLTOP_LPAREN), watch_token)) {
        size_t count = function_arguments_count(U, E, 1);

        struct expression_call *result = ast_create_expression_call(U, A, watch_token.line, count);
        result->expression = ast_node_as_expression(U, reduce.node);
        function_arguments_insert(U, E, 1, result->arguments);

        return ast_as_node(result);
    }

    if (matcher_symbol(symbol_operator(MCLTOP_COLON), watch_token) ||
        matcher_symbol(symbol_operator(MCLTOP_RARROW), watch_token)) {

        struct mc_lex_token access_token = elements_get_token(E, 2);
        struct ast_node *access_node;
        size_t start_index;
        if (matcher_symbol(symbol_operator(MCLTOP_LBRACKET), access_token)) {
            access_node = elements_get_reduce(E, 3).node;
            start_index = 5;
        } else if (matcher_symbol(symbol_operator(MCLTOP_RARROW), watch_token) && matcher_symbol(symbol_word, access_token)) {
            struct expression_variable *variable = ast_create_expression_variable(U, A, access_token.line);
            access_node = ast_as_node(variable);
            variable->index = access_token.word;
            start_index = 3;
        } else if (matcher_symbol(symbol_word, access_token)) {
            struct expression_value *value = ast_create_expression_value(U, A, access_token.line);
            access_node = ast_as_node(value);
            value->type = EXPRESSION_VALUE_TYPE_STR;
            value->value.string = access_token.word;
            start_index = 3;
        } else {
            return NULL;
        }

        size_t count = function_arguments_count(U, E, start_index);

        struct expression_call_self *result = ast_create_expression_call_self(U, A, watch_token.line, count);
        result->self = ast_node_as_expression(U, reduce.node);
        result->callable = ast_node_as_expression(U, access_node);
        result->extract_callable = matcher_symbol(symbol_operator(MCLTOP_COLON), watch_token);
        function_arguments_insert(U, E, start_index, result->arguments);

        return ast_as_node(result);
    }

    return NULL;
}
