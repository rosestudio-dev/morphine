//
// Created by why-iskra on 01.06.2024.
//

#include "impl.h"
#include "support/decompose.h"

void match_iterator(struct matcher *M) {
    matcher_consume(M, symbol_predef_word(MCLTPW_iterator));
    matcher_consume(M, symbol_operator(MCLTOP_LPAREN));

    match_decompose(M, true);
    matcher_consume(M, symbol_predef_word(MCLTPW_in));
    matcher_reduce(M, REDUCE_TYPE_EXPRESSION);

    matcher_consume(M, symbol_operator(MCLTOP_RPAREN));

    matcher_reduce(M, REDUCE_TYPE_STATEMENT_BLOCK);
}

struct ast_node *assemble_iterator(morphine_coroutine_t U, struct ast *A, struct elements *E) {
    size_t index = 0;
    size_t size = size_decompose(U, E, true, 2, &index);

    struct statement_iterator *result = ast_create_statement_iterator(U, A, elements_line(E, 0), size);
    result->expression = ast_node_as_expression(U, elements_get_reduce(E, index + 1).node);
    result->statement = ast_node_as_statement(U, elements_get_reduce(E, index + 3).node);

    if (size == 0) {
        insert_decompose(U, A, E, true, 2, &result->name, NULL, NULL);
    } else {
        insert_decompose(U, A, E, true, 2, result->multi.names, NULL, result->multi.keys);
    }

    return ast_as_node(result);
}