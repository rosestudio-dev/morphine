//
// Created by why-iskra on 03.06.2024.
//

#pragma once

#include <morphine.h>
#include "morphinec/visitor.h"
#include "morphinec/strtable.h"
#include "morphinec/ast.h"

#define MC_CODEGEN_USERDATA_TYPE "morphinec-codegen"

struct mc_codegen;

MORPHINE_API struct mc_codegen *mcapi_push_codegen(morphine_coroutine_t);
MORPHINE_API struct mc_codegen *mcapi_get_codegen(morphine_coroutine_t);

MORPHINE_API bool mcapi_codegen_step(
    morphine_coroutine_t,
    struct mc_codegen *,
    struct mc_visitor *,
    struct mc_strtable *,
    struct mc_ast *
);

MORPHINE_API void mcapi_codegen_build(
    morphine_coroutine_t,
    struct mc_codegen *,
    struct mc_strtable *,
    struct mc_ast *A,
    const char *main
);
