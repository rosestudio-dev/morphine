//
// Created by why-iskra on 31.05.2024.
//

#include "impl.h"

void match_prefix(struct matcher *M) {
    bool matched = matcher_match(M, symbol_operator(MCLTOP_MINUS)) ||
                   matcher_match(M, symbol_operator(MCLTOP_STAR)) ||
                   matcher_match(M, symbol_predef_word(MCLTPW_not)) ||
                   matcher_match(M, symbol_predef_word(MCLTPW_typeof)) ||
                   matcher_match(M, symbol_predef_word(MCLTPW_lenof)) ||
                   matcher_match(M, symbol_predef_word(MCLTPW_ref)) ||
                   matcher_match(M, symbol_operator(MCLTOP_PLUSPLUS)) ||
                   matcher_match(M, symbol_operator(MCLTOP_MINUSMINUS));

    if (matched) {
        matcher_reduce(M, REDUCE_TYPE_PREFIX);
    } else {
        matcher_reduce(M, REDUCE_TYPE_POSTFIX);
    }
}

struct ast_node *assemble_prefix(morphine_coroutine_t U, struct ast *A, struct elements *E) {
    if (elements_size(E) == 1) {
        struct reduce reduce = elements_get_reduce(E, 0);
        return ast_as_node(ast_node_as_expression(U, reduce.node));
    }

    struct mc_lex_token watch_token = elements_get_token(E, 0);
    struct reduce reduce = elements_get_reduce(E, 1);

    if (matcher_symbol(symbol_operator(MCLTOP_PLUSPLUS), watch_token) ||
        matcher_symbol(symbol_operator(MCLTOP_MINUSMINUS), watch_token)) {
        struct expression_increment *result = ast_create_expression_increment(U, A, watch_token.line);
        result->is_postfix = false;
        result->is_decrement = matcher_symbol(symbol_operator(MCLTOP_MINUSMINUS), watch_token);
        result->container = ast_node_as_expression(U, reduce.node);

        return ast_as_node(result);
    }

    struct expression_unary *result = ast_create_expression_unary(U, A, watch_token.line);
    result->expression = ast_node_as_expression(U, reduce.node);

    if (matcher_symbol(symbol_operator(MCLTOP_MINUS), watch_token)) {
        result->type = EXPRESSION_UNARY_TYPE_NEGATE;
    } else if (matcher_symbol(symbol_operator(MCLTOP_STAR), watch_token)) {
        result->type = EXPRESSION_UNARY_TYPE_DEREF;
    } else if (matcher_symbol(symbol_predef_word(MCLTPW_not), watch_token)) {
        result->type = EXPRESSION_UNARY_TYPE_NOT;
    } else if (matcher_symbol(symbol_predef_word(MCLTPW_lenof), watch_token)) {
        result->type = EXPRESSION_UNARY_TYPE_LEN;
    } else if (matcher_symbol(symbol_predef_word(MCLTPW_typeof), watch_token)) {
        result->type = EXPRESSION_UNARY_TYPE_TYPE;
    } else if (matcher_symbol(symbol_predef_word(MCLTPW_ref), watch_token)) {
        result->type = EXPRESSION_UNARY_TYPE_REF;
    } else {
        return NULL;
    }

    return ast_as_node(result);
}