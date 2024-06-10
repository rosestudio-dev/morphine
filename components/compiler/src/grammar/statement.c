//
// Created by why-iskra on 31.05.2024.
//

#include "impl.h"

void match_statement(struct matcher *M) {
    if (matcher_match(M, symbol_predef_word(TPW_pass))) { return; }
    if (matcher_match(M, symbol_predef_word(TPW_yield))) { return; }
    if (matcher_match(M, symbol_predef_word(TPW_leave))) { return; }
    if (matcher_match(M, symbol_predef_word(TPW_break))) { return; }
    if (matcher_match(M, symbol_predef_word(TPW_continue))) { return; }

    if (matcher_match(M, symbol_predef_word(TPW_eval))) {
        matcher_reduce(M, REDUCE_TYPE_EXPRESSION);
        return;
    }

    if (matcher_match(M, symbol_predef_word(TPW_return))) {
        matcher_reduce(M, REDUCE_TYPE_EXPRESSION);
        return;
    }

    if (matcher_look(M, symbol_predef_word(TPW_while)) ||
        matcher_is_reduced(M, REDUCE_TYPE_WHILE)) {
        matcher_reduce(M, REDUCE_TYPE_WHILE);
        return;
    }

    if (matcher_look(M, symbol_predef_word(TPW_do)) ||
        matcher_is_reduced(M, REDUCE_TYPE_DO_WHILE)) {
        matcher_reduce(M, REDUCE_TYPE_DO_WHILE);
        return;
    }

    if (matcher_look(M, symbol_predef_word(TPW_for)) ||
        matcher_is_reduced(M, REDUCE_TYPE_FOR)) {
        matcher_reduce(M, REDUCE_TYPE_FOR);
        return;
    }

    if (matcher_look(M, symbol_predef_word(TPW_iterator)) ||
        matcher_is_reduced(M, REDUCE_TYPE_ITERATOR)) {
        matcher_reduce(M, REDUCE_TYPE_ITERATOR);
        return;
    }

    if (matcher_look(M, symbol_predef_word(TPW_if)) ||
        matcher_is_reduced(M, REDUCE_TYPE_STATEMENT_IF)) {
        matcher_reduce(M, REDUCE_TYPE_STATEMENT_IF);
        return;
    }

    if (matcher_look(M, symbol_predef_word(TPW_val)) ||
        matcher_look(M, symbol_predef_word(TPW_var)) ||
        matcher_look(M, symbol_predef_word(TPW_fun)) ||
        matcher_is_reduced(M, REDUCE_TYPE_DECLARATION)) {
        matcher_reduce(M, REDUCE_TYPE_DECLARATION);
        return;
    }

    matcher_reduce(M, REDUCE_TYPE_ASSIGMENT);
}

struct ast_node *assemble_statement(morphine_coroutine_t U, struct ast *A, struct elements *E) {
    if (!elements_is_token(E, 0)) {
        return elements_get_reduce(E, 0).node;
    }

    struct token watch_token = elements_get_token(E, 0);

    if (matcher_symbol(symbol_predef_word(TPW_pass), watch_token)) {
        struct statement_simple *result = ast_create_statement_simple(U, A, watch_token.line);
        result->type = STATEMENT_SIMPLE_TYPE_PASS;

        return ast_as_node(result);
    }

    if (matcher_symbol(symbol_predef_word(TPW_yield), watch_token)) {
        struct statement_simple *result = ast_create_statement_simple(U, A, watch_token.line);
        result->type = STATEMENT_SIMPLE_TYPE_YIELD;

        return ast_as_node(result);
    }

    if (matcher_symbol(symbol_predef_word(TPW_leave), watch_token)) {
        struct statement_simple *result = ast_create_statement_simple(U, A, watch_token.line);
        result->type = STATEMENT_SIMPLE_TYPE_LEAVE;

        return ast_as_node(result);
    }

    if (matcher_symbol(symbol_predef_word(TPW_break), watch_token)) {
        struct statement_simple *result = ast_create_statement_simple(U, A, watch_token.line);
        result->type = STATEMENT_SIMPLE_TYPE_BREAK;

        return ast_as_node(result);
    }

    if (matcher_symbol(symbol_predef_word(TPW_continue), watch_token)) {
        struct statement_simple *result = ast_create_statement_simple(U, A, watch_token.line);
        result->type = STATEMENT_SIMPLE_TYPE_CONTINUE;

        return ast_as_node(result);
    }

    if (matcher_symbol(symbol_predef_word(TPW_eval), watch_token)) {
        struct reduce reduce = elements_get_reduce(E, 1);
        struct statement_eval *result = ast_create_statement_eval(U, A, watch_token.line);
        result->expression = ast_node_as_expression(U, reduce.node);
        result->implicit = false;

        return ast_as_node(result);
    }

    if (matcher_symbol(symbol_predef_word(TPW_return), watch_token)) {
        struct reduce reduce = elements_get_reduce(E, 1);
        struct statement_return *result = ast_create_statement_return(U, A, watch_token.line);
        result->expression = ast_node_as_expression(U, reduce.node);

        return ast_as_node(result);
    }

    return NULL;
}
