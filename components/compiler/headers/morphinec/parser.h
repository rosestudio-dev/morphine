//
// Created by why-iskra on 27.05.2024.
//

#pragma once

#include <morphine.h>
#include "morphinec/lex.h"
#include "morphinec/ast.h"

struct parser;

struct parser *parser(morphine_coroutine_t, struct lex *, struct ast *);
struct parser *get_parser(morphine_coroutine_t);

bool parser_step(morphine_coroutine_t, struct parser *);
