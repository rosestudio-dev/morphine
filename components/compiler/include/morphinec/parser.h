//
// Created by why-iskra on 05.08.2024.
//

#pragma once

#include <morphine.h>
#include "morphinec/lex.h"
#include "morphinec/ast.h"

#define MC_PARSER_USERDATA_TYPE "morphinec-parser"

struct mc_parser;

MORPHINE_API struct mc_parser *mcapi_push_parser(morphine_coroutine_t);
MORPHINE_API struct mc_parser *mcapi_get_parser(morphine_coroutine_t);
MORPHINE_API bool mcapi_parser_step(
    morphine_coroutine_t,
    struct mc_parser *,
    struct mc_ast *,
    struct mc_lex *,
    struct mc_strtable *
);
