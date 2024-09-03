//
// Created by why-iskra on 05.08.2024.
//

#include "arguments.h"

struct arguments extra_arguments_init_full(
    struct parse_controller *C,
    bool consume_open,
    struct expected_token open,
    struct expected_token close,
    struct expected_token separator
) {
    bool opened = true;
    if (consume_open) {
        parser_consume(C, open);
    } else {
        opened = parser_match(C, open);
    }

    if (parser_look(C, close)) {
        opened = false;
    }

    return (struct arguments) {
        .has_open_close = true,
        .consume = consume_open,
        .open = open,
        .close = close,
        .separator = separator,
        .opened = opened,
        .count = 0
    };
}

struct arguments extra_arguments_init_simple(
    struct parse_controller *C,
    struct expected_token separator
) {
    (void) C;

    return (struct arguments) {
        .has_open_close = false,
        .consume = false,
        .separator = separator,
        .opened = true,
        .count = 0
    };
}

bool extra_arguments_next(struct parse_controller *C, struct arguments *A) {
    if (!A->opened) {
        return false;
    }

    if (A->count > 0) {
        if (!parser_match(C, A->separator)) {
            A->opened = false;
            return false;
        }

        if (A->has_open_close && parser_look(C, A->close)) {
            A->opened = false;
            return false;
        }
    }

    A->count++;
    return true;
}

size_t extra_arguments_finish(struct parse_controller *C, struct arguments *A) {
    A->opened = false;
    if (A->has_open_close) {
        if (A->consume) {
            parser_consume(C, A->close);
        } else {
            parser_match(C, A->close);
        }
    }

    return A->count;
}
