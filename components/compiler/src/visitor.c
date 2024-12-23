//
// Created by why-iskra on 03.06.2024.
//

#include <setjmp.h>
#include "morphinec/visitor.h"

enum command {
    COMMAND_NODE,
    COMMAND_FUNCTION,
    COMMAND_JUMP,
    COMMAND_COMPLETE,
};

struct node_context {
    bool has_allocated_data;
    void *allocated_data;
    struct mc_ast_node *node;
    size_t state;

    struct node_context *prev;
};

struct function_context {
    struct mc_ast_function *function;
    struct node_context *root_node_context;
    struct function_context *prev;
};

struct mc_visitor {
    bool first_launch;

    struct node_context *node_context;
    struct function_context *function_context;

    struct {
        struct node_context *node_context;
        struct function_context *function_context;
    } trash;
};

struct mc_visitor_controller {
    morphine_coroutine_t U;
    struct mc_visitor *V;

    void *data;
    jmp_buf jump;

    struct {
        enum command type;
        size_t next_state;
        union {
            struct mc_ast_node *node;
            struct mc_ast_function *function;
        };
    } command;
};

static struct function_context *push_function(
    morphine_coroutine_t U,
    struct mc_visitor *V,
    struct mc_ast_function *function
) {
    struct function_context *context;
    if (V->trash.function_context != NULL) {
        context = V->trash.function_context;
        V->trash.function_context = context->prev;
    } else {
        context = mapi_allocator_uni(
            mapi_instance(U), NULL, sizeof(struct function_context)
        );
    }

    *context = (struct function_context) {
        .function = function,
        .root_node_context = NULL,
        .prev = V->function_context
    };

    V->function_context = context;

    return context;
}

static struct node_context *push_node(
    morphine_coroutine_t U,
    struct mc_visitor *V,
    struct mc_ast_node *node
) {
    struct node_context *context;
    if (V->trash.node_context != NULL) {
        context = V->trash.node_context;
        V->trash.node_context = context->prev;
    } else {
        context = mapi_allocator_uni(
            mapi_instance(U), NULL, sizeof(struct node_context)
        );

        context->allocated_data = NULL;
    }

    context->has_allocated_data = false;
    context->state = 0;
    context->node = node;
    context->prev = V->node_context;

    V->node_context = context;

    return context;
}

static void pop_function(morphine_coroutine_t U, struct mc_visitor *V) {
    if (V->function_context == NULL) {
        mapi_error(U, "empty stack of function context");
    }

    struct function_context *context = V->function_context;
    V->function_context = context->prev;

    context->prev = V->trash.function_context;
    V->trash.function_context = context;
}

static void pop_node(morphine_coroutine_t U, struct mc_visitor *V) {
    if (V->node_context == NULL) {
        mapi_error(U, "empty stack of node context");
    }

    struct node_context *context = V->node_context;
    V->node_context = context->prev;

    context->prev = V->trash.node_context;
    V->trash.node_context = context;
}

// controller

MORPHINE_API morphine_noret void mcapi_visitor_node(
    struct mc_visitor_controller *C,
    struct mc_ast_node *node,
    size_t next_state
) {
    C->command.type = COMMAND_NODE;
    C->command.node = node;
    C->command.next_state = next_state;
    longjmp(C->jump, 1);
}

MORPHINE_API morphine_noret void mcapi_visitor_function(
    struct mc_visitor_controller *C,
    struct mc_ast_function *function,
    size_t next_state
) {
    C->command.type = COMMAND_FUNCTION;
    C->command.function = function;
    C->command.next_state = next_state;
    longjmp(C->jump, 1);
}

MORPHINE_API morphine_noret void mcapi_visitor_jump(
    struct mc_visitor_controller *C,
    size_t next_state
) {
    C->command.type = COMMAND_JUMP;
    C->command.next_state = next_state;
    longjmp(C->jump, 1);
}

MORPHINE_API morphine_noret void mcapi_visitor_complete(
    struct mc_visitor_controller *C
) {
    C->command.type = COMMAND_COMPLETE;
    longjmp(C->jump, 1);
}

MORPHINE_API void *mcapi_visitor_data(struct mc_visitor_controller *C) {
    return C->data;
}

MORPHINE_API void *mcapi_visitor_saved(struct mc_visitor_controller *C) {
    struct node_context *context = C->V->node_context;
    if (context->has_allocated_data) {
        return context->allocated_data;
    }

    return NULL;
}

MORPHINE_API void *mcapi_visitor_alloc_saved_uni(
    struct mc_visitor_controller *C, size_t size
) {
    struct node_context *context = C->V->node_context;
    context->allocated_data = mapi_allocator_uni(
        mapi_instance(C->U), context->allocated_data, size
    );
    context->has_allocated_data = true;

    return context->allocated_data;
}

MORPHINE_API void *mcapi_visitor_alloc_saved_vec(
    struct mc_visitor_controller *C, size_t count, size_t size
) {
    struct node_context *context = C->V->node_context;
    context->allocated_data = mapi_allocator_vec(
        mapi_instance(C->U), context->allocated_data, count, size
    );
    context->has_allocated_data = true;

    return context->allocated_data;
}

// api

static void visitor_userdata_constructor(morphine_instance_t I, void *data) {
    (void) I;

    struct mc_visitor *V = data;
    *V = (struct mc_visitor) {
        .first_launch = true,
        .node_context = NULL,
        .function_context = NULL,
        .trash.node_context = NULL,
        .trash.function_context = NULL
    };
}

static void visitor_userdata_free_node_context(
    morphine_instance_t I,
    struct node_context *context
) {
    struct node_context *current = context;
    while (current != NULL) {
        struct node_context *prev = current->prev;

        mapi_allocator_free(I, current->allocated_data);
        mapi_allocator_free(I, current);

        current = prev;
    }
}

static void visitor_userdata_free_function_context(
    morphine_instance_t I,
    struct function_context *context
) {
    struct function_context *current = context;
    while (current != NULL) {
        struct function_context *prev = current->prev;

        mapi_allocator_free(I, current);

        current = prev;
    }
}

static void visitor_userdata_destructor(morphine_instance_t I, void *data) {
    struct mc_visitor *V = data;
    visitor_userdata_free_node_context(I, V->node_context);
    visitor_userdata_free_node_context(I, V->trash.node_context);
    visitor_userdata_free_function_context(I, V->function_context);
    visitor_userdata_free_function_context(I, V->trash.function_context);
}

MORPHINE_API struct mc_visitor *mcapi_push_visitor(morphine_coroutine_t U) {
    mapi_type_declare(
        mapi_instance(U),
        MC_VISITOR_USERDATA_TYPE,
        sizeof(struct mc_visitor),
        false,
        visitor_userdata_constructor,
        visitor_userdata_destructor,
        NULL,
        NULL
    );

    return mapi_push_userdata(U, MC_VISITOR_USERDATA_TYPE);
}

MORPHINE_API struct mc_visitor *mcapi_get_visitor(morphine_coroutine_t U) {
    return mapi_userdata_pointer(U, MC_VISITOR_USERDATA_TYPE);
}

MORPHINE_API bool mcapi_visitor_step(
    morphine_coroutine_t U,
    struct mc_visitor *V,
    struct mc_ast *A,
    mc_visitor_function_t function,
    enum mc_visitor_event *event,
    void *data
) {
    struct mc_visitor_controller controller = {
        .U = U,
        .V = V,
        .data = data
    };

    if (setjmp(controller.jump) != 0) {
        switch (controller.command.type) {
            case COMMAND_NODE: {
                V->node_context->state = controller.command.next_state;
                push_node(U, V, controller.command.node);

                if (event != NULL) {
                    *event = MCVE_ENTER_NODE;
                }
                return true;
            }
            case COMMAND_FUNCTION: {
                struct function_context *current = V->function_context;
                while (current != NULL) {
                    if (current->function == controller.command.function) {
                        mapi_error(U, "ast recursion");
                    }

                    current = current->prev;
                }

                V->node_context->state = controller.command.next_state;
                struct function_context *function_context = push_function(
                    U, V, controller.command.function
                );

                struct node_context *node_context = push_node(
                    U, V, mcapi_ast_statement2node(controller.command.function->body)
                );

                function_context->root_node_context = node_context;

                if (event != NULL) {
                    *event = MCVE_ENTER_FUNCTION;
                }
                return true;
            }
            case COMMAND_JUMP: {
                V->node_context->state = controller.command.next_state;

                if (event != NULL) {
                    *event = MCVE_NEXT_STEP;
                }
                return true;
            }
            case COMMAND_COMPLETE: {
                struct function_context *function_context = V->function_context;
                struct node_context *node_context = V->node_context;
                bool is_root_context = node_context == function_context->root_node_context;

                if (is_root_context) {
                    pop_function(U, V);
                }

                pop_node(U, V);

                if (event != NULL) {
                    if (is_root_context) {
                        *event = MCVE_DROP_FUNCTION;
                    } else {
                        *event = MCVE_DROP_NODE;
                    }
                }
                return true;
            }
        }

        mapi_error(U, "unknown visitor command");
    }

    if (V->first_launch) {
        V->first_launch = false;

        struct mc_ast_function *root = mcapi_ast_get_root_function(A);

        struct function_context *function_context = push_function(U, V, root);
        struct node_context *node_context = push_node(
            U, V, mcapi_ast_statement2node(root->body)
        );

        function_context->root_node_context = node_context;

        if (event != NULL) {
            *event = MCVE_INIT;
        }

        return true;
    } else if (V->node_context == NULL) {
        if (event != NULL) {
            *event = MCVE_COMPLETE;
        }

        return false;
    }

    function(&controller, V->node_context->node, V->node_context->state);

    mapi_error(U, "incomplete visitor");
}
