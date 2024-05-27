//
// Created by why-iskra on 31.05.2024.
//

#include "impl.h"

bool match_binary_or(struct matcher *M, bool is_wrapped) {
    if (!is_wrapped) {
        matcher_reduce(M, REDUCE_TYPE_AND);
    }

    bool matched = matcher_match(M, symbol_predef_word(TPW_or));

    if (matched) {
        matcher_reduce(M, REDUCE_TYPE_AND);
    }

    return matched;
}

bool match_binary_and(struct matcher *M, bool is_wrapped) {
    if (!is_wrapped) {
        matcher_reduce(M, REDUCE_TYPE_EQUAL);
    }

    bool matched = matcher_match(M, symbol_predef_word(TPW_and));

    if (matched) {
        matcher_reduce(M, REDUCE_TYPE_EQUAL);
    }

    return matched;
}

bool match_binary_equal(struct matcher *M, bool is_wrapped) {
    if (!is_wrapped) {
        matcher_reduce(M, REDUCE_TYPE_CONDITION);
    }

    bool matched = matcher_match(M, symbol_operator(TOP_EQEQ)) ||
                   matcher_match(M, symbol_operator(TOP_EXCLEQ));

    if (matched) {
        matcher_reduce(M, REDUCE_TYPE_CONDITION);
    }

    return matched;
}

bool match_binary_condition(struct matcher *M, bool is_wrapped) {
    if (!is_wrapped) {
        matcher_reduce(M, REDUCE_TYPE_CONCAT);
    }

    bool matched = matcher_match(M, symbol_operator(TOP_LT)) ||
                   matcher_match(M, symbol_operator(TOP_GT)) ||
                   matcher_match(M, symbol_operator(TOP_LTEQ)) ||
                   matcher_match(M, symbol_operator(TOP_GTEQ));

    if (matched) {
        matcher_reduce(M, REDUCE_TYPE_CONCAT);
    }

    return matched;
}

bool match_binary_concat(struct matcher *M, bool is_wrapped) {
    if (!is_wrapped) {
        matcher_reduce(M, REDUCE_TYPE_ADDITIVE);
    }

    bool matched = matcher_match(M, symbol_operator(TOP_DOTDOT));

    if (matched) {
        matcher_reduce(M, REDUCE_TYPE_ADDITIVE);
    }

    return matched;
}

bool match_binary_additive(struct matcher *M, bool is_wrapped) {
    if (!is_wrapped) {
        matcher_reduce(M, REDUCE_TYPE_MULTIPLICATIVE);
    }

    bool matched = matcher_match(M, symbol_operator(TOP_PLUS)) ||
                   matcher_match(M, symbol_operator(TOP_MINUS));

    if (matched) {
        matcher_reduce(M, REDUCE_TYPE_MULTIPLICATIVE);
    }

    return matched;
}

bool match_binary_multiplicative(struct matcher *M, bool is_wrapped) {
    if (!is_wrapped) {
        matcher_reduce(M, REDUCE_TYPE_PREFIX);
    }

    bool matched = matcher_match(M, symbol_operator(TOP_STAR)) ||
                   matcher_match(M, symbol_operator(TOP_SLASH)) ||
                   matcher_match(M, symbol_operator(TOP_PERCENT));

    if (matched) {
        matcher_reduce(M, REDUCE_TYPE_PREFIX);
    }

    return matched;
}

struct ast_node *assemble_binary(morphine_coroutine_t U, struct elements *E) {
    struct reduce reduce_a = elements_get_reduce(E, 0);
    if (elements_size(E) == 1) {
        return ast_as_node(ast_node_as_expression(U, reduce_a.node));
    }

    struct token watch_token = elements_get_token(E, 1);
    struct expression_binary *result = ast_create_expression_binary(U, watch_token.line);

    if (matcher_symbol(symbol_operator(TOP_PLUS), watch_token)) {
        result->type = EXPRESSION_BINARY_TYPE_ADD;
    } else if (matcher_symbol(symbol_operator(TOP_MINUS), watch_token)) {
        result->type = EXPRESSION_BINARY_TYPE_SUB;
    } else if (matcher_symbol(symbol_operator(TOP_STAR), watch_token)) {
        result->type = EXPRESSION_BINARY_TYPE_MUL;
    } else if (matcher_symbol(symbol_operator(TOP_SLASH), watch_token)) {
        result->type = EXPRESSION_BINARY_TYPE_DIV;
    } else if (matcher_symbol(symbol_operator(TOP_PERCENT), watch_token)) {
        result->type = EXPRESSION_BINARY_TYPE_MOD;
    } else if (matcher_symbol(symbol_operator(TOP_EQEQ), watch_token)) {
        result->type = EXPRESSION_BINARY_TYPE_EQUAL;
    } else if (matcher_symbol(symbol_operator(TOP_EXCLEQ), watch_token)) {
        result->type = EXPRESSION_BINARY_TYPE_NOT_EQUAL;
    } else if (matcher_symbol(symbol_operator(TOP_LT), watch_token)) {
        result->type = EXPRESSION_BINARY_TYPE_LESS;
    } else if (matcher_symbol(symbol_operator(TOP_GT), watch_token)) {
        result->type = EXPRESSION_BINARY_TYPE_MORE;
    } else if (matcher_symbol(symbol_operator(TOP_LTEQ), watch_token)) {
        result->type = EXPRESSION_BINARY_TYPE_LESS_EQUAL;
    } else if (matcher_symbol(symbol_operator(TOP_GTEQ), watch_token)) {
        result->type = EXPRESSION_BINARY_TYPE_MORE_EQUAL;
    } else if (matcher_symbol(symbol_operator(TOP_DOTDOT), watch_token)) {
        result->type = EXPRESSION_BINARY_TYPE_CONCAT;
    } else if (matcher_symbol(symbol_predef_word(TPW_and), watch_token)) {
        result->type = EXPRESSION_BINARY_TYPE_AND;
    } else if (matcher_symbol(symbol_predef_word(TPW_or), watch_token)) {
        result->type = EXPRESSION_BINARY_TYPE_OR;
    } else {
        return NULL;
    }

    struct reduce reduce_b = elements_get_reduce(E, 2);

    result->a = ast_node_as_expression(U, reduce_a.node);
    result->b = ast_node_as_expression(U, reduce_b.node);

    return ast_as_node(result);
}