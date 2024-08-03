//
// Created by why-iskra on 31.05.2024.
//

#include "impl.h"

bool match_binary_or(struct matcher *M, bool is_wrapped) {
    if (!is_wrapped) {
        matcher_reduce(M, REDUCE_TYPE_AND);
    }

    bool matched = matcher_match(M, symbol_predef_word(MCLTPW_or));

    if (matched) {
        matcher_reduce(M, REDUCE_TYPE_AND);
    }

    return matched;
}

bool match_binary_and(struct matcher *M, bool is_wrapped) {
    if (!is_wrapped) {
        matcher_reduce(M, REDUCE_TYPE_EQUAL);
    }

    bool matched = matcher_match(M, symbol_predef_word(MCLTPW_and));

    if (matched) {
        matcher_reduce(M, REDUCE_TYPE_EQUAL);
    }

    return matched;
}

bool match_binary_equal(struct matcher *M, bool is_wrapped) {
    if (!is_wrapped) {
        matcher_reduce(M, REDUCE_TYPE_CONDITION);
    }

    bool matched = matcher_match(M, symbol_operator(MCLTOP_EQEQ)) ||
                   matcher_match(M, symbol_operator(MCLTOP_EXCLEQ));

    if (matched) {
        matcher_reduce(M, REDUCE_TYPE_CONDITION);
    }

    return matched;
}

bool match_binary_condition(struct matcher *M, bool is_wrapped) {
    if (!is_wrapped) {
        matcher_reduce(M, REDUCE_TYPE_CONCAT);
    }

    bool matched = matcher_match(M, symbol_operator(MCLTOP_LT)) ||
                   matcher_match(M, symbol_operator(MCLTOP_GT)) ||
                   matcher_match(M, symbol_operator(MCLTOP_LTEQ)) ||
                   matcher_match(M, symbol_operator(MCLTOP_GTEQ));

    if (matched) {
        matcher_reduce(M, REDUCE_TYPE_CONCAT);
    }

    return matched;
}

bool match_binary_concat(struct matcher *M, bool is_wrapped) {
    if (!is_wrapped) {
        matcher_reduce(M, REDUCE_TYPE_ADDITIVE);
    }

    bool matched = matcher_match(M, symbol_operator(MCLTOP_DOTDOT));

    if (matched) {
        matcher_reduce(M, REDUCE_TYPE_ADDITIVE);
    }

    return matched;
}

bool match_binary_additive(struct matcher *M, bool is_wrapped) {
    if (!is_wrapped) {
        matcher_reduce(M, REDUCE_TYPE_MULTIPLICATIVE);
    }

    bool matched = matcher_match(M, symbol_operator(MCLTOP_PLUS)) ||
                   matcher_match(M, symbol_operator(MCLTOP_MINUS));

    if (matched) {
        matcher_reduce(M, REDUCE_TYPE_MULTIPLICATIVE);
    }

    return matched;
}

bool match_binary_multiplicative(struct matcher *M, bool is_wrapped) {
    if (!is_wrapped) {
        matcher_reduce(M, REDUCE_TYPE_PREFIX);
    }

    bool matched = matcher_match(M, symbol_operator(MCLTOP_STAR)) ||
                   matcher_match(M, symbol_operator(MCLTOP_SLASH)) ||
                   matcher_match(M, symbol_operator(MCLTOP_PERCENT));

    if (matched) {
        matcher_reduce(M, REDUCE_TYPE_PREFIX);
    }

    return matched;
}

struct ast_node *assemble_binary(morphine_coroutine_t U, struct ast *A, struct elements *E) {
    struct reduce reduce_a = elements_get_reduce(E, 0);
    if (elements_size(E) == 1) {
        return ast_as_node(ast_node_as_expression(U, reduce_a.node));
    }

    struct mc_lex_token watch_token = elements_get_token(E, 1);
    struct reduce reduce_b = elements_get_reduce(E, 2);

    if (matcher_symbol(symbol_operator(MCLTOP_GT), watch_token)) {
        struct expression_binary *result = ast_create_expression_binary(U, A, watch_token.line);
        result->type = EXPRESSION_BINARY_TYPE_LESS;
        result->b = ast_node_as_expression(U, reduce_a.node);
        result->a = ast_node_as_expression(U, reduce_b.node);

        return ast_as_node(result);
    } else if (matcher_symbol(symbol_operator(MCLTOP_LTEQ), watch_token)) {
        struct expression_binary *less = ast_create_expression_binary(U, A, watch_token.line);
        less->type = EXPRESSION_BINARY_TYPE_LESS;
        less->a = ast_node_as_expression(U, reduce_a.node);
        less->b = ast_node_as_expression(U, reduce_b.node);

        struct expression_binary *eq = ast_create_expression_binary(U, A, watch_token.line);
        eq->type = EXPRESSION_BINARY_TYPE_EQUAL;
        eq->a = ast_node_as_expression(U, reduce_a.node);
        eq->b = ast_node_as_expression(U, reduce_b.node);

        struct expression_binary *result = ast_create_expression_binary(U, A, watch_token.line);
        result->type = EXPRESSION_BINARY_TYPE_OR;
        result->a = ast_as_expression(less);
        result->b = ast_as_expression(eq);

        return ast_as_node(result);
    } else if (matcher_symbol(symbol_operator(MCLTOP_GTEQ), watch_token)) {
        struct expression_binary *less = ast_create_expression_binary(U, A, watch_token.line);
        less->type = EXPRESSION_BINARY_TYPE_LESS;
        less->b = ast_node_as_expression(U, reduce_a.node);
        less->a = ast_node_as_expression(U, reduce_b.node);

        struct expression_binary *eq = ast_create_expression_binary(U, A, watch_token.line);
        eq->type = EXPRESSION_BINARY_TYPE_EQUAL;
        eq->a = ast_node_as_expression(U, reduce_a.node);
        eq->b = ast_node_as_expression(U, reduce_b.node);

        struct expression_binary *result = ast_create_expression_binary(U, A, watch_token.line);
        result->type = EXPRESSION_BINARY_TYPE_OR;
        result->a = ast_as_expression(less);
        result->b = ast_as_expression(eq);

        return ast_as_node(result);
    } else if (matcher_symbol(symbol_operator(MCLTOP_EXCLEQ), watch_token)) {
        struct expression_binary *binary = ast_create_expression_binary(U, A, watch_token.line);
        binary->type = EXPRESSION_BINARY_TYPE_EQUAL;
        binary->a = ast_node_as_expression(U, reduce_a.node);
        binary->b = ast_node_as_expression(U, reduce_b.node);

        struct expression_unary *result = ast_create_expression_unary(U, A, watch_token.line);
        result->type = EXPRESSION_UNARY_TYPE_NOT;
        result->expression = ast_as_expression(binary);

        return ast_as_node(result);
    }

    struct expression_binary *result = ast_create_expression_binary(U, A, watch_token.line);

    if (matcher_symbol(symbol_operator(MCLTOP_PLUS), watch_token)) {
        result->type = EXPRESSION_BINARY_TYPE_ADD;
    } else if (matcher_symbol(symbol_operator(MCLTOP_MINUS), watch_token)) {
        result->type = EXPRESSION_BINARY_TYPE_SUB;
    } else if (matcher_symbol(symbol_operator(MCLTOP_STAR), watch_token)) {
        result->type = EXPRESSION_BINARY_TYPE_MUL;
    } else if (matcher_symbol(symbol_operator(MCLTOP_SLASH), watch_token)) {
        result->type = EXPRESSION_BINARY_TYPE_DIV;
    } else if (matcher_symbol(symbol_operator(MCLTOP_PERCENT), watch_token)) {
        result->type = EXPRESSION_BINARY_TYPE_MOD;
    } else if (matcher_symbol(symbol_operator(MCLTOP_EQEQ), watch_token)) {
        result->type = EXPRESSION_BINARY_TYPE_EQUAL;
    } else if (matcher_symbol(symbol_operator(MCLTOP_LT), watch_token)) {
        result->type = EXPRESSION_BINARY_TYPE_LESS;
    } else if (matcher_symbol(symbol_operator(MCLTOP_DOTDOT), watch_token)) {
        result->type = EXPRESSION_BINARY_TYPE_CONCAT;
    } else if (matcher_symbol(symbol_predef_word(MCLTPW_and), watch_token)) {
        result->type = EXPRESSION_BINARY_TYPE_AND;
    } else if (matcher_symbol(symbol_predef_word(MCLTPW_or), watch_token)) {
        result->type = EXPRESSION_BINARY_TYPE_OR;
    } else {
        return NULL;
    }

    result->a = ast_node_as_expression(U, reduce_a.node);
    result->b = ast_node_as_expression(U, reduce_b.node);

    return ast_as_node(result);
}