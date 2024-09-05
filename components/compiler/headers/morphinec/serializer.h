//
// Created by why-iskra on 05.09.2024.
//

#pragma once

#include <morphine.h>
#include "morphinec/visitor.h"
#include "morphinec/strtable.h"
#include "morphinec/ast.h"

MORPHINE_API bool mcapi_serializer_step(
    morphine_coroutine_t,
    struct mc_visitor *,
    struct mc_strtable *,
    struct mc_ast *
);
