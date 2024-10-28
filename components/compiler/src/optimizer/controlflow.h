//
// Created by why-iskra on 29.09.2024.
//

#pragma once

#include <morphine.h>
#include "blocks.h"

struct controlflow;

struct controlflow *controlflow_alloc(morphine_coroutine_t);
void controlflow_free(morphine_instance_t, struct controlflow *);
void controlflow_build(morphine_coroutine_t, struct instructions *, struct blocks *, struct controlflow *);
void controlflow_add_edge(morphine_coroutine_t, struct controlflow *, ml_size, ml_size);
void controlflow_incoming_edges(morphine_coroutine_t, struct controlflow *, ml_size);
void controlflow_outgoing_edges(morphine_coroutine_t, struct controlflow *, ml_size);
void controlflow_dfs(morphine_coroutine_t, struct controlflow *, ml_size);
void controlflow_dump(struct controlflow *);
