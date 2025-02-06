//
// Created by why-iskra on 03.06.2024.
//

#include <string.h>
#include "morphinec/codegen.h"
#include "codegen/controller.h"
#include "codegen/compiler.h"

#define TEMPS_PER_SCOPE_LIMIT mm_typemax(ml_argument)
#define VARIABLES_LIMIT       mm_typemax(ml_argument)
#define RESULTS_LIMIT         131072
#define SCOPES_LIMIT          131072
#define TEMPORARIES_LIMIT     131072
#define ANCHORS_LIMIT         131072
#define CLOSURES_LIMIT        mm_typemax(ml_argument)
#define CONSTANTS_LIMIT       mm_typemax(ml_argument)
#define INSTRUCTIONS_LIMIT    mm_typemax(ml_argument)
#define EXPANSION_FACTOR      32

#define list_struct(type) struct { size_t allocated; size_t size; type *array; }
#define init_list_struct(x, name) do { (x).name.allocated = 0; (x).name.size = 0; (x).name.array = NULL; } while(0)

enum result_type {
    RT_STATEMENT,
    RT_EXPRESSION,
    RT_EVAL,
    RT_SET
};

enum constant_type {
    CT_NIL,
    CT_INTEGER,
    CT_DECIMAL,
    CT_STRING,
    CT_BOOLEAN,
    CT_FUNCTION,
};

struct constant {
    enum constant_type type;
    union {
        ml_integer integer;
        ml_decimal decimal;
        mc_strtable_index_t string;
        bool boolean;
        struct mc_ast_function *function;
    } value;
};

struct instruction {
    ml_line line;
    mtype_opcode_t opcode;

    struct instruction_argument argument1;
    struct instruction_argument argument2;
    struct instruction_argument argument3;
};

struct scope {
    size_t variables_from;

    bool has_control_anchors;
    size_t break_anchor;
    size_t continue_anchor;
};

struct variable {
    bool mutable;
    mc_strtable_index_t index;
};

struct result {
    enum result_type type;
    struct instruction_slot slot;
};

struct context {
    struct mc_ast_function *function;

    list_struct(size_t) temporaries;
    list_struct(struct scope) scopes;
    list_struct(struct variable) variables;
    list_struct(struct result) results;

    list_struct(struct variable) closures;
    list_struct(size_t) anchors;
    list_struct(struct constant) constants;
    list_struct(struct instruction) instructions;

    struct context *prev;
};

struct mc_codegen {
    struct context *context;
    struct context *compiled;
};

struct codegen_controller {
    morphine_coroutine_t U;
    struct mc_codegen *G;
    struct mc_strtable *T;
    struct mc_visitor_controller *VC;
    struct mc_ast_node *node;
};

static struct variable_info deep_variable(
    struct codegen_controller *,
    struct context *,
    mc_strtable_index_t
);

// push/pop

static void push_result(struct codegen_controller *C, struct result result) {
    struct context *context = C->G->context;
    if (context->results.size > context->results.allocated) {
        mapi_error(C->U, "results stack corrupted");
    } else if (context->results.size == context->results.allocated) {
        if (context->results.allocated >= RESULTS_LIMIT) {
            mapi_error(C->U, "results limit");
        }

        context->results.array = mapi_allocator_vec(
            mapi_instance(C->U),
            context->results.array,
            context->results.allocated + EXPANSION_FACTOR,
            sizeof(struct result)
        );

        context->results.allocated += EXPANSION_FACTOR;
    }

    context->results.array[context->results.size] = result;
    context->results.size++;
}

static void pop_result(struct codegen_controller *C) {
    struct context *context = C->G->context;
    if (context->results.size == 0) {
        mapi_error(C->U, "empty results stack");
    }

    context->results.size--;
}

static void add_closure(
    struct codegen_controller *C,
    struct context *context,
    struct variable closure
) {
    if (context->closures.size > context->closures.allocated) {
        mapi_error(C->U, "closures array corrupted");
    } else if (context->closures.size == context->closures.allocated) {
        if (context->closures.allocated >= CLOSURES_LIMIT) {
            mapi_error(C->U, "closures limit");
        }

        context->closures.array = mapi_allocator_vec(
            mapi_instance(C->U),
            context->closures.array,
            context->closures.allocated + EXPANSION_FACTOR,
            sizeof(struct variable)
        );

        context->closures.allocated += EXPANSION_FACTOR;
    }

    context->closures.array[context->closures.size] = closure;
    context->closures.size++;
}

static void push_context(struct codegen_controller *C, struct mc_ast_function *function) {
    struct context *context = mapi_allocator_uni(mapi_instance(C->U), NULL, sizeof(struct context));

    *context = (struct context) {
        .function = function,
        .prev = C->G->context
    };

    init_list_struct(*context, temporaries);
    init_list_struct(*context, scopes);
    init_list_struct(*context, variables);
    init_list_struct(*context, results);
    init_list_struct(*context, closures);
    init_list_struct(*context, anchors);
    init_list_struct(*context, constants);
    init_list_struct(*context, instructions);

    C->G->context = context;

    struct result result = {
        .type = RT_STATEMENT,
    };

    push_result(C, result);

    for (size_t i = 0; i < function->args_size; i++) {
        size_t count = 0;
        for (size_t index = 0; index < function->args_size; index++) {
            if (function->arguments[i] == function->arguments[index]) {
                count++;
            }
        }

        if (count > 1) {
            codegen_errorf(C, "argument duplicates");
        }
    }

    if (!function->auto_closure) {
        for (size_t i = 0; i < function->closures_size; i++) {
            size_t count = 0;
            for (size_t index = 0; index < function->closures_size; index++) {
                if (function->closures[i] == function->closures[index]) {
                    count++;
                }
            }

            if (count > 1) {
                codegen_errorf(C, "closure duplicates");
            }

            if (C->G->context->prev == NULL) {
                mapi_error(C->U, "no previous function");
            }

            struct variable_info info = deep_variable(
                C, C->G->context->prev, function->closures[i]
            );

            if (info.type == VIT_NOT_FOUND) {
                codegen_errorf(
                    C, "closure variable '%s' not found",
                    mcapi_strtable_access(C->U, C->T, function->closures[i])
                );
            }

            struct variable closure = {
                .index = function->closures[i],
                .mutable = info.mutable
            };

            add_closure(C, C->G->context, closure);
        }
    }
}

static void pop_context(struct codegen_controller *C) {
    struct context *context = C->G->context;
    C->G->context = context->prev;

    context->prev = C->G->compiled;
    C->G->compiled = context;

    mapi_allocator_free(mapi_instance(C->U), context->temporaries.array);
    init_list_struct(*context, temporaries);

    mapi_allocator_free(mapi_instance(C->U), context->scopes.array);
    init_list_struct(*context, scopes);

    mapi_allocator_free(mapi_instance(C->U), context->variables.array);
    init_list_struct(*context, variables);

    mapi_allocator_free(mapi_instance(C->U), context->results.array);
    init_list_struct(*context, results);
}

static void add_instruction(struct codegen_controller *C, struct instruction instruction) {
    struct context *context = C->G->context;
    if (context->instructions.size > context->instructions.allocated) {
        mapi_error(C->U, "instructions array corrupted");
    } else if (context->instructions.size == context->instructions.allocated) {
        if (context->instructions.allocated >= INSTRUCTIONS_LIMIT) {
            mapi_error(C->U, "instructions limit");
        }

        context->instructions.array = mapi_allocator_vec(
            mapi_instance(C->U),
            context->instructions.array,
            context->instructions.allocated + EXPANSION_FACTOR,
            sizeof(struct instruction)
        );

        context->instructions.allocated += EXPANSION_FACTOR;
    }

    context->instructions.array[context->instructions.size] = instruction;
    context->instructions.size++;
}

static size_t add_constant(struct codegen_controller *C, struct constant constant) {
    struct context *context = C->G->context;

    for (size_t i = 0; i < context->constants.size; i++) {
        struct constant current = context->constants.array[i];

        if (constant.type != current.type) {
            continue;
        }

        bool eq = false;
        switch (current.type) {
            case CT_NIL:
                eq = true;
                break;
            case CT_INTEGER:
                eq = current.value.integer == constant.value.integer;
                break;
            case CT_DECIMAL:
                eq = current.value.decimal == constant.value.decimal;
                break;
            case CT_STRING:
                eq = current.value.string == constant.value.string;
                break;
            case CT_BOOLEAN:
                eq = current.value.boolean == constant.value.boolean;
                break;
            case CT_FUNCTION:
                eq = current.value.function == constant.value.function;
                break;
        }

        if (eq) {
            return i;
        }
    }

    if (context->constants.size > context->constants.allocated) {
        mapi_error(C->U, "constants array corrupted");
    } else if (context->constants.size == context->constants.allocated) {
        if (context->constants.allocated >= CONSTANTS_LIMIT) {
            mapi_error(C->U, "constants limit");
        }

        context->constants.array = mapi_allocator_vec(
            mapi_instance(C->U),
            context->constants.array,
            context->constants.allocated + EXPANSION_FACTOR,
            sizeof(struct constant)
        );

        context->constants.allocated += EXPANSION_FACTOR;
    }

    context->constants.array[context->constants.size] = constant;
    context->constants.size++;

    return context->constants.size - 1;
}

static anchor_t add_anchor(struct codegen_controller *C) {
    struct context *context = C->G->context;
    if (context->anchors.size > context->anchors.allocated) {
        mapi_error(C->U, "anchors array corrupted");
    } else if (context->anchors.size == context->anchors.allocated) {
        if (context->anchors.allocated >= ANCHORS_LIMIT) {
            mapi_error(C->U, "anchors limit");
        }

        context->anchors.array = mapi_allocator_vec(
            mapi_instance(C->U),
            context->anchors.array,
            context->anchors.allocated + EXPANSION_FACTOR,
            sizeof(size_t)
        );

        context->anchors.allocated += EXPANSION_FACTOR;
    }

    context->anchors.array[context->anchors.size] = context->instructions.size;
    context->anchors.size++;

    return context->anchors.size - 1;
}

static void enter_temporaries(struct codegen_controller *C) {
    struct context *context = C->G->context;
    if (context->temporaries.size > context->temporaries.allocated) {
        mapi_error(C->U, "temporaries stack corrupted");
    } else if (context->temporaries.size == context->temporaries.allocated) {
        if (context->temporaries.allocated >= TEMPORARIES_LIMIT) {
            mapi_error(C->U, "temporaries limit");
        }

        context->temporaries.array = mapi_allocator_vec(
            mapi_instance(C->U),
            context->temporaries.array,
            context->temporaries.allocated + EXPANSION_FACTOR,
            sizeof(size_t)
        );

        context->temporaries.allocated += EXPANSION_FACTOR;
    }

    if (context->temporaries.size > 0) {
        context->temporaries.array[context->temporaries.size] =
            context->temporaries.array[context->temporaries.size - 1];
    } else {
        context->temporaries.array[context->temporaries.size] = 0;
    }

    context->temporaries.size++;
}

static void exit_temporaries(struct codegen_controller *C) {
    struct context *context = C->G->context;
    if (context->temporaries.size == 0) {
        mapi_error(C->U, "empty temporaries stack");
    }

    context->temporaries.size--;
}

static temporary_slot_t push_temporary(struct codegen_controller *C) {
    struct context *context = C->G->context;
    if (context->temporaries.size == 0) {
        mapi_error(C->U, "empty temporaries stack");
    }

    size_t *temps = context->temporaries.array + (context->temporaries.size - 1);
    if (*temps >= TEMPS_PER_SCOPE_LIMIT) {
        mapi_error(C->U, "temporaries per scope limit");
    }

    (*temps)++;

    return *temps;
}

static variable_slot_t push_variable(struct codegen_controller *C, struct variable variable) {
    struct context *context = C->G->context;

    if (context->variables.size > context->variables.allocated) {
        mapi_error(C->U, "variables stack corrupted");
    } else if (context->variables.size == context->variables.allocated) {
        if (context->variables.allocated >= VARIABLES_LIMIT) {
            mapi_error(C->U, "variables limit");
        }

        context->variables.array = mapi_allocator_vec(
            mapi_instance(C->U),
            context->variables.array,
            context->variables.allocated + EXPANSION_FACTOR,
            sizeof(struct variable)
        );

        context->variables.allocated += EXPANSION_FACTOR;
    }

    context->variables.array[context->variables.size] = variable;
    context->variables.size++;

    return context->variables.size - 1;
}

static void pop_variable(struct codegen_controller *C) {
    struct context *context = C->G->context;
    if (context->variables.size == 0) {
        mapi_error(C->U, "empty variable stack");
    }

    context->variables.size--;
}

static void push_scope(struct codegen_controller *C, bool create_control_anchors) {
    struct context *context = C->G->context;
    if (context->scopes.size > context->scopes.allocated) {
        mapi_error(C->U, "scopes stack corrupted");
    } else if (context->scopes.size == context->scopes.allocated) {
        if (context->scopes.allocated >= SCOPES_LIMIT) {
            mapi_error(C->U, "scopes limit");
        }

        context->scopes.array = mapi_allocator_vec(
            mapi_instance(C->U),
            context->scopes.array,
            context->scopes.allocated + EXPANSION_FACTOR,
            sizeof(struct scope)
        );

        context->scopes.allocated += EXPANSION_FACTOR;
    }

    struct scope *scope = context->scopes.array + context->scopes.size;
    *scope = (struct scope) {
        .variables_from = context->variables.size,
        .has_control_anchors = false
    };

    context->scopes.size++;

    if (create_control_anchors) {
        scope->break_anchor = add_anchor(C);
        scope->continue_anchor = add_anchor(C);
        scope->has_control_anchors = true;
    }
}

static void pop_scope(struct codegen_controller *C) {
    struct context *context = C->G->context;
    if (context->scopes.size == 0) {
        mapi_error(C->U, "empty scope stack");
    }

    size_t from = context->scopes.array[context->scopes.size - 1].variables_from;

    if (from > context->variables.size) {
        mapi_error(C->U, "corrupted scope");
    }

    size_t size = context->variables.size - from;

    for (size_t i = 0; i < size; i++) {
        pop_variable(C);
    }

    context->scopes.size--;
}

// support

static struct variable_info get_variable(
    struct context *context,
    mc_strtable_index_t name
) {
    struct mc_ast_function *function = context->function;

    for (size_t i = 0; i < context->variables.size; i++) {
        size_t index = context->variables.size - i - 1;
        struct variable variable = context->variables.array[index];
        if (variable.index == name) {
            return (struct variable_info) {
                .type = VIT_VARIABLE,
                .mutable = variable.mutable,
                .variable.is_variable = true,
                .variable.variable_slot = index,
            };
        }
    }

    for (size_t i = 0; i < function->args_size; i++) {
        if (function->arguments[i] == name) {
            return (struct variable_info) {
                .type = VIT_ARGUMENT,
                .mutable = false,
                .argument = i
            };
        }
    }

    if (function->recursive && !function->anonymous && function->name == name) {
        return (struct variable_info) {
            .type = VIT_RECURSIVE,
            .mutable = false
        };
    }

    for (size_t i = 0; i < context->closures.size; i++) {
        struct variable closures = context->closures.array[i];
        if (closures.index == name) {
            return (struct variable_info) {
                .type = VIT_CLOSURE,
                .mutable = closures.mutable,
                .closure_variable = i
            };
        }
    }

    return (struct variable_info) {
        .type = VIT_NOT_FOUND,
        .mutable = true
    };
}

static struct variable_info deep_variable(
    struct codegen_controller *C,
    struct context *context_from,
    mc_strtable_index_t name
) {
    struct context *context = context_from;
    struct variable_info info;
    while (context != NULL) {
        info = get_variable(context, name);

        if (info.type != VIT_NOT_FOUND) {
            break;
        } else if (!context->function->auto_closure) {
            return (struct variable_info) {
                .type = VIT_NOT_FOUND,
                .mutable = true
            };
        }

        context = context->prev;
    }

    if (context == NULL) {
        return (struct variable_info) {
            .type = VIT_NOT_FOUND,
            .mutable = true
        };
    } else if (context == context_from) {
        return info;
    }

    struct context *current = context_from;
    while (current != context) {
        struct variable closure = {
            .index = name,
            .mutable = info.mutable
        };

        add_closure(C, current, closure);

        current = current->prev;
    }

    return (struct variable_info) {
        .type = VIT_CLOSURE,
        .mutable = info.mutable,
        .closure_variable = context_from->closures.size - 1
    };
}

// controller

struct mc_strtable_entry codegen_string(struct codegen_controller *C, mc_strtable_index_t index) {
    return mcapi_strtable_access(C->U, C->T, index);
}

mattr_noret void codegen_errorf(struct codegen_controller *C, const char *str, ...) {
    va_list args;
    va_start(args, str);
    mapi_push_stringv(C->U, str, args);
    va_end(args);

    if (C->node != NULL) {
        mapi_errorf(
            C->U,
            "line %"MLIMIT_LINE_PR": %s",
            C->node->line,
            mapi_get_string(C->U)
        );
    } else {
        mapi_error(C->U, mapi_get_string(C->U));
    }
}

mattr_noret void codegen_lined_errorf(struct codegen_controller *C, ml_line line, const char *str, ...) {
    va_list args;
    va_start(args, str);
    mapi_push_stringv(C->U, str, args);
    va_end(args);

    mapi_errorf(
        C->U,
        "line %"MLIMIT_LINE_PR": %s",
        line,
        mapi_get_string(C->U)
    );
}

mattr_noret void codegen_statement(
    struct codegen_controller *C,
    struct mc_ast_statement *statement,
    size_t next_state
) {
    struct result result = {
        .type = RT_STATEMENT,
    };

    push_result(C, result);
    mcapi_visitor_node(C->VC, mcapi_ast_statement2node(statement), next_state);
}

mattr_noret void codegen_expression(
    struct codegen_controller *C,
    struct mc_ast_expression *expression,
    struct instruction_slot result_slot,
    size_t next_state
) {
    struct result result = {
        .type = RT_EXPRESSION,
        .slot = result_slot
    };

    push_result(C, result);
    mcapi_visitor_node(C->VC, mcapi_ast_expression2node(expression), next_state);
}

mattr_noret void codegen_eval(
    struct codegen_controller *C,
    struct mc_ast_statement *statement,
    struct instruction_slot result_slot,
    size_t next_state
) {
    struct result result = {
        .type = RT_EVAL,
        .slot = result_slot
    };

    push_result(C, result);
    mcapi_visitor_node(C->VC, mcapi_ast_statement2node(statement), next_state);
}

mattr_noret void codegen_set(
    struct codegen_controller *C,
    struct mc_ast_expression *expression,
    struct instruction_slot slot,
    size_t next_state
) {
    struct result result = {
        .type = RT_SET,
        .slot = slot
    };

    push_result(C, result);
    mcapi_visitor_node(C->VC, mcapi_ast_expression2node(expression), next_state);
}

mattr_noret void codegen_function(
    struct codegen_controller *C,
    struct mc_ast_function *function,
    size_t next_state
) {
    push_context(C, function);
    mcapi_visitor_function(C->VC, function, next_state);
}

mattr_noret void codegen_jump(
    struct codegen_controller *C,
    size_t next_state
) {
    mcapi_visitor_jump(C->VC, next_state);
}

mattr_noret void codegen_complete(struct codegen_controller *C) {
    mcapi_visitor_complete(C->VC);
}

void *codegen_saved(struct codegen_controller *C) {
    return mcapi_visitor_saved(C->VC);
}

void *codegen_alloc_saved_uni(struct codegen_controller *C, size_t size) {
    return mcapi_visitor_alloc_saved_uni(C->VC, size);
}

void *codegen_alloc_saved_vec(struct codegen_controller *C, size_t count, size_t size) {
    return mcapi_visitor_alloc_saved_vec(C->VC, count, size);
}

void codegen_add_instruction(
    struct codegen_controller *C,
    mtype_opcode_t opcode,
    struct instruction_argument argument1,
    struct instruction_argument argument2,
    struct instruction_argument argument3
) {
    struct instruction instruction = {
        .opcode = opcode,
        .line = C->node == NULL ? 0 : C->node->line,
        .argument1 = argument1,
        .argument2 = argument2,
        .argument3 = argument3
    };

    add_instruction(C, instruction);
}

size_t codegen_add_constant_nil(struct codegen_controller *C) {
    struct constant constant = {
        .type = CT_NIL,
    };

    return add_constant(C, constant);
}

size_t codegen_add_constant_int(struct codegen_controller *C, ml_integer value) {
    struct constant constant = {
        .type = CT_INTEGER,
        .value.integer = value
    };

    return add_constant(C, constant);
}

size_t codegen_add_constant_index(struct codegen_controller *C, size_t value) {
    struct constant constant = {
        .type = CT_INTEGER,
        .value.integer = mapi_csize2size(C->U, value, "index")
    };

    return add_constant(C, constant);
}

size_t codegen_add_constant_dec(struct codegen_controller *C, ml_decimal value) {
    struct constant constant = {
        .type = CT_DECIMAL,
        .value.decimal = value
    };

    return add_constant(C, constant);
}

size_t codegen_add_constant_bool(struct codegen_controller *C, bool value) {
    struct constant constant = {
        .type = CT_BOOLEAN,
        .value.boolean = value
    };

    return add_constant(C, constant);
}

size_t codegen_add_constant_str(struct codegen_controller *C, mc_strtable_index_t value) {
    struct constant constant = {
        .type = CT_STRING,
        .value.string = value
    };

    return add_constant(C, constant);
}

size_t codegen_add_constant_cstr(struct codegen_controller *C, const char *value) {
    struct constant constant = {
        .type = CT_STRING,
        .value.string = mcapi_strtable_record(C->U, C->T, value, strlen(value))
    };

    return add_constant(C, constant);
}

size_t codegen_add_constant_fun(struct codegen_controller *C, struct mc_ast_function *value) {
    struct constant constant = {
        .type = CT_FUNCTION,
        .value.function = value
    };

    return add_constant(C, constant);
}

anchor_t codegen_add_anchor(struct codegen_controller *C) {
    return add_anchor(C);
}

void codegen_anchor_change(struct codegen_controller *C, anchor_t anchor) {
    struct context *context = C->G->context;
    if (anchor >= context->anchors.size) {
        mapi_error(C->U, "unknown anchor");
    }

    context->anchors.array[anchor] = context->instructions.size;
}

void codegen_enter_scope(struct codegen_controller *C, bool create_control_anchors) {
    push_scope(C, create_control_anchors);
}

void codegen_exit_scope(struct codegen_controller *C) {
    pop_scope(C);
}

anchor_t codegen_scope_break_anchor(struct codegen_controller *C) {
    struct context *context = C->G->context;

    for (size_t i = 0; i < context->scopes.size; i++) {
        struct scope scope = context->scopes.array[context->scopes.size - i - 1];
        if (scope.has_control_anchors) {
            return scope.break_anchor;
        }
    }

    codegen_errorf(C, "hasn't breakable statement");
}

anchor_t codegen_scope_continue_anchor(struct codegen_controller *C) {
    struct context *context = C->G->context;

    for (size_t i = 0; i < context->scopes.size; i++) {
        struct scope scope = context->scopes.array[context->scopes.size - i - 1];
        if (scope.has_control_anchors) {
            return scope.continue_anchor;
        }
    }

    codegen_errorf(C, "hasn't continuable statement");
}

struct instruction_slot codegen_result(struct codegen_controller *C) {
    struct context *context = C->G->context;
    if (context->results.size == 0) {
        mapi_error(C->U, "empty results stack");
    }

    struct result result = context->results.array[context->results.size - 1];

    switch (result.type) {
        case RT_STATEMENT:
            mapi_error(C->U, "no results");
        case RT_EXPRESSION:
        case RT_EVAL:
        case RT_SET:
            break;
    }

    return result.slot;
}

bool codegen_is_recursive(struct codegen_controller *C) {
    return C->G->context->function->recursive;
}

struct instruction_slot codegen_declare_temporary(struct codegen_controller *C) {
    return (struct instruction_slot) {
        .is_variable = false,
        .temporary_slot = push_temporary(C)
    };
}

struct instruction_slot codegen_declare_variable(
    struct codegen_controller *C,
    mc_strtable_index_t name,
    bool mutable
) {
    struct context *context = C->G->context;
    size_t from = 0;
    if (context->scopes.size > 0) {
        from = context->scopes.array[context->scopes.size - 1].variables_from;
    }

    for (size_t i = from; i < context->variables.size; i++) {
        if (context->variables.array[i].index == name) {
            codegen_errorf(
                C, "variable '%s' already declared",
                mcapi_strtable_access(C->U, C->T, name)
            );
        }
    }

    struct variable variable = {
        .mutable = mutable,
        .index = name
    };

    struct instruction_slot slot = {
        .is_variable = true,
        .variable_slot = push_variable(C, variable)
    };

    return slot;
}

struct variable_info codegen_get_variable(
    struct codegen_controller *C,
    mc_strtable_index_t name
) {
    return deep_variable(C, C->G->context, name);
}

mc_strtable_index_t codegen_variable_name(struct variable *variables, size_t index) {
    return variables[index].index;
}

size_t codegen_closures(
    struct codegen_controller *C,
    struct mc_ast_function *function,
    struct variable **result
) {
    struct context *context = C->G->compiled;
    while (context != NULL) {
        if (context->function == function) {
            break;
        }

        context = context->prev;
    }

    if (context == NULL) {
        mapi_error(C->U, "function not found");
    }

    if (result != NULL) {
        *result = context->closures.array;
    }

    return context->closures.size;
}

// visitor

static void visitor_function_eval(
    struct codegen_controller *C,
    struct mc_ast_node *node,
    size_t state
) {
    switch (state) {
        case 0: {
            if (node == NULL) {
                codegen_jump(C, 2);
            }

            struct mc_ast_statement *statement =
                mcapi_ast_node2statement(C->U, node);

            if (statement->type == MCSTMTT_eval) {
                struct mc_ast_statement_eval *eval = mcapi_ast_statement2eval(C->U, statement);
                codegen_expression(C, eval->expression, codegen_result(C), 1);
            } else {
                codegen_statement(C, statement, 2);
            }
        }
        case 1: {
            codegen_complete(C);
        }
        case 2: {
            size_t constant = codegen_add_constant_nil(C);
            codegen_add_instruction(
                C, MTYPE_OPCODE_LOAD,
                (struct instruction_argument) { .type = IAT_constant_index, .value_constant_index = constant },
                (struct instruction_argument) { .type = IAT_dslot, .value_dslot = codegen_result(C) },
                (struct instruction_argument) { .type = IAT_stub, .value_stub = 0 }
            );

            codegen_complete(C);
        }
        default:
            break;
    }
}

#define expr_case(n) case MCEXPRT_##n: codegen_compile_expression_##n(C, mcapi_ast_expression2##n(C->U, expression), state); break;
#define stmt_case(n) case MCSTMTT_##n: codegen_compile_statement_##n(C, mcapi_ast_statement2##n(C->U, statement), state); break;
#define set_case(n)  case MCEXPRT_##n: codegen_compile_set_##n(C, mcapi_ast_expression2##n(C->U, expression), state); break;

static void visitor_function(
    struct mc_visitor_controller *VC,
    struct mc_ast_node *node,
    size_t state
) {
    struct codegen_controller *C = mcapi_visitor_data(VC);
    C->VC = VC;
    C->node = node;

    struct context *context = C->G->context;

    if (context->results.size == 0) {
        mapi_error(C->U, "no results");
    }

    switch (context->results.array[context->results.size - 1].type) {
        case RT_STATEMENT: {
            struct mc_ast_statement *statement =
                mcapi_ast_node2statement(C->U, node);

            switch (statement->type) {
                stmt_case(pass)
                stmt_case(yield)
                stmt_case(eval)
                stmt_case(while)
                stmt_case(for)
                stmt_case(iterator)
                stmt_case(declaration)
                stmt_case(assigment)
            }
            break;
        }
        case RT_EXPRESSION: {
            struct mc_ast_expression *expression =
                mcapi_ast_node2expression(C->U, node);

            switch (expression->type) {
                expr_case(value)
                expr_case(binary)
                expr_case(unary)
                expr_case(increment)
                expr_case(variable)
                expr_case(env)
                expr_case(invoked)
                expr_case(leave)
                expr_case(break)
                expr_case(continue)
                expr_case(table)
                expr_case(vector)
                expr_case(access)
                expr_case(call)
                expr_case(function)
                expr_case(block)
                expr_case(if)
                expr_case(when)
                expr_case(asm)
            }
            break;
        }
        case RT_SET: {
            struct mc_ast_expression *expression =
                mcapi_ast_node2expression(C->U, node);

            switch (expression->type) {
                set_case(variable)
                set_case(access)
                default:
                    codegen_errorf(C, "unsupported for set");
            }
            break;
        }
        case RT_EVAL: {
            visitor_function_eval(C, node, state);
            break;
        }
    }
}

// api

static void codegen_userdata_constructor(morphine_instance_t I, void *data) {
    (void) I;

    struct mc_codegen *G = data;
    *G = (struct mc_codegen) {
        .context = NULL,
        .compiled = NULL
    };
}

static void codegen_userdata_free_context(morphine_instance_t I, struct context *context) {
    struct context *current = context;
    while (current != NULL) {
        struct context *prev = current->prev;

        mapi_allocator_free(I, current->temporaries.array);
        mapi_allocator_free(I, current->scopes.array);
        mapi_allocator_free(I, current->variables.array);
        mapi_allocator_free(I, current->results.array);
        mapi_allocator_free(I, current->closures.array);
        mapi_allocator_free(I, current->anchors.array);
        mapi_allocator_free(I, current->constants.array);
        mapi_allocator_free(I, current->instructions.array);
        mapi_allocator_free(I, current);

        current = prev;
    }
}

static void codegen_userdata_destructor(morphine_instance_t I, void *data) {
    struct mc_codegen *G = data;
    codegen_userdata_free_context(I, G->context);
    codegen_userdata_free_context(I, G->compiled);
}

MORPHINE_API struct mc_codegen *mcapi_push_codegen(morphine_coroutine_t U) {
    morphine_usertype_t usertype = {
        .name = MC_CODEGEN_USERDATA_TYPE,
        .size = sizeof(struct mc_codegen),
        .constructor = codegen_userdata_constructor,
        .destructor = codegen_userdata_destructor,
        .compare = NULL,
        .hash = NULL,
        .metatable = false,
    };

    mapi_usertype_declare(U, usertype);
    return mapi_push_userdata(U, MC_CODEGEN_USERDATA_TYPE);
}

MORPHINE_API struct mc_codegen *mcapi_get_codegen(morphine_coroutine_t U) {
    return mapi_userdata_pointer(U, MC_CODEGEN_USERDATA_TYPE);
}

MORPHINE_API bool mcapi_codegen_step(
    morphine_coroutine_t U,
    struct mc_codegen *G,
    struct mc_visitor *V,
    struct mc_strtable *T,
    struct mc_ast *A
) {
    struct codegen_controller controller = {
        .U = U,
        .G = G,
        .T = T,
        .VC = NULL,
        .node = NULL
    };

    enum mc_visitor_event event = MCVE_INIT;
    bool result = mcapi_visitor_step(
        U, V, A, visitor_function, &event, &controller
    );

    switch (event) {
        case MCVE_INIT: {
            push_context(&controller, mcapi_ast_get_root_function(A));
            enter_temporaries(&controller);
            break;
        }
        case MCVE_ENTER_FUNCTION:
        case MCVE_ENTER_NODE: {
            enter_temporaries(&controller);
            break;
        }
        case MCVE_DROP_FUNCTION: {
            pop_context(&controller);
            break;
        }
        case MCVE_DROP_NODE: {
            pop_result(&controller);
            exit_temporaries(&controller);
            break;
        }
        case MCVE_NEXT_STEP:
        case MCVE_COMPLETE:
            break;
    }

    return result;
}

// build

static inline void update_slot_argument(
    struct instruction_argument argument,
    size_t *variables,
    size_t *temporaries
) {
    if (argument.type != IAT_sslot && argument.type != IAT_dslot) {
        return;
    }

    if (argument.value_slot.is_variable && *variables < argument.value_slot.variable_slot + 1) {
        *variables = argument.value_slot.variable_slot + 1;
    } else if (!argument.value_slot.is_variable && *temporaries < argument.value_slot.temporary_slot + 1) {
        *temporaries = argument.value_slot.temporary_slot + 1;
    }
}

static inline void update_param_argument(
    struct instruction_argument argument,
    size_t *params
) {
    if (argument.type == IAT_params_count) {
        if (argument.value_params_count > *params) {
            *params = argument.value_params_count;
        }
        return;
    }

    if (argument.type == IAT_param_index) {
        if (argument.value_param_index + 1 > *params) {
            *params = argument.value_param_index + 1;
        }
        return;
    }
}

static inline size_t compiled_size(struct mc_codegen *G) {
    size_t size = 0;
    struct context *context = G->compiled;
    while (context != NULL) {
        size++;
        context = context->prev;
    }

    return size;
}

static inline ml_argument argument_normalize(
    morphine_coroutine_t U,
    struct context *context,
    struct instruction_argument argument,
    size_t variables
) {
    switch (argument.type) {
        case IAT_stub:
            return 0;
        case IAT_sslot:
        case IAT_dslot: {
            size_t slot;
            if (argument.value_slot.is_variable) {
                slot = argument.value_slot.variable_slot;
            } else {
                slot = argument.value_slot.temporary_slot + variables;
            }

            if (slot > mm_typemax(ml_argument)) {
                mapi_error(U, "slot too big");
            }

            return (ml_argument) slot;
        }
        case IAT_position: {
            if (argument.value_position >= context->anchors.size) {
                mapi_error(U, "unknown anchor");
            }

            size_t position = context->anchors.array[argument.value_position];

            if (position > mm_typemax(ml_argument)) {
                mapi_error(U, "position too big");
            }

            return (ml_argument) position;
        }
        case IAT_size:
            if (argument.value_size > mm_typemax(ml_argument)) {
                mapi_error(U, "size too big");
            }
            return (ml_argument) argument.value_size;
        case IAT_constant_index:
            if (argument.value_constant_index > mm_typemax(ml_argument)) {
                mapi_error(U, "constant index too big");
            }
            return (ml_argument) argument.value_constant_index;
        case IAT_param_index:
            if (argument.value_param_index > mm_typemax(ml_argument)) {
                mapi_error(U, "param index too big");
            }
            return (ml_argument) argument.value_param_index;
        case IAT_argument_index:
            if (argument.value_argument_index > mm_typemax(ml_argument)) {
                mapi_error(U, "argument index too big");
            }
            return (ml_argument) argument.value_argument_index;
        case IAT_params_count:
            if (argument.value_params_count > mm_typemax(ml_argument)) {
                mapi_error(U, "params count too big");
            }
            return (ml_argument) argument.value_params_count;
    }

    mapi_error(U, "undefined argument");
}

static inline void load_instructions(
    morphine_coroutine_t U,
    struct context *context,
    size_t variables
) {
    for (size_t i = 0; i < context->instructions.size; i++) {
        struct instruction instruction = context->instructions.array[i];

        morphine_instruction_t instr = {
            .opcode = instruction.opcode,
            .line = instruction.line,
            .argument1 = argument_normalize(U, context, instruction.argument1, variables),
            .argument2 = argument_normalize(U, context, instruction.argument2, variables),
            .argument3 = argument_normalize(U, context, instruction.argument3, variables),
        };

        mapi_instruction_set(U, mapi_csize2size(U, i, "index"), instr);
    }
}

static inline void fill_vector(
    morphine_coroutine_t U,
    struct mc_codegen *G,
    struct mc_strtable *T,
    struct mc_ast *A,
    const char *main
) {
    struct context *context = G->compiled;
    for (size_t i = 0; context != NULL; i++) {
        size_t variables = 0;
        size_t temporaries = 0;
        size_t params = 0;

        for (size_t index = 0; index < context->instructions.size; index++) {
            struct instruction instruction = context->instructions.array[index];

            update_slot_argument(instruction.argument1, &variables, &temporaries);
            update_slot_argument(instruction.argument2, &variables, &temporaries);
            update_slot_argument(instruction.argument3, &variables, &temporaries);

            update_param_argument(instruction.argument1, &params);
            update_param_argument(instruction.argument2, &params);
            update_param_argument(instruction.argument3, &params);
        }

        if (mcapi_ast_get_root_function(A) == context->function) {
            mapi_push_string(U, main);
        } else if (context->function->anonymous) {
            mapi_push_string(U, "anonymous");
        } else {
            struct mc_strtable_entry entry = mcapi_strtable_access(U, T, context->function->name);
            mapi_push_stringn(U, entry.string, entry.size);
        }

        mapi_push_function(
            U, context->function->line,
            mapi_csize2size(U, context->instructions.size, NULL),
            mapi_csize2size(U, context->constants.size, NULL),
            mapi_csize2size(U, variables + temporaries, NULL),
            mapi_csize2size(U, params, NULL)
        );

        load_instructions(U, context, variables);

        mapi_vector_set(U, mapi_csize2size(U, i, "index"));
        context = context->prev;
    }
}

static inline void load_constants(
    morphine_coroutine_t U,
    struct mc_codegen *G,
    struct mc_strtable *T,
    struct context *context
) {
    for (size_t i = 0; i < context->constants.size; i++) {
        struct constant constant = context->constants.array[i];

        switch (constant.type) {
            case CT_NIL:
                mapi_push_nil(U);
                mapi_constant_set(U, mapi_csize2size(U, i, "index"));
                break;
            case CT_INTEGER:
                mapi_push_integer(U, constant.value.integer);
                mapi_constant_set(U, mapi_csize2size(U, i, "index"));
                break;
            case CT_DECIMAL:
                mapi_push_decimal(U, constant.value.decimal);
                mapi_constant_set(U, mapi_csize2size(U, i, "index"));
                break;
            case CT_STRING: {
                struct mc_strtable_entry entry = mcapi_strtable_access(U, T, constant.value.string);
                mapi_push_stringn(U, entry.string, entry.size);
                mapi_constant_set(U, mapi_csize2size(U, i, "index"));
                break;
            }
            case CT_BOOLEAN:
                mapi_push_boolean(U, constant.value.boolean);
                mapi_constant_set(U, mapi_csize2size(U, i, "index"));
                break;
            case CT_FUNCTION: {
                struct context *current = G->compiled;
                size_t index = 0;
                while (current != NULL) {
                    if (current->function == constant.value.function) {
                        break;
                    }

                    index++;
                    current = current->prev;
                }

                if (current == NULL) {
                    mapi_error(U, "function not found");
                }

                mapi_peek(U, 1);
                mapi_vector_get(U, mapi_csize2size(U, index, "index"));
                mapi_rotate(U, 2);
                mapi_pop(U, 1);

                mapi_constant_set(U, mapi_csize2size(U, i, "index"));
                break;
            }
        }
    }
}

static inline void build_vector(
    morphine_coroutine_t U,
    struct mc_codegen *G,
    struct mc_strtable *T
) {
    struct context *context = G->compiled;
    for (size_t i = 0; context != NULL; i++) {
        mapi_vector_get(U, mapi_csize2size(U, i, "index"));
        load_constants(U, G, T, context);
        mapi_pop(U, 1);

        context = context->prev;
    }
}

static inline void extract_main(
    morphine_coroutine_t U,
    struct mc_codegen *G,
    struct mc_ast *A
) {
    struct context *current = G->compiled;
    size_t index = 0;
    while (current != NULL) {
        if (current->function == mcapi_ast_get_root_function(A)) {
            break;
        }

        index++;
        current = current->prev;
    }

    mapi_vector_get(U, mapi_csize2size(U, index, "index"));
}

MORPHINE_API void mcapi_codegen_build(
    morphine_coroutine_t U,
    struct mc_codegen *G,
    struct mc_strtable *T,
    struct mc_ast *A,
    const char *main,
    bool vector
) {
    size_t size = compiled_size(G);
    mapi_push_vector(U, mapi_csize2size(U, size, NULL));
    fill_vector(U, G, T, A, main);
    build_vector(U, G, T);

    if (!vector) {
        extract_main(U, G, A);
        mapi_rotate(U, 2);
        mapi_pop(U, 1);
    }
}
