//
// Created by why-iskra on 31.05.2024.
//

#include "impl.h"

void match_value(struct matcher *M) {
    if (matcher_match(M, symbol_int)) { return; }
    if (matcher_match(M, symbol_dec)) { return; }
    if (matcher_match(M, symbol_str)) { return; }
    if (matcher_match(M, symbol_word)) { return; }
    if (matcher_match(M, symbol_predef_word(MCLTPW_nil))) { return; }
    if (matcher_match(M, symbol_predef_word(MCLTPW_true))) { return; }
    if (matcher_match(M, symbol_predef_word(MCLTPW_false))) { return; }
    if (matcher_match(M, symbol_predef_word(MCLTPW_env))) { return; }
    if (matcher_match(M, symbol_predef_word(MCLTPW_self))) { return; }

    if (matcher_match(M, symbol_operator(MCLTOP_LPAREN))) {
        matcher_reduce(M, REDUCE_TYPE_EXPRESSION);
        matcher_consume(M, symbol_operator(MCLTOP_RPAREN));
        return;
    }

    matcher_error(M, "unexpected token");
}

struct ast_node *assemble_value(morphine_coroutine_t U, struct ast *A, struct elements *E) {
    if (!elements_is_token(E, 0)) {
        return elements_get_reduce(E, 0).node;
    }

    struct mc_lex_token watch_token = elements_get_token(E, 0);

    if (matcher_symbol(symbol_int, watch_token)) {
        struct expression_value *result = ast_create_expression_value(U, A, watch_token.line);
        result->type = EXPRESSION_VALUE_TYPE_INT;
        result->value.integer = watch_token.integer;

        return ast_as_node(result);
    }

    if (matcher_symbol(symbol_dec, watch_token)) {
        struct expression_value *result = ast_create_expression_value(U, A, watch_token.line);
        result->type = EXPRESSION_VALUE_TYPE_DEC;
        result->value.decimal = watch_token.decimal;

        return ast_as_node(result);
    }

    if (matcher_symbol(symbol_str, watch_token)) {
        struct expression_value *result = ast_create_expression_value(U, A, watch_token.line);
        result->type = EXPRESSION_VALUE_TYPE_STR;
        result->value.string = watch_token.string;

        return ast_as_node(result);
    }

    if (matcher_symbol(symbol_word, watch_token)) {
        struct expression_variable *result = ast_create_expression_variable(U, A, watch_token.line);
        result->index = watch_token.word;

        return ast_as_node(result);
    }

    if (matcher_symbol(symbol_predef_word(MCLTPW_nil), watch_token)) {
        struct expression_value *result = ast_create_expression_value(U, A, watch_token.line);
        result->type = EXPRESSION_VALUE_TYPE_NIL;

        return ast_as_node(result);
    }

    if (matcher_symbol(symbol_predef_word(MCLTPW_true), watch_token)) {
        struct expression_value *result = ast_create_expression_value(U, A, watch_token.line);
        result->type = EXPRESSION_VALUE_TYPE_BOOL;
        result->value.boolean = true;

        return ast_as_node(result);
    }

    if (matcher_symbol(symbol_predef_word(MCLTPW_false), watch_token)) {
        struct expression_value *result = ast_create_expression_value(U, A, watch_token.line);
        result->type = EXPRESSION_VALUE_TYPE_BOOL;
        result->value.boolean = false;

        return ast_as_node(result);
    }

    if (matcher_symbol(symbol_predef_word(MCLTPW_env), watch_token)) {
        struct expression_global *result = ast_create_expression_global(U, A, watch_token.line);
        result->type = EXPRESSION_GLOBAL_TYPE_ENV;

        return ast_as_node(result);
    }

    if (matcher_symbol(symbol_predef_word(MCLTPW_self), watch_token)) {
        struct expression_global *result = ast_create_expression_global(U, A, watch_token.line);
        result->type = EXPRESSION_GLOBAL_TYPE_SELF;

        return ast_as_node(result);
    }

    if (matcher_symbol(symbol_operator(MCLTOP_LPAREN), watch_token)) {
        struct reduce reduce = elements_get_reduce(E, 1);
        return ast_as_node(ast_node_as_expression(U, reduce.node));
    }

    return NULL;
}