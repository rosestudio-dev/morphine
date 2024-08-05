//
// Created by why-iskra on 05.08.2024.
//

#pragma once

#include "../controller.h"

struct arguments {
    bool has_open_close;
    bool consume;
    struct mc_lex_token open;
    struct mc_lex_token close;
    struct mc_lex_token separator;

    bool opened;
    size_t count;
};

struct arguments extra_arguments_init_full(
    struct parse_controller *C,
    bool consume_open,
    struct mc_lex_token open,
    struct mc_lex_token close,
    struct mc_lex_token separator
);

struct arguments extra_arguments_init_simple(
    struct parse_controller *C,
    struct mc_lex_token separator
);

bool extra_arguments_next(struct parse_controller *C, struct arguments *A);
size_t extra_arguments_finish(struct parse_controller *C, struct arguments *A);
