//
// Created by why-iskra on 31.05.2024.
//

#include "impl.h"

void match_expression(struct matcher *M) {
    matcher_reduce(M, REDUCE_TYPE_OR);
}

struct ast_node *assemble_expression(morphine_coroutine_t U, struct ast *A, struct elements *E) {
    (void) A;

    struct reduce reduce = elements_get_reduce(E, 0);
    return ast_as_node(ast_node_as_expression(U, reduce.node));
}
