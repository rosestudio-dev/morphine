//
// Created by why-iskra on 02.06.2024.
//

#pragma once

#include <morphine.h>
#include "morphinec/strtable.h"
#include "morphinec/ast.h"

void printer_strtable(morphine_coroutine_t, struct strtable *);
void printer_ast(morphine_coroutine_t, struct strtable *, struct ast *);
