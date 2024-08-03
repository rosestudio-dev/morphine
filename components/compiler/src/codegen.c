//
// Created by why-iskra on 02.06.2024.
//

#include <string.h>
#include "morphinec/codegen.h"
#include "morphinec/ast.h"
#include "morphinec/visitor.h"
#include "morphinec/stack.h"
#include "morphinec/config.h"
#include "codegen/impl.h"
#include "codegen/support/controller.h"

enum codegen_constant_type {
    CCT_NIL,
    CCT_INTEGER,
    CCT_DECIMAL,
    CCT_STRING,
    CCT_BOOLEAN,
    CCT_FUNCTION
};

struct codegen_constant {
    enum codegen_constant_type type;
    union {
        ml_integer integer;
        ml_decimal decimal;
        morphinec_strtable_index_t string;
        bool boolean;
        struct ast_function *function;
    } value;
};

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
    morphinec_strtable_index_t index;
    bool mutable;
};

struct codegen_scope {
    size_t count;

    bool break_continue_inited;
    struct codegen_argument_anchor break_anchor;
    struct codegen_argument_anchor continue_anchor;
};

struct codegen_temporary {
    size_t size;
};

define_stack(temporary, struct codegen_temporary)
define_stack_push(temporary, struct codegen_temporary)
define_stack_pop(temporary, struct codegen_temporary)
define_stack_peek(temporary, struct codegen_temporary)

define_stack(scope, struct codegen_scope)
define_stack_push(scope, struct codegen_scope)
define_stack_pop(scope, struct codegen_scope)
define_stack_peek(scope, struct codegen_scope)

define_stack(variable, struct codegen_variable)
define_stack_push(variable, struct codegen_variable)
define_stack_pop(variable, struct codegen_variable)

define_stack(anchor, struct codegen_anchor)
define_stack_push(anchor, struct codegen_anchor)
define_stack_get(anchor, struct codegen_anchor)

define_stack(instruction, struct codegen_instruction)
define_stack_push(instruction, struct codegen_instruction)

define_stack(constant, struct codegen_constant)
define_stack_push(constant, struct codegen_constant)

define_stack(closure, struct codegen_closure)
define_stack_push(closure, struct codegen_closure)
define_stack_get(closure, struct codegen_closure)

struct codegen_function {
    struct ast_function *ref;

    bool process;
    bool compiled;

    struct stack_temporary temporary_stack;
    struct stack_scope scopes_stack;
    struct stack_variable variables_stack;
    struct stack_anchor anchors_stack;
    struct stack_instruction instructions_stack;
    struct stack_constant constants_stack;
    struct stack_closure closures_stack;

    struct codegen_function *prev;
    struct codegen_function *stack_prev;
    struct codegen_function *stack_next;
};

struct codegen {
    morphine_coroutine_t U;
    struct morphinec_strtable *T;
    struct visitor *V;

    struct codegen_function *current_function;
    struct codegen_function *functions;
};

struct codegen_controller {
    struct codegen *C;
    struct visitor_controller *visitor_controller;
};

struct codegen_save {
    bool has_result;
    struct codegen_argument_slot result;
};

static void visit_node_prepare(
    struct codegen_controller *N,
    bool has_result,
    struct codegen_argument_slot result
) {
    struct codegen_save *save;
    visitor_save(N->visitor_controller, sizeof(struct codegen_save), (void **) &save);
    save->has_result = has_result;
    save->result = result;

    struct codegen_temporary temporary = *stack_temporary_peek(
        N->C->U,
        &N->C->current_function->temporary_stack
    );

    *stack_temporary_push(
        N->C->U,
        &N->C->current_function->temporary_stack
    ) = temporary;
}

static size_t add_constant(struct codegen_controller *N, struct codegen_constant constant) {
    stack_iterator(N->C->current_function->constants_stack, index) {
        struct codegen_constant *cnst = stack_it(
            N->C->current_function->constants_stack, index
        );

        if (constant.type == cnst->type) {
            switch (constant.type) {
                case CCT_NIL:
                    return index;
                case CCT_INTEGER:
                    if (constant.value.integer == cnst->value.integer) {
                        return index;
                    }
                    break;
                case CCT_DECIMAL:
                    if (constant.value.decimal == cnst->value.decimal) {
                        return index;
                    }
                    break;
                case CCT_STRING:
                    if (constant.value.string == cnst->value.string) {
                        return index;
                    }
                    break;
                case CCT_BOOLEAN:
                    if (constant.value.boolean == cnst->value.boolean) {
                        return index;
                    }
                    break;
                case CCT_FUNCTION:
                    if (constant.value.function == cnst->value.function) {
                        return index;
                    }
                    break;
            }
        }
    }

    size_t result = stack_size(N->C->current_function->constants_stack);

    *stack_constant_push(
        N->C->U,
        &N->C->current_function->constants_stack
    ) = constant;

    return result;
}

static struct codegen_variable_info get_variable(
    morphinec_strtable_index_t index,
    struct codegen_function *cf
) {
    stack_iterator(cf->variables_stack, i) {
        struct codegen_variable *variable = stack_it_invert(cf->variables_stack, i);

        if (variable->index == index) {
            return (struct codegen_variable_info) {
                .type = CVT_VARIABLE,
                .mutable = variable->mutable,
                .variable = (struct codegen_argument_slot) {
                    .is_variable = true,
                    .index = stack_idx_invert(cf->variables_stack, i)
                }
            };
        }
    }

    for (size_t i = 0; i < cf->ref->statics_size; i++) {
        if (cf->ref->statics[i] == index) {
            return (struct codegen_variable_info) {
                .type = CVT_STATIC,
                .mutable = true,
                .static_variable = (struct codegen_argument_index) {
                    .index = i
                }
            };
        }
    }

    for (size_t i = 0; i < cf->ref->args_size; i++) {
        if (cf->ref->arguments[i] == index) {
            return (struct codegen_variable_info) {
                .type = CVT_ARGUMENT,
                .mutable = true,
                .argument = (struct codegen_argument_index) {
                    .index = i
                }
            };
        }
    }

    if (cf->ref->recursive && !cf->ref->anonymous && cf->ref->name == index) {
        return (struct codegen_variable_info) {
            .type = CVT_RECURSION,
            .mutable = false,
        };
    }

    stack_iterator(cf->closures_stack, i) {
        struct codegen_closure *closure = stack_it(cf->closures_stack, i);

        if (closure->index == index) {
            return (struct codegen_variable_info) {
                .type = CVT_CLOSURE,
                .mutable = closure->mutable,
                .closure = (struct codegen_argument_index) {
                    .index = i
                }
            };
        }
    }

    return (struct codegen_variable_info) {
        .type = CVT_NOT_FOUND,
        .mutable = true
    };
}

static struct codegen_variable_info deep_variable(
    struct codegen *C,
    morphinec_strtable_index_t index,
    struct codegen_function *head
) {
    struct codegen_function *cf = head;
    struct codegen_variable_info info;
    while (cf != NULL) {
        info = get_variable(index, cf);

        if (info.type != CVT_NOT_FOUND) {
            break;
        } else if (!cf->ref->auto_closure) {
            return (struct codegen_variable_info) {
                .type = CVT_NOT_FOUND,
                .mutable = true
            };
        }

        cf = cf->stack_prev;
    }

    if (cf == NULL) {
        return (struct codegen_variable_info) {
            .type = CVT_NOT_FOUND,
            .mutable = true
        };
    } else if (cf == head) {
        return info;
    }

    do {
        cf = cf->stack_next;

        *stack_closure_push(
            C->U,
            &cf->closures_stack
        ) = (struct codegen_closure) {
            .index = index,
            .mutable = info.mutable
        };
    } while (cf != head);

    return (struct codegen_variable_info) {
        .type = CVT_CLOSURE,
        .mutable = info.mutable,
        .closure = (struct codegen_argument_index) {
            .index = stack_size(cf->closures_stack) - 1
        }
    };
}

static void enter_function(struct codegen *C, struct codegen_controller *N, struct codegen_function *cf) {
    if (C->current_function != NULL) {
        C->current_function->stack_next = cf;
    }

    cf->stack_prev = C->current_function;
    C->current_function = cf;

    if (cf->process) {
        if (N != NULL) {
            codegen_error(N, "codegen recursion");
        } else {
            mapi_error(C->U, "codegen recursion");
        }
    }

    cf->process = true;

    *stack_temporary_push(
        C->U,
        &cf->temporary_stack
    ) = (struct codegen_temporary) {
        .size = 0
    };

    *stack_scope_push(
        C->U,
        &cf->scopes_stack
    ) = (struct codegen_scope) {
        .count = 0
    };

    for (size_t i = 0; i < cf->ref->args_size; i++) {
        morphinec_strtable_index_t arg = cf->ref->arguments[i];
        size_t count = 0;
        for (size_t j = 0; j < cf->ref->args_size; j++) {
            if (arg == cf->ref->arguments[j]) {
                count++;
            }
        }

        if (count > 1) {
            if (N != NULL) {
                codegen_error(N, "arguments duplicates");
            } else {
                mapi_error(C->U, "arguments duplicates");
            }
        }
    }

    for (size_t i = 0; i < cf->ref->statics_size; i++) {
        morphinec_strtable_index_t static_idx = cf->ref->statics[i];
        size_t count = 0;
        for (size_t j = 0; j < cf->ref->statics_size; j++) {
            if (static_idx == cf->ref->statics[j]) {
                count++;
            }
        }

        if (count > 1) {
            if (N != NULL) {
                codegen_error(N, "static duplicates");
            } else {
                mapi_error(C->U, "static duplicates");
            }
        }
    }

    for (size_t i = 0; i < cf->ref->closures_size; i++) {
        morphinec_strtable_index_t closure = cf->ref->closures[i];
        size_t count = 0;
        for (size_t j = 0; j < cf->ref->closures_size; j++) {
            if (closure == cf->ref->closures[j]) {
                count++;
            }
        }

        if (count > 1) {
            if (N != NULL) {
                codegen_error(N, "closure duplicates");
            } else {
                mapi_error(C->U, "closure duplicates");
            }
        }

        struct codegen_variable_info info = deep_variable(C, closure, cf->stack_prev);

        if (info.type == CVT_NOT_FOUND) {
            if (N != NULL) {
                codegen_error(N, "closure not found");
            } else {
                mapi_error(C->U, "closure not found");
            }
        } else {
            *stack_closure_push(
                C->U,
                &cf->closures_stack
            ) = (struct codegen_closure) {
                .index = closure,
                .mutable = info.mutable
            };
        }
    }
}

static void exit_function(struct codegen *C) {
    C->current_function = C->current_function->stack_prev;
    if (C->current_function != NULL) {
        C->current_function->stack_next = NULL;
    }
}

static struct codegen_scope *get_anchor_scope(struct codegen_controller *N) {
    struct stack_scope stack = N->C->current_function->scopes_stack;
    stack_iterator(stack, index) {
        struct codegen_scope *scope = stack_it_invert(stack, index);

        if (scope->break_continue_inited) {
            return scope;
        }
    }

    codegen_error(N, "block anchors aren't initialized");
}

morphine_coroutine_t codegen_U(struct codegen_controller *N) {
    return N->C->U;
}

morphine_noret void codegen_error(struct codegen_controller *N, const char *str) {
    mapi_errorf(N->C->U, "line %"MLIMIT_LINE_PR": %s", visitor_line(N->visitor_controller), str);
}

morphine_noret void codegen_errorf(struct codegen_controller *N, const char *str, ...) {
    va_list args;
    va_start(args, str);
    mapi_push_stringv(N->C->U, str, args);
    va_end(args);

    mapi_errorf(
        N->C->U,
        "line %"MLIMIT_LINE_PR": %s",
        visitor_line(N->visitor_controller),
        mapi_get_string(N->C->U)
    );
}

struct morphinec_strtable_entry codegen_string(
    struct codegen_controller *N,
    morphinec_strtable_index_t index
) {
    return mcapi_strtable_access(N->C->U, N->C->T, index);
}

bool codegen_save(struct codegen_controller *N, size_t size, void **container) {
    void *save;
    bool result = visitor_save(N->visitor_controller, sizeof(struct codegen_save) + size, &save);

    save += sizeof(struct codegen_save);
    *container = save;

    return result;
}

morphine_noret void codegen_visit_expression(
    struct codegen_controller *N,
    struct expression *expression,
    struct codegen_argument_slot result,
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
    struct codegen_argument_slot result = { .is_variable = false, .index = 0 };

    visit_node_prepare(N, false, result);
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

    enter_function(N->C, N, cf);

    visitor_function(N->visitor_controller, state, function);
}

morphine_noret void codegen_visit_next(struct codegen_controller *N, size_t state) {
    visitor_next(N->visitor_controller, state);
}

morphine_noret void codegen_visit_return(struct codegen_controller *N) {
    if (visitor_is_function_root(N->visitor_controller)) {
        N->C->current_function->compiled = true;
        exit_function(N->C);
    } else {
        stack_temporary_pop(N->C->U, &N->C->current_function->temporary_stack, 1);
    }

    visitor_return(N->visitor_controller);
}

struct codegen_argument_slot codegen_result(struct codegen_controller *N) {
    struct codegen_save *save = visitor_prev_save(N->visitor_controller);

    if (save == NULL || !save->has_result) {
        codegen_error(N, "cannot get result");
    }

    return save->result;
}

struct codegen_argument_anchor codegen_anchor(struct codegen_controller *N) {
    size_t pos = stack_size(N->C->current_function->instructions_stack);
    size_t anchor = stack_size(N->C->current_function->anchors_stack);

    *stack_anchor_push(
        N->C->U,
        &N->C->current_function->anchors_stack
    ) = (struct codegen_anchor) {
        .instruction = pos
    };

    return (struct codegen_argument_anchor) {
        .index = anchor
    };
}

void codegen_anchor_change(
    struct codegen_controller *N,
    struct codegen_argument_anchor anchor
) {
    stack_anchor_get(
        N->C->U,
        &N->C->current_function->anchors_stack,
        anchor.index
    )->instruction = stack_size(N->C->current_function->instructions_stack);
}

void codegen_init_break_continue(struct codegen_controller *N) {
    struct codegen_scope *scope = stack_scope_peek(
        N->C->U, &N->C->current_function->scopes_stack
    );

    if (scope->break_continue_inited) {
        codegen_error(N, "block anchors are already initialized");
    }

    struct codegen_argument_anchor break_anchor = codegen_anchor(N);
    struct codegen_argument_anchor continue_anchor = codegen_anchor(N);

    scope->break_anchor = break_anchor;
    scope->continue_anchor = continue_anchor;
    scope->break_continue_inited = true;
}

void codegen_break_change(struct codegen_controller *N) {
    codegen_anchor_change(N, get_anchor_scope(N)->break_anchor);
}

void codegen_continue_change(struct codegen_controller *N) {
    codegen_anchor_change(N, get_anchor_scope(N)->continue_anchor);
}

struct codegen_argument_anchor codegen_break_get(struct codegen_controller *N) {
    return get_anchor_scope(N)->break_anchor;
}

struct codegen_argument_anchor codegen_continue_get(struct codegen_controller *N) {
    return get_anchor_scope(N)->continue_anchor;
}

struct codegen_argument_slot codegen_temporary(struct codegen_controller *N) {
    struct codegen_temporary *temporary = stack_temporary_peek(
        N->C->U, &N->C->current_function->temporary_stack
    );

    if (temporary->size >= CODEGEN_TEMPORARY_LIMIT) {
        codegen_error(N, "temporary slots limit reached");
    }

    struct codegen_argument_slot result = {
        .is_variable = false,
        .index = temporary->size
    };

    temporary->size++;

    return result;
}

struct codegen_argument_index codegen_constant_nil(struct codegen_controller *N) {
    struct codegen_constant constant = {
        .type = CCT_NIL
    };

    size_t index = add_constant(N, constant);
    return (struct codegen_argument_index) {
        .index = index
    };
}

struct codegen_argument_index codegen_constant_int(struct codegen_controller *N, ml_integer integer) {
    struct codegen_constant constant = {
        .type = CCT_INTEGER,
        .value.integer = integer
    };

    size_t index = add_constant(N, constant);
    return (struct codegen_argument_index) {
        .index = index
    };
}

struct codegen_argument_index codegen_constant_dec(struct codegen_controller *N, ml_decimal decimal) {
    struct codegen_constant constant = {
        .type = CCT_DECIMAL,
        .value.decimal = decimal
    };

    size_t index = add_constant(N, constant);
    return (struct codegen_argument_index) {
        .index = index
    };
}

struct codegen_argument_index codegen_constant_str(
    struct codegen_controller *N,
    morphinec_strtable_index_t str
) {
    struct codegen_constant constant = {
        .type = CCT_STRING,
        .value.string = str
    };

    size_t index = add_constant(N, constant);
    return (struct codegen_argument_index) {
        .index = index
    };
}

struct codegen_argument_index codegen_constant_cstr(struct codegen_controller *N, const char *str) {
    morphinec_strtable_index_t str_index = mcapi_strtable_record(N->C->U, N->C->T, str, strlen(str));

    struct codegen_constant constant = {
        .type = CCT_STRING,
        .value.string = str_index
    };

    size_t index = add_constant(N, constant);
    return (struct codegen_argument_index) {
        .index = index
    };
}

struct codegen_argument_index codegen_constant_bool(struct codegen_controller *N, bool boolean) {
    struct codegen_constant constant = {
        .type = CCT_BOOLEAN,
        .value.boolean = boolean
    };

    size_t index = add_constant(N, constant);
    return (struct codegen_argument_index) {
        .index = index
    };
}

struct codegen_argument_index codegen_constant_fun(struct codegen_controller *N, struct ast_function *fun) {
    struct codegen_constant constant = {
        .type = CCT_FUNCTION,
        .value.function = fun
    };

    size_t index = add_constant(N, constant);
    return (struct codegen_argument_index) {
        .index = index
    };
}

void codegen_scope_enter(struct codegen_controller *N) {
    *stack_scope_push(
        N->C->U, &N->C->current_function->scopes_stack
    ) = (struct codegen_scope) {
        .count = 0,
        .break_continue_inited = false
    };
}

void codegen_scope_exit(struct codegen_controller *N) {
    struct codegen_scope *scope = stack_scope_peek(
        N->C->U, &N->C->current_function->scopes_stack
    );

    stack_variable_pop(N->C->U, &N->C->current_function->variables_stack, scope->count);
    stack_scope_pop(N->C->U, &N->C->current_function->scopes_stack, 1);
}

void codegen_closure(
    struct codegen_controller *N,
    struct ast_function *function,
    size_t *size,
    struct codegen_closure **closures
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

    *size = stack_size(cf->closures_stack);

    if (*size == 0) {
        *closures = NULL;
    } else {
        *closures = stack_closure_get(N->C->U, &cf->closures_stack, 0);
    }
}

void codegen_declare_variable(
    struct codegen_controller *N,
    morphinec_strtable_index_t index,
    bool mutable
) {
    struct codegen_scope *scope = stack_scope_peek(
        N->C->U, &N->C->current_function->scopes_stack
    );

    *stack_variable_push(
        N->C->U, &N->C->current_function->variables_stack
    ) = (struct codegen_variable) {
        .index = index,
        .mutable = mutable
    };

    scope->count++;
}

struct codegen_variable_info codegen_get_variable(
    struct codegen_controller *N,
    morphinec_strtable_index_t index
) {
    return deep_variable(N->C, index, N->C->current_function);
}

void codegen_instruction(
    struct codegen_controller *N,
    morphine_opcode_t opcode,
    struct codegen_argument a1,
    struct codegen_argument a2,
    struct codegen_argument a3
) {
    *stack_instruction_push(
        N->C->U, &N->C->current_function->instructions_stack
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

    switch (ast_node_type(node)) {
        case AST_NODE_TYPE_EXPRESSION:
            gen_expression(&controller, ast_as_expression(node), state);
            break;
        case AST_NODE_TYPE_STATEMENT:
            gen_statement(&controller, ast_as_statement(node), state);
            break;
    }
}

static void codegen_free(morphine_instance_t I, void *p) {
    struct codegen *C = p;

    struct codegen_function *function = C->functions;
    while (function != NULL) {
        struct codegen_function *prev = function->prev;

        stack_instruction_free(I, &function->instructions_stack);
        stack_anchor_free(I, &function->anchors_stack);
        stack_temporary_free(I, &function->temporary_stack);
        stack_variable_free(I, &function->variables_stack);
        stack_scope_free(I, &function->scopes_stack);
        stack_constant_free(I, &function->constants_stack);
        stack_closure_free(I, &function->closures_stack);

        mapi_allocator_free(I, function);

        function = prev;
    }
}

struct codegen *codegen(
    morphine_coroutine_t U,
    struct morphinec_strtable *T,
    struct ast *A,
    struct visitor *V
) {
    struct codegen *C = mapi_push_userdata_uni(U, sizeof(struct codegen));

    *C = (struct codegen) {
        .U = U,
        .V = V,
        .T = T,
        .current_function = NULL,
        .functions = NULL
    };

    mapi_userdata_set_free(U, codegen_free);

    struct codegen_function *main = NULL;

    struct ast_function *functions = ast_functions(A);
    while (functions != NULL) {
        struct codegen_function *cf = mapi_allocator_uni(
            mapi_instance(U),
            NULL,
            sizeof(struct codegen_function)
        );

        *cf = (struct codegen_function) {
            .ref = functions,
            .process = false,
            .compiled = false,
            .stack_prev = NULL,
            .stack_next = NULL
        };

        stack_instruction_init(
            &cf->instructions_stack,
            CODEGEN_STACK_EXPANSION_FACTOR,
            CODEGEN_LIMIT_STACK_INSTRUCTIONS
        );

        stack_scope_init(
            &cf->scopes_stack,
            CODEGEN_STACK_EXPANSION_FACTOR,
            CODEGEN_LIMIT_STACK_SCOPES
        );

        stack_variable_init(
            &cf->variables_stack,
            CODEGEN_STACK_EXPANSION_FACTOR,
            CODEGEN_LIMIT_STACK_VARIABLES
        );

        stack_anchor_init(
            &cf->anchors_stack,
            CODEGEN_STACK_EXPANSION_FACTOR,
            CODEGEN_LIMIT_STACK_ANCHORS
        );

        stack_temporary_init(
            &cf->temporary_stack,
            CODEGEN_STACK_EXPANSION_FACTOR,
            CODEGEN_LIMIT_STACK_TEMPORARY
        );

        stack_constant_init(
            &cf->constants_stack,
            CODEGEN_STACK_EXPANSION_FACTOR,
            CODEGEN_LIMIT_STACK_CONSTANTS
        );

        stack_closure_init(
            &cf->closures_stack,
            CODEGEN_STACK_EXPANSION_FACTOR,
            CODEGEN_LIMIT_STACK_CLOSURES
        );

        cf->prev = C->functions;
        C->functions = cf;

        if (main == NULL) {
            main = cf;
        }

        functions = functions->prev;
    }

    if (C->functions == NULL || main == NULL) {
        mapi_error(U, "empty ast");
    }

    enter_function(C, NULL, main);

    visitor_setup(U, V, visit, C);
    return C;
}

struct codegen *get_codegen(morphine_coroutine_t U) {
    return mapi_userdata_pointer(U, NULL);
}

bool codegen_step(morphine_coroutine_t U, struct codegen *C) {
    C->U = U;

    if (C->current_function == NULL) {
        return false;
    }

    return visitor_step(U, C->V, NULL);
}

static inline void update_slot_argument(
    struct codegen_argument argument,
    size_t *variables,
    size_t *temporaries
) {
    if (argument.type == CAT_slot) {
        size_t *update;
        if (argument.value.slot.is_variable) {
            update = variables;
        } else {
            update = temporaries;
        }

        if (*update < argument.value.slot.index + 1) {
            *update = argument.value.slot.index + 1;
        }
    }
}

static inline ml_argument argument_normalize(
    morphine_coroutine_t U,
    struct codegen_function *function,
    struct codegen_argument argument,
    size_t variables
) {
    switch (argument.type) {
        case CAT_stub:
            return 0;
        case CAT_index:
            if (argument.value.index.index > MLIMIT_ARGUMENT_MAX) {
                mapi_error(U, "index too big");
            }
            return (ml_argument) argument.value.index.index;
        case CAT_count:
            if (argument.value.count.count > MLIMIT_ARGUMENT_MAX) {
                mapi_error(U, "count too big");
            }
            return (ml_argument) argument.value.count.count;
        case CAT_slot: {
            size_t slot;
            if (argument.value.slot.is_variable) {
                slot = argument.value.slot.index;
            } else {
                slot = argument.value.slot.index + variables;
            }

            if (slot > MLIMIT_ARGUMENT_MAX) {
                mapi_error(U, "slot too big");
            }

            return (ml_argument) slot;
        }
        case CAT_anchor: {
            struct codegen_anchor anchor = *stack_anchor_get(
                U, &function->anchors_stack, argument.value.anchor.index
            );

            if (anchor.instruction > MLIMIT_ARGUMENT_MAX) {
                mapi_error(U, "position too big");
            }

            return (ml_argument) anchor.instruction;
        }
    }

    mapi_error(U, "undefined argument");
}

static inline void load_instructions(
    morphine_coroutine_t U,
    struct codegen_function *function,
    size_t variables
) {
    stack_iterator(function->instructions_stack, index) {
        struct codegen_instruction instruction = *stack_it(function->instructions_stack, index);

        morphine_instruction_t instr = {
            .opcode = instruction.opcode,
            .line = instruction.line,
            .argument1 = argument_normalize(U, function, instruction.argument1, variables),
            .argument2 = argument_normalize(U, function, instruction.argument2, variables),
            .argument3 = argument_normalize(U, function, instruction.argument3, variables),
        };

        mapi_instruction_set(U, mapi_csize2index(U, index), instr);
    }
}

static inline void load_constants(
    morphine_coroutine_t U,
    struct morphinec_strtable *T,
    ml_size count,
    struct codegen_function *pool,
    struct codegen_function *function
) {
    stack_iterator(function->constants_stack, index) {
        struct codegen_constant constant = *stack_it(function->constants_stack, index);

        switch (constant.type) {
            case CCT_NIL:
                mapi_push_nil(U);
                mapi_constant_set(U, mapi_csize2index(U, index));
                break;
            case CCT_INTEGER:
                mapi_push_integer(U, constant.value.integer);
                mapi_constant_set(U, mapi_csize2index(U, index));
                break;
            case CCT_DECIMAL:
                mapi_push_decimal(U, constant.value.decimal);
                mapi_constant_set(U, mapi_csize2index(U, index));
                break;
            case CCT_STRING: {
                struct morphinec_strtable_entry string = mcapi_strtable_access(U, T, constant.value.string);
                mapi_push_stringn(U, string.string, string.size);
                mapi_constant_set(U, mapi_csize2index(U, index));
                break;
            }
            case CCT_BOOLEAN:
                mapi_push_boolean(U, constant.value.boolean);
                mapi_constant_set(U, mapi_csize2index(U, index));
                break;
            case CCT_FUNCTION: {
                ml_size fun_index = 0;
                struct codegen_function *current = pool;
                while (current != NULL) {
                    if (current->ref == constant.value.function) {
                        break;
                    }

                    fun_index++;
                    current = current->prev;
                }

                if (current == NULL) {
                    mapi_error(U, "function not found");
                }

                mapi_peek(U, 1);
                mapi_vector_get(U, count - fun_index - 1);
                mapi_rotate(U, 2);
                mapi_pop(U, 1);

                mapi_constant_set(U, mapi_csize2index(U, index));
                break;
            }
        }
    }
}

void codegen_construct(morphine_coroutine_t U, struct codegen *C) {
    ml_size count = 0;
    struct codegen_function *function = C->functions;
    while (function != NULL) {
        if (!function->compiled) {
            mapi_error(U, "uncompiled function");
        }

        count++;
        function = function->prev;
    }

    mapi_push_vector(U, count);

    ml_size index = 0;
    function = C->functions;
    while (function != NULL) {
        size_t params = 0;
        size_t variables = 0;
        size_t temporaries = 0;
        stack_iterator(function->instructions_stack, it) {
            struct codegen_instruction *instruction = stack_it(function->instructions_stack, it);

            if (instruction->opcode == MORPHINE_OPCODE_PARAM) {
                if (params < instruction->argument2.value.index.index + 1) {
                    params = instruction->argument2.value.index.index + 1;
                }
            }

            update_slot_argument(instruction->argument1, &variables, &temporaries);
            update_slot_argument(instruction->argument2, &variables, &temporaries);
            update_slot_argument(instruction->argument3, &variables, &temporaries);
        }

        const char *name;
        if (function->ref->anonymous) {
            name = "anonymous";
        } else {
            name = mcapi_strtable_access(U, C->T, function->ref->name).string;
        }

        mapi_push_function(
            U,
            name,
            function->ref->line,
            mapi_csize2size(U, stack_size(function->constants_stack)),
            mapi_csize2size(U, stack_size(function->instructions_stack)),
            mapi_csize2size(U, function->ref->statics_size),
            mapi_csize2size(U, function->ref->args_size),
            mapi_csize2size(U, variables + temporaries),
            mapi_csize2size(U, params)
        );

        load_instructions(U, function, variables);

        mapi_vector_set(U, count - index - 1);

        index++;
        function = function->prev;
    }

    index = 0;
    function = C->functions;
    while (function != NULL) {
        mapi_vector_get(U, count - index - 1);
        load_constants(U, C->T, count, C->functions, function);
        mapi_function_complete(U);
        mapi_pop(U, 1);

        index++;
        function = function->prev;
    }
}
