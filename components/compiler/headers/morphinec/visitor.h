//
// Created by why-iskra on 03.06.2024.
//

#pragma once

#include <morphine.h>
#include "morphinec/ast.h"

#define MC_VISITOR_USERDATA_TYPE "morphinec-visitor"

enum mc_visitor_event {
    MCVE_INIT,
    MCVE_ENTER_NODE,
    MCVE_ENTER_FUNCTION,
    MCVE_DROP_NODE,
    MCVE_DROP_FUNCTION,
    MCVE_NEXT_STEP,
    MCVE_COMPLETE,
};

struct mc_visitor;
struct mc_visitor_controller;

typedef void (*mc_visitor_function_t)(
    struct mc_visitor_controller *,
    struct mc_ast_node *,
    size_t state
);

MORPHINE_API struct mc_visitor *mcapi_push_visitor(morphine_coroutine_t);
MORPHINE_API struct mc_visitor *mcapi_get_visitor(morphine_coroutine_t);
MORPHINE_API bool mcapi_visitor_step(
    morphine_coroutine_t,
    struct mc_visitor *,
    struct mc_ast *,
    mc_visitor_function_t,
    enum mc_visitor_event *,
    void *
);

MORPHINE_API morphine_noret void mcapi_visitor_node(
    struct mc_visitor_controller *,
    struct mc_ast_node *,
    size_t next_state
);

MORPHINE_API morphine_noret void mcapi_visitor_function(
    struct mc_visitor_controller *,
    struct mc_ast_function *,
    size_t next_state
);

MORPHINE_API morphine_noret void mcapi_visitor_jump(
    struct mc_visitor_controller *,
    size_t next_state
);

MORPHINE_API morphine_noret void mcapi_visitor_complete(struct mc_visitor_controller *);

MORPHINE_API void *mcapi_visitor_data(struct mc_visitor_controller *);
MORPHINE_API void *mcapi_visitor_saved(struct mc_visitor_controller *);
MORPHINE_API void *mcapi_visitor_alloc_saved_uni(struct mc_visitor_controller *, size_t);
MORPHINE_API void *mcapi_visitor_alloc_saved_vec(struct mc_visitor_controller *, size_t, size_t);
