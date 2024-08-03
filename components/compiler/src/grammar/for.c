//
// Created by why-iskra on 31.05.2024.
//

#include "impl.h"

void match_for(struct matcher *M) {
    matcher_consume(M, symbol_predef_word(MCLTPW_for));

    matcher_consume(M, symbol_operator(MCLTOP_LPAREN));
    matcher_reduce(M, REDUCE_TYPE_STATEMENT);
    matcher_consume(M, symbol_operator(MCLTOP_SEMICOLON));
    matcher_reduce(M, REDUCE_TYPE_EXPRESSION);
    matcher_consume(M, symbol_operator(MCLTOP_SEMICOLON));
    matcher_reduce(M, REDUCE_TYPE_STATEMENT);
    matcher_consume(M, symbol_operator(MCLTOP_RPAREN));

    matcher_reduce(M, REDUCE_TYPE_STATEMENT_BLOCK);
}

struct ast_node *assemble_for(morphine_coroutine_t U, struct ast *A, struct elements *E) {
    ml_line line = elements_get_token(E, 0).line;
    struct statement_for *result = ast_create_statement_for(U, A, line);

    struct reduce reduce_initial = elements_get_reduce(E, 2);
    struct reduce reduce_condition = elements_get_reduce(E, 4);
    struct reduce reduce_increment = elements_get_reduce(E, 6);
    struct reduce reduce_statement = elements_get_reduce(E, 8);

    result->initial = ast_node_as_statement(U, reduce_initial.node);
    result->condition = ast_node_as_expression(U, reduce_condition.node);
    result->increment = ast_node_as_statement(U, reduce_increment.node);
    result->statement = ast_node_as_statement(U, reduce_statement.node);

    return ast_as_node(result);
}