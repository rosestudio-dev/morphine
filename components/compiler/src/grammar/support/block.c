//
// Created by why-iskra on 02.06.2024.
//

#include "block.h"

size_t match_sblock(
    struct matcher *M,
    size_t closes_size,
    struct matcher_symbol *closes
) {
    while (true) {
        for (size_t i = 0; i < closes_size; i++) {
            if (matcher_match(M, closes[i])) {
                return i;
            }
        }

        matcher_reduce(M, REDUCE_TYPE_BLOCK_ELEM);
    }
}

size_t get_sblock(
    morphine_coroutine_t U,
    struct ast *A,
    struct elements *E,
    size_t closes_size,
    struct matcher_symbol *closes,
    size_t start_index,
    struct statement **result,
    size_t *end_index
) {
    size_t closed;
    size_t size = 0;
    while (true) {
        for (size_t i = 0; i < closes_size; i++) {
            if (elements_look(E, start_index + size, closes[i])) {
                closed = i;
                goto exit;
            }
        }

        size++;
    }

exit:;
    struct statement_block *block = ast_create_statement_block(U, A, elements_line(E, start_index), size);

    for (size_t i = 0; i < size; i++) {
        struct reduce reduce = elements_get_reduce(E, start_index + i);
        block->statements[i] = ast_node_as_statement(U, reduce.node);
    }

    if(result != NULL) {
        *result = ast_as_statement(block);
    }

    if(end_index != NULL) {
        *end_index = start_index + size;
    }

    return closed;
}

size_t match_eblock(
    struct matcher *M,
    size_t closes_size,
    struct matcher_symbol *closes
) {
    while (true) {
        matcher_reduce(M, REDUCE_TYPE_IMPLICIT_BLOCK_ELEM);

        for (size_t i = 0; i < closes_size; i++) {
            if (matcher_match(M, closes[i])) {
                return i;
            }
        }
    }
}

size_t get_eblock(
    morphine_coroutine_t U,
    struct ast *A,
    struct elements *E,
    size_t closes_size,
    struct matcher_symbol *closes,
    size_t start_index,
    struct expression **result,
    size_t *end_index
) {
    size_t closed;
    size_t size = 0;
    while (true) {
        for (size_t i = 0; i < closes_size; i++) {
            if (elements_look(E, start_index + size, closes[i])) {
                closed = i;
                goto exit;
            }
        }

        size++;
    }

exit:
    if (size == 0) {
        elements_error(E, start_index, "expression block must contain expression");
    } else {
        size--;
    }

    struct expression_block *block = ast_create_expression_block(U, A, elements_line(E, start_index), size);

    for (size_t i = 0; i < size; i++) {
        struct reduce reduce = elements_get_reduce(E, start_index + i);
        struct statement *statement = ast_node_as_statement(U, reduce.node);
        if (ast_statement_type(statement) == STATEMENT_TYPE_eval) {
            struct statement_eval *eval = ast_as_statement_eval(U, reduce.node);
            if (eval->implicit) {
                elements_error(E, start_index + i, "implicit expression");
            }
        }

        block->statements[i] = statement;
    }

    struct reduce reduce = elements_get_reduce(E, start_index + size);
    struct statement *statement = ast_node_as_statement(U, reduce.node);
    if (ast_statement_type(statement) == STATEMENT_TYPE_eval) {
        struct statement_eval *eval = ast_as_statement_eval(U, reduce.node);
        block->result = eval;
    } else {
        elements_error(E, start_index + size, "expression block must end with expression");
    }

    if(result != NULL) {
        *result = ast_as_expression(block);
    }

    if(end_index != NULL) {
        *end_index = start_index + size + 1;
    }

    return closed;
}