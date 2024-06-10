//
// Created by why-iskra on 31.05.2024.
//

#include "arguments.h"

struct token argument_matcher_consume(
    struct argument_matcher *R,
    struct matcher_symbol symbol
) {
    if (R->assemble) {
        struct token token = elements_get_token(R->E, R->pos);
        if (!matcher_symbol(symbol, token)) {
            mapi_error(R->U, "unexpected argument pattern while assembling");
        }

        R->pos++;
        return token;
    } else {
        return matcher_consume(R->M, symbol);
    }
}

bool argument_matcher_match(
    struct argument_matcher *R,
    struct matcher_symbol symbol
) {
    if (R->assemble) {
        if (!elements_is_token(R->E, R->pos)) {
            return false;
        }

        struct token token = elements_get_token(R->E, R->pos);
        bool eq = matcher_symbol(symbol, token);
        if (eq) {
            R->pos++;
        }

        return eq;
    } else {
        return matcher_match(R->M, symbol);
    }
}

struct reduce argument_matcher_reduce(
    struct argument_matcher *R,
    enum reduce_type type
) {
    if (!R->assemble) {
        matcher_error(R->M, "argument matcher isn't in assemble mode");
    }

    struct reduce reduce = elements_get_reduce(R->E, R->pos);

    if (reduce.type != type) {
        mapi_error(R->U, "unexpected argument pattern while assembling");
    }

    R->pos++;
    return reduce;
}

bool argument_matcher_init(struct argument_matcher *R, size_t init_pos) {
    R->pos = init_pos;
    R->count = 0;
    R->closed = false;
    R->opened = false;

    if (R->has_open_close) {
        if (R->consume_open) {
            argument_matcher_consume(R, R->open_symbol);
            R->opened = true;
        } else {
            R->opened = argument_matcher_match(R, R->open_symbol);
        }
    }

    if (R->opened && argument_matcher_match(R, R->close_symbol)) {
        R->closed = true;
        return false;
    }

    return true;
}

bool argument_matcher_next(struct argument_matcher *R) {
    R->count++;

    if (R->closed || !argument_matcher_match(R, R->separator)) {
        return false;
    }

    if (R->opened && argument_matcher_match(R, R->close_symbol)) {
        R->closed = true;
        return false;
    }

    return true;
}

size_t argument_matcher_close(struct argument_matcher *R) {
    if (R->opened && !R->closed) {
        argument_matcher_consume(R, R->close_symbol);
    }

    return R->count;
}