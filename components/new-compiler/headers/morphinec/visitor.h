//
// Created by why-iskra on 03.06.2024.
//

#pragma once

#include "ast.h"

struct visitor_controller;

typedef void (*visit_func_t)(struct visitor_controller *, struct ast_node *, size_t state);

void visitor(morphine_coroutine_t, visit_func_t, void *);
bool visitor_step(morphine_coroutine_t, void *);

bool visitor_has_saved(struct visitor_controller *);
bool visitor_save(struct visitor_controller *, size_t, void **);
void *visitor_prev_save(struct visitor_controller *);
void *visitor_data(struct visitor_controller *);
ml_line visitor_line(struct visitor_controller *);
bool visitor_is_function_root(struct visitor_controller *);
morphine_noret void visitor_next(struct visitor_controller *, size_t state);
morphine_noret void visitor_node(struct visitor_controller *, size_t state, struct ast_node *);
morphine_noret void visitor_function(struct visitor_controller *, size_t state, struct ast_function *);
morphine_noret void visitor_return(struct visitor_controller *);
