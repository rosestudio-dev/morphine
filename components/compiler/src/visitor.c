//
// Created by why-iskra on 03.06.2024.
//

#include <string.h>
#include <setjmp.h>
#include "morphinec/visitor.h"
#include "morphinec/stack.h"
#include "morphinec/config.h"

enum jump_type {
    JT_VISIT_NODE,
    JT_VISIT_FUNCTION,
    JT_NEXT,
    JT_RETURN,
};

struct context {
    struct ast_node *node;
    bool function_root;
    size_t state;
    void *save;
};

define_stack(function, struct ast_function *)
define_stack_push(function, struct ast_function *)
define_stack(context, struct context)

define_stack_push(context, struct context)
define_stack_pop(context, struct context)
define_stack_peek(context, struct context)
define_stack_get(context, struct context)

struct visitor {
    bool inited;

    void *data;
    visit_func_t visit_func;

    struct stack_function function_stack;
    struct stack_context context_stack;
};

struct visitor_controller {
    morphine_coroutine_t U;
    struct visitor *V;
    jmp_buf jump;
    struct context *context;
    void *prev_save;

    size_t next_state;
    enum jump_type jump_type;
    union {
        struct ast_node *node;
        struct ast_function *function;
    };
};

bool visitor_has_saved(struct visitor_controller *C) {
    return C->context->save != NULL;
}

bool visitor_save(struct visitor_controller *C, size_t size, void **save) {
    if (C->context->save == NULL) {
        C->context->save = mapi_allocator_uni(mapi_instance(C->U), NULL, size);
        *save = C->context->save;
        return true;
    } else {
        *save = C->context->save;
        return false;
    }
}

void *visitor_prev_save(struct visitor_controller *C) {
    return C->prev_save;
}

void *visitor_data(struct visitor_controller *C) {
    return C->V->data;
}

ml_line visitor_line(struct visitor_controller *C) {
    return ast_node_line(C->context->node);
}

bool visitor_is_function_root(struct visitor_controller *C) {
    return C->context->function_root;
}

morphine_noret void visitor_next(struct visitor_controller *C, size_t state) {
    C->jump_type = JT_NEXT;
    C->next_state = state;
    longjmp(C->jump, 1);
}

morphine_noret void visitor_node(
    struct visitor_controller *C,
    size_t state,
    struct ast_node *node
) {
    C->jump_type = JT_VISIT_NODE;
    C->next_state = state;
    C->node = node;
    longjmp(C->jump, 1);
}

morphine_noret void visitor_function(
    struct visitor_controller *C,
    size_t state,
    struct ast_function *function
) {
    C->jump_type = JT_VISIT_FUNCTION;
    C->next_state = state;
    C->function = function;
    longjmp(C->jump, 1);
}

morphine_noret void visitor_return(struct visitor_controller *C) {
    C->jump_type = JT_RETURN;
    longjmp(C->jump, 1);
}

static void visitor_free(morphine_instance_t I, void *p) {
    struct visitor *V = p;

    stack_iterator(V->context_stack, index) {
        struct context *context = stack_it(V->context_stack, index);
        mapi_allocator_free(I, context->save);
    }

    stack_function_free(I, &V->function_stack);
    stack_context_free(I, &V->context_stack);
}

struct visitor *visitor(morphine_coroutine_t U, struct ast *A) {
    struct ast_function *functions = ast_functions(A);
    if (functions == NULL) {
        mapi_error(U, "empty ast");
    }

    struct visitor *V = mapi_push_userdata_uni(U, sizeof(struct visitor));

    *V = (struct visitor) {
        .inited = false,
        .visit_func = NULL,
        .data = NULL,
    };

    stack_context_init(
        &V->context_stack,
        VISITOR_STACK_EXPANSION_FACTOR,
        VISITOR_LIMIT_STACK_FUNCTIONS
    );

    stack_function_init(
        &V->function_stack,
        VISITOR_STACK_EXPANSION_FACTOR,
        VISITOR_LIMIT_STACK_CONTEXTS
    );

    mapi_userdata_set_free(U, visitor_free);

    *stack_function_push(U, &V->function_stack) = functions;
    *stack_context_push(U, &V->context_stack) = (struct context) {
        .save = NULL,
        .function_root = true,
        .node = ast_as_node(functions->body),
        .state = 0
    };

    return V;
}

struct visitor *get_visitor(morphine_coroutine_t U) {
    return mapi_userdata_pointer(U, NULL);
}

void visitor_setup(morphine_coroutine_t U, struct visitor *V, visit_func_t func, void *data) {
    if (V->inited) {
        mapi_error(U, "visitor is already inited");
    }

    V->visit_func = func;
    V->data = data;
    V->inited = true;
}

bool visitor_step(morphine_coroutine_t U, struct visitor *V, void *save) {
    if (!V->inited) {
        mapi_error(U, "visitor isn't inited");
    }

    if (stack_size(V->context_stack) == 0) {
        return false;
    }

    struct context *context = stack_context_peek(U, &V->context_stack);

    struct visitor_controller controller = {
        .U = U,
        .V = V,
        .context = context
    };

    if (stack_size(V->context_stack) > 1) {
        struct context prev_context = *stack_context_get(
            U,
            &V->context_stack,
            stack_size(V->context_stack) - 2
        );

        controller.prev_save = prev_context.save;
    } else {
        controller.prev_save = save;
    }

    if (setjmp(controller.jump) != 0) {
        switch (controller.jump_type) {
            case JT_VISIT_NODE: {
                context->state = controller.next_state;
                *stack_context_push(U, &V->context_stack) = (struct context) {
                    .save = NULL,
                    .function_root = false,
                    .node = ast_as_node(controller.node),
                    .state = 0
                };
                break;
            }
            case JT_VISIT_FUNCTION: {
                stack_iterator(V->function_stack, index) {
                    struct ast_function *fun = *stack_it(V->function_stack, index);
                    if (fun == controller.function) {
                        mapi_error(U, "ast recursion");
                    }
                }

                context->state = controller.next_state;
                *stack_function_push(U, &V->function_stack) = controller.function;
                *stack_context_push(U, &V->context_stack) = (struct context) {
                    .save = NULL,
                    .function_root = true,
                    .node = ast_as_node(controller.function->body),
                    .state = 0
                };
                break;
            }
            case JT_NEXT: {
                context->state = controller.next_state;
                break;
            }
            case JT_RETURN: {
                stack_context_pop(U, &V->context_stack, 1);
                mapi_allocator_free(mapi_instance(U), context->save);
                break;
            }
            default:
                mapi_error(U, "unknown visitor jump");
        }

        return true;
    }

    V->visit_func(&controller, context->node, context->state);
    mapi_error(U, "incomplete visitor");
}
