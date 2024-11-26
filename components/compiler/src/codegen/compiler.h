//
// Created by why-iskra on 16.08.2024.
//

#pragma once

#include "controller.h"

#define expr(n) void codegen_compile_expression_##n(struct codegen_controller *, struct mc_ast_expression_##n *, size_t)
#define stmt(n) void codegen_compile_statement_##n(struct codegen_controller *, struct mc_ast_statement_##n *, size_t)
#define set(n)  void codegen_compile_set_##n(struct codegen_controller *, struct mc_ast_expression_##n *, size_t)

expr(value);
expr(binary);
expr(unary);
expr(increment);
expr(variable);
expr(global);
expr(leave);
expr(break);
expr(continue);
expr(table);
expr(vector);
expr(access);
expr(call);
expr(function);
expr(block);
expr(if);
expr(when);
expr(asm);

stmt(pass);
stmt(yield);
stmt(eval);
stmt(while);
stmt(for);
stmt(iterator);
stmt(declaration);
stmt(assigment);

set(variable);
set(access);

#undef expr
#undef stmt
#undef set
