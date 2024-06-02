//
// Created by why-iskra on 31.05.2024.
//

#include "impl.h"

void match_while(struct matcher *M) {
    matcher_consume(M, symbol_predef_word(TPW_while));
    matcher_consume(M, symbol_operator(TOP_LPAREN));
    matcher_reduce(M, REDUCE_TYPE_EXPRESSION);
    matcher_consume(M, symbol_operator(TOP_RPAREN));
    matcher_reduce(M, REDUCE_TYPE_STATEMENT_BLOCK);
}

struct ast_node *assemble_while(morphine_coroutine_t U, struct elements *E) {
    uint32_t line = elements_get_token(E, 0).line;
    struct statement_while *result = ast_create_statement_while(U, line);

    struct reduce reduce_expression = elements_get_reduce(E, 2);
    struct reduce reduce_statement = elements_get_reduce(E, 4);

    result->first_condition = true;
    result->condition = ast_node_as_expression(U, reduce_expression.node);
    result->statement = ast_node_as_statement(U, reduce_statement.node);

    return ast_as_node(result);
}