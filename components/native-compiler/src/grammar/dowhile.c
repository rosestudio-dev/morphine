//
// Created by why-iskra on 31.05.2024.
//

#include "impl.h"

void match_do_while(struct matcher *M) {
    matcher_consume(M, symbol_predef_word(TPW_do));
    matcher_reduce(M, REDUCE_TYPE_STATEMENT_BLOCK);

    if (matcher_match(M, symbol_predef_word(TPW_while))) {
        matcher_consume(M, symbol_operator(TOP_LPAREN));
        matcher_reduce(M, REDUCE_TYPE_EXPRESSION);
        matcher_consume(M, symbol_operator(TOP_RPAREN));
    }
}

struct ast_node *assemble_do_while(morphine_coroutine_t U, struct elements *E) {
    if (elements_size(E) == 2) {
        return elements_get_reduce(E, 1).node;
    }

    uint32_t line = elements_get_token(E, 0).line;
    struct statement_while *result = ast_create_statement_while(U, line);

    struct reduce reduce_expression = elements_get_reduce(E, 4);
    struct reduce reduce_statement = elements_get_reduce(E, 1);

    result->first_condition = false;
    result->condition = ast_node_as_expression(U, reduce_expression.node);
    result->statement = ast_node_as_statement(U, reduce_statement.node);

    return ast_as_node(result);
}