//
// Created by why-iskra on 02.06.2024.
//

#pragma once

#include <morphine.h>
#include "visitor.h"

struct codegen;

struct codegen *codegen(morphine_coroutine_t, struct strtable *, struct ast *, struct visitor *);
struct codegen *get_codegen(morphine_coroutine_t);

bool codegen_step(morphine_coroutine_t, struct codegen *);
void codegen_construct(morphine_coroutine_t, struct codegen *);
