//
// Created by why-iskra on 04.06.2024.
//

#pragma once

enum codegen_argument_type {
    CAT_VARIABLE,
    CAT_TEMPORARY,
    CAT_ANCHOR,
    CAT_FUNCTION,
    CAT_STABLE,
};

struct codegen_argument {
    enum codegen_argument_type type;
    union {
        size_t variable;
        size_t temporary;
        size_t anchor;
        uintptr_t function;
        ml_argument stable;
    };
};

struct codegen_controller;

morphine_noret void codegen_error(struct codegen_controller *, const char *);

morphine_noret void codegen_visit_expression(
    struct codegen_controller *,
    struct expression *,
    struct codegen_argument,
    size_t state
);

morphine_noret void codegen_visit_statement(
    struct codegen_controller *,
    struct statement *,
    size_t state
);

morphine_noret void codegen_visit_function(
    struct codegen_controller *,
    struct ast_function *,
    size_t state
);

morphine_noret void codegen_visit_next(struct codegen_controller *, size_t state);
morphine_noret void codegen_visit_return(struct codegen_controller *);

struct codegen_argument codegen_anchor(struct codegen_controller *);
struct codegen_argument codegen_temporary(struct codegen_controller *);

void codegen_scope_enter(struct codegen_controller *);
void codegen_scope_exit(struct codegen_controller *);

//void codegen_declare_variable(struct codegen_controller *N);
//void codegen_get_variable(struct codegen_controller *N);

void codegen_instruction(
    struct codegen_controller *,
    morphine_opcode_t,
    struct codegen_argument,
    struct codegen_argument,
    struct codegen_argument
);
