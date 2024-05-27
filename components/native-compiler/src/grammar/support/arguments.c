//
// Created by why-iskra on 31.05.2024.
//

#include "arguments.h"

struct token argument_matcher_consume(
    struct argument_matcher *A,
    struct matcher_symbol symbol
) {
    if (A->assemble) {
        struct token token = elements_get_token(A->E, A->pos);
        if (!matcher_symbol(symbol, token)) {
            mapi_error(A->U, "unexpected argument pattern while assembling");
        }

        A->pos++;
        return token;
    } else {
        return matcher_consume(A->M, symbol);
    }
}

bool argument_matcher_match(
    struct argument_matcher *A,
    struct matcher_symbol symbol
) {
    if (A->assemble) {
        if (!elements_is_token(A->E, A->pos)) {
            return false;
        }

        struct token token = elements_get_token(A->E, A->pos);
        bool eq = matcher_symbol(symbol, token);
        if (eq) {
            A->pos++;
        }

        return eq;
    } else {
        return matcher_match(A->M, symbol);
    }
}

struct reduce argument_matcher_reduce(
    struct argument_matcher *A,
    enum reduce_type type
) {
    if (!A->assemble) {
        matcher_error(A->M, "argument matcher isn't in assemble mode");
    }

    struct reduce reduce = elements_get_reduce(A->E, A->pos);

    if (reduce.type != type) {
        mapi_error(A->U, "unexpected argument pattern while assembling");
    }

    A->pos++;
    return reduce;
}

bool argument_matcher_init(struct argument_matcher *A, size_t init_pos) {
    A->pos = init_pos;
    A->count = 0;
    A->closed = false;
    A->opened = false;

    if (A->has_open_close) {
        if (A->consume_open) {
            argument_matcher_consume(A, A->open_symbol);
            A->opened = true;
        } else {
            A->opened = argument_matcher_match(A, A->open_symbol);
        }
    }

    if (A->opened && argument_matcher_match(A, A->close_symbol)) {
        A->closed = true;
        return false;
    }

    return true;
}

bool argument_matcher_next(struct argument_matcher *A) {
    A->count++;

    if (A->closed || !argument_matcher_match(A, A->separator)) {
        return false;
    }

    if (A->opened && argument_matcher_match(A, A->close_symbol)) {
        A->closed = true;
        return false;
    }

    return true;
}

size_t argument_matcher_close(struct argument_matcher *A) {
    if (A->opened && !A->closed) {
        argument_matcher_consume(A, A->close_symbol);
    }

    return A->count;
}