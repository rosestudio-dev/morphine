//
// Created by why-iskra on 02.06.2024.
//

#include <string.h>
#include "morphinec/codegen.h"
#include "morphinec/ast.h"
#include "morphinec/visitor.h"
#include "codegen/impl.h"
#include "codegen/support/controller.h"
#include "morphinec/stack.h"
#include "morphinec/config.h"

#define MORPHINE_TYPE "codegen"

struct codegen_instruction {
    ml_line line;
    morphine_opcode_t opcode;
    struct codegen_argument argument1;
    struct codegen_argument argument2;
    struct codegen_argument argument3;
};

struct codegen_anchor {
    size_t instruction;
};

struct codegen_variable {
    strtable_index_t index;
    bool mutable;
};

struct codegen_scope {
    size_t start;
    size_t count;
};

struct codegen_temporary {
    size_t size;
};

struct codegen_function {
    struct ast_function *ref;

    struct stack temporary;
    struct stack scopes;
    struct stack variables;
    struct stack anchors;
    struct stack instructions;

    struct codegen_function *prev;
    struct codegen_function *stack_prev;
};

struct codegen {
    morphine_coroutine_t U;

    struct codegen_function *current_function;
    struct codegen_function *functions;
    struct codegen_function *compiled;
};

struct codegen_controller {
    struct codegen *C;
    struct visitor_controller *visitor_controller;
};

struct codegen_save {
    bool has_result;
    struct codegen_argument result;
};

static void prepare_function(struct codegen *C) {
    *stack_push_typed(
        struct codegen_temporary,
        C->U,
        &C->current_function->temporary
    ) = (struct codegen_temporary) {
        .size = 0
    };

    *stack_push_typed(
        struct codegen_scope,
        C->U,
        &C->current_function->scopes
    ) = (struct codegen_scope) {
        .start = 0,
        .count = 0
    };
}

static void visit_node_prepare(
    struct codegen_controller *N,
    bool has_result,
    struct codegen_argument result
) {
    struct codegen_save *save;
    visitor_save(N->visitor_controller, sizeof(struct codegen_save), (void **) &save);
    save->has_result = has_result;
    save->result = result;

    struct codegen_temporary temporary = *stack_peek_typed(
        struct codegen_temporary,
        N->C->U,
        &N->C->current_function->temporary
    );

    *stack_push_typed(
        struct codegen_temporary,
        N->C->U,
        &N->C->current_function->temporary
    ) = temporary;
}

morphine_noret void codegen_error(struct codegen_controller *N, const char *str) {
    mapi_error(N->C->U, str);
}

morphine_noret void codegen_visit_expression(
    struct codegen_controller *N,
    struct expression *expression,
    struct codegen_argument result,
    size_t state
) {
    visit_node_prepare(N, true, result);
    visitor_node(N->visitor_controller, state, ast_as_node(expression));
}

morphine_noret void codegen_visit_statement(
    struct codegen_controller *N,
    struct statement *statement,
    size_t state
) {
    struct codegen_argument argument = {
        .type = CAT_STABLE,
        .stable = 0
    };

    visit_node_prepare(N, false, argument);
    visitor_node(N->visitor_controller, state, ast_as_node(statement));
}

morphine_noret void codegen_visit_function(
    struct codegen_controller *N,
    struct ast_function *function,
    size_t state
) {
    struct codegen_function *cf = N->C->functions;
    while (cf != NULL) {
        if (cf->ref == function) {
            break;
        }

        cf = cf->prev;
    }

    if (cf == NULL) {
        codegen_error(N, "function not found");
    }

    cf->stack_prev = N->C->current_function;
    N->C->current_function = cf;

    prepare_function(N->C);

    visitor_function(N->visitor_controller, state, function);
}

morphine_noret void codegen_visit_next(struct codegen_controller *N, size_t state) {
    visitor_next(N->visitor_controller, state);
}

morphine_noret void codegen_visit_return(struct codegen_controller *N) {
    if (visitor_is_function_root(N->visitor_controller)) {
        struct codegen_function *cf_prev = NULL;
        struct codegen_function *cf = N->C->functions;
        while (cf != NULL) {
            if (cf == N->C->current_function) {
                break;
            }

            cf_prev = cf;
            cf = cf->prev;
        }

        if (cf == NULL) {
            codegen_error(N, "function not found");
        }

        if (cf_prev == NULL) {
            N->C->functions = cf->prev;
        } else {
            cf_prev->prev = cf->prev;
        }

        cf->prev = N->C->compiled;
        N->C->compiled = cf;
    } else {
        stack_pop(N->C->U, &N->C->current_function->temporary, 1);
    }

    visitor_return(N->visitor_controller);
}

struct codegen_argument codegen_anchor(struct codegen_controller *N) {
    size_t pos = stack_size(N->C->current_function->instructions);
    size_t anchor = stack_size(N->C->current_function->anchors);

    *stack_push_typed(
        struct codegen_anchor,
        N->C->U,
        &N->C->current_function->anchors
    ) = (struct codegen_anchor) {
        .instruction = pos
    };

    return (struct codegen_argument) {
        .type = CAT_ANCHOR,
        .anchor = anchor
    };
}

struct codegen_argument codegen_temporary(struct codegen_controller *N) {
    struct codegen_temporary *temporary = stack_peek_typed(
        struct codegen_temporary,
        N->C->U,
        &N->C->current_function->temporary
    );

    if (temporary->size >= CODEGEN_TEMPORARY_LIMIT) {
        codegen_error(N, "temporary slots limit reached");
    }

    struct codegen_argument result = {
        .type = CAT_TEMPORARY,
        .temporary = temporary->size
    };

    temporary->size++;

    return result;
}

void codegen_scope_enter(struct codegen_controller *N) {
    struct codegen_scope *scope = stack_peek_typed(
        struct codegen_scope,
        N->C->U,
        &N->C->current_function->scopes
    );

    *stack_push_typed(
        struct codegen_scope,
        N->C->U,
        &N->C->current_function->scopes
    ) = (struct codegen_scope) {
        .start = scope->start + scope->count,
        .count = 0
    };
}

void codegen_scope_exit(struct codegen_controller *N) {
    stack_pop(N->C->U, &N->C->current_function->scopes, 1);
}

//void codegen_declare_variable(struct codegen_controller *N);
//void codegen_get_variable(struct codegen_controller *N);

void codegen_instruction(
    struct codegen_controller *N,
    morphine_opcode_t opcode,
    struct codegen_argument a1,
    struct codegen_argument a2,
    struct codegen_argument a3
) {
    *stack_push_typed(
        struct codegen_instruction,
        N->C->U,
        &N->C->current_function->instructions
    ) = (struct codegen_instruction) {
        .opcode = opcode,
        .argument1 = a1,
        .argument2 = a2,
        .argument3 = a3,
        .line = visitor_line(N->visitor_controller)
    };
}

static void visit(struct visitor_controller *visitor_controller, struct ast_node *node, size_t state) {
    struct codegen_controller controller = {
        .C = visitor_data(visitor_controller),
        .visitor_controller = visitor_controller
    };

    (void) controller;
    (void) state;
    switch (ast_node_type(node)) {
        case AST_NODE_TYPE_EXPRESSION:
//            gen_expression(&controller, ast_as_expression(node), state);
            break;
        case AST_NODE_TYPE_STATEMENT:
//            gen_statement(&controller, ast_as_statement(node), state);
            break;
    }
}

static void codegen_free_functions(morphine_instance_t I, struct codegen_function *functions) {
    struct codegen_function *function = functions;
    while (function != NULL) {
        struct codegen_function *prev = function->prev;

        stack_free(I, &function->instructions);
        stack_free(I, &function->anchors);
        stack_free(I, &function->temporary);
        stack_free(I, &function->variables);
        stack_free(I, &function->scopes);

        mapi_allocator_free(I, function);

        function = prev;
    }
}

static void codegen_free(morphine_instance_t I, void *p) {
    struct codegen *C = p;

    codegen_free_functions(I, C->functions);
    codegen_free_functions(I, C->compiled);
}

static struct codegen *get_codegen(morphine_coroutine_t U) {
    mapi_peek(U, 1);
    if (strcmp(mapi_userdata_type(U), MORPHINE_TYPE) == 0) {
        struct codegen *C = mapi_userdata_pointer(U);
        mapi_pop(U, 1);
        return C;
    } else {
        mapi_error(U, "expected "MORPHINE_TYPE);
    }
}

void codegen(morphine_coroutine_t U) {
    struct codegen *C = mapi_push_userdata(U, MORPHINE_TYPE, sizeof(struct codegen));

    *C = (struct codegen) {
        .U = U,
        .current_function = NULL,
        .functions = NULL,
        .compiled = NULL
    };

    mapi_userdata_set_free(U, codegen_free);

    mapi_rotate(U, 2);
    struct ast_function *functions = ast_functions(U);
    mapi_rotate(U, 2);

    while (functions != NULL) {
        struct codegen_function *cf = mapi_allocator_uni(
            mapi_instance(U),
            NULL,
            sizeof(struct codegen_function)
        );

        *cf = (struct codegen_function) {
            .ref = functions,
            .stack_prev = NULL
        };

        stack_init(
            &cf->instructions,
            sizeof(struct codegen_instruction),
            CODEGEN_STACK_EXPANSION_FACTOR,
            CODEGEN_LIMIT_STACK_INSTRUCTIONS
        );

        stack_init(
            &cf->scopes,
            sizeof(struct codegen_scope),
            CODEGEN_STACK_EXPANSION_FACTOR,
            CODEGEN_LIMIT_STACK_SCOPES
        );

        stack_init(
            &cf->variables,
            sizeof(struct codegen_variable),
            CODEGEN_STACK_EXPANSION_FACTOR,
            CODEGEN_LIMIT_STACK_VARIABLES
        );

        stack_init(
            &cf->anchors,
            sizeof(struct codegen_anchor),
            CODEGEN_STACK_EXPANSION_FACTOR,
            CODEGEN_LIMIT_STACK_ANCHORS
        );

        stack_init(
            &cf->temporary,
            sizeof(struct codegen_temporary),
            CODEGEN_LIMIT_STACK_TEMPORARY,
            CODEGEN_LIMIT_STACK_ANCHORS
        );

        cf->prev = C->functions;
        C->functions = cf;

        functions = functions->prev;
    }

    C->current_function = C->functions;
    prepare_function(C);

    mapi_rotate(U, 2);
    visitor(U, visit, C);
    mapi_rotate(U, 2);
    mapi_rotate(U, 3);
}

bool codegen_step(morphine_coroutine_t U) {
    struct codegen *C = get_codegen(U);
    C->U = U;

    return visitor_step(U, NULL);
}
