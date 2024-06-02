//
// Created by why-iskra on 28.05.2024.
//

#pragma once

#include "matcher.h"

struct elements;

struct reduce {
    enum reduce_type type;
    struct ast_node *node;
};

size_t elements_size(struct elements *);
ml_line elements_line(struct elements *, size_t);
morphine_noret void elements_error(struct elements *, size_t, const char *);
bool elements_is_token(struct elements *, size_t);
struct token elements_get_token(struct elements *, size_t);
struct reduce elements_get_reduce(struct elements *, size_t);
bool elements_look(struct elements *, size_t, struct matcher_symbol);
