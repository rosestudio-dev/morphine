//
// Created by why-iskra on 31.05.2024.
//

#pragma once

#include "elements.h"

struct argument_matcher {
    bool assemble;
    morphine_coroutine_t U;
    union {
        struct matcher *M;
        struct elements *E;
    };

    struct matcher_symbol separator;
    bool has_open_close;
    bool consume_open;
    struct matcher_symbol open_symbol;
    struct matcher_symbol close_symbol;

    size_t pos;
    size_t count;
    bool closed;
    bool opened;
};

struct mc_lex_token argument_matcher_consume(struct argument_matcher *, struct matcher_symbol);
bool argument_matcher_match(struct argument_matcher *, struct matcher_symbol);
bool argument_matcher_look(struct argument_matcher *, struct matcher_symbol);
struct reduce argument_matcher_reduce(struct argument_matcher *, enum reduce_type);

bool argument_matcher_init(struct argument_matcher *, size_t init_pos);
bool argument_matcher_next(struct argument_matcher *);
size_t argument_matcher_close(struct argument_matcher *);
