//
// Created by why-iskra on 31.05.2024.
//

#include "impl.h"
#include "support/block.h"

void match_statement_block(struct matcher *M) {
    struct matcher_symbol end = symbol_predef_word(TPW_end);
    match_sblock(M, 1, &end);
}

void match_expression_block(struct matcher *M) {
    struct matcher_symbol end = symbol_predef_word(TPW_end);
    match_eblock(M, 1, &end);
}

struct ast_node *assemble_statement_block(morphine_coroutine_t U, struct elements *E) {
    struct matcher_symbol end = symbol_predef_word(TPW_end);
    struct statement *result = NULL;
    get_sblock(U, E, 1, &end, 0, &result, NULL);

    return ast_as_node(result);
}

struct ast_node *assemble_expression_block(morphine_coroutine_t U, struct elements *E) {
    struct matcher_symbol end = symbol_predef_word(TPW_end);
    struct expression *result = NULL;
    get_eblock(U, E, 1, &end, 0, &result, NULL);

    return ast_as_node(result);
}

void match_block_elem(struct matcher *M) {
    matcher_reduce(M, REDUCE_TYPE_STATEMENT);
    matcher_match(M, symbol_operator(TOP_SEMICOLON));
}

void match_implicit_block_elem(struct matcher *M) {
    matcher_reduce(M, REDUCE_TYPE_STATEMENT);
    matcher_match(M, symbol_operator(TOP_SEMICOLON));
}

struct ast_node *assemble_block_elem(morphine_coroutine_t U, struct elements *E) {
    struct reduce reduce = elements_get_reduce(E, 0);
    struct statement *statement = ast_node_as_statement(U, reduce.node);
    if (ast_statement_type(statement) == STATEMENT_TYPE_eval) {
        struct statement_eval *eval = ast_as_statement_eval(U, reduce.node);
        if (eval->implicit) {
            elements_error(E, 0, "implicit expression");
        }
    }

    return ast_as_node(statement);
}

struct ast_node *assemble_implicit_block_elem(morphine_coroutine_t U, struct elements *E) {
    struct reduce reduce = elements_get_reduce(E, 0);
    return ast_as_node(ast_node_as_statement(U, reduce.node));
}
