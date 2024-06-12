//
// Created by why-iskra on 04.06.2024.
//

#pragma once

#include "morphinec/ast.h"

struct codegen_closure {
    strtable_index_t index;
    bool mutable;
};

enum codegen_argument_type {
    CAT_stub,
    CAT_index,
    CAT_count,
    CAT_slot,
    CAT_anchor
};

struct codegen_argument_slot {
    bool is_variable;
    size_t index;
};

struct codegen_argument_anchor {
    size_t index;
};

struct codegen_argument_index {
    size_t index;
};

struct codegen_argument_count {
    size_t count;
};

struct codegen_argument {
    enum codegen_argument_type type;
    union {
        struct codegen_argument_index index;
        struct codegen_argument_count count;
        struct codegen_argument_slot slot;
        struct codegen_argument_anchor anchor;
        size_t stub;
    } value;
};

enum codegen_variable_type {
    CVT_VARIABLE,
    CVT_ARGUMENT,
    CVT_RECURSION,
    CVT_CLOSURE,
    CVT_STATIC,
    CVT_NOT_FOUND
};

struct codegen_variable_info {
    bool mutable;
    enum codegen_variable_type type;
    struct {
        struct codegen_argument_slot variable;
        struct codegen_argument_index argument;
        struct codegen_argument_index closure;
        struct codegen_argument_index static_variable;
    };
};

struct codegen_controller;

morphine_noret void codegen_visit_expression(
    struct codegen_controller *,
    struct expression *,
    struct codegen_argument_slot,
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

void codegen_instruction(
    struct codegen_controller *,
    morphine_opcode_t,
    struct codegen_argument,
    struct codegen_argument,
    struct codegen_argument
);

morphine_coroutine_t codegen_U(struct codegen_controller *);
morphine_noret void codegen_error(struct codegen_controller *, const char *);
morphine_noret void codegen_errorf(struct codegen_controller *, const char *, ...);
struct strtable_entry codegen_string(struct codegen_controller *, strtable_index_t);
bool codegen_save(struct codegen_controller *, size_t, void **);
morphine_noret void codegen_visit_next(struct codegen_controller *, size_t state);
morphine_noret void codegen_visit_return(struct codegen_controller *);
struct codegen_argument_slot codegen_result(struct codegen_controller *);
struct codegen_argument_anchor codegen_anchor(struct codegen_controller *);
void codegen_anchor_change(struct codegen_controller *, struct codegen_argument_anchor);
void codegen_init_break_continue(struct codegen_controller *);
void codegen_break_change(struct codegen_controller *);
void codegen_continue_change(struct codegen_controller *);
struct codegen_argument_anchor codegen_break_get(struct codegen_controller *);
struct codegen_argument_anchor codegen_continue_get(struct codegen_controller *);
struct codegen_argument_slot codegen_temporary(struct codegen_controller *);
struct codegen_argument_index codegen_constant_nil(struct codegen_controller *);
struct codegen_argument_index codegen_constant_int(struct codegen_controller *, ml_integer);
struct codegen_argument_index codegen_constant_dec(struct codegen_controller *, ml_decimal);
struct codegen_argument_index codegen_constant_str(struct codegen_controller *, strtable_index_t);
struct codegen_argument_index codegen_constant_bool(struct codegen_controller *, bool);
struct codegen_argument_index codegen_constant_fun(struct codegen_controller *, struct ast_function *);
void codegen_scope_enter(struct codegen_controller *);
void codegen_scope_exit(struct codegen_controller *);
void codegen_closure(struct codegen_controller *, struct ast_function *, size_t *, struct codegen_closure **);
void codegen_declare_variable(struct codegen_controller *, strtable_index_t, bool mutable);
struct codegen_variable_info codegen_get_variable(struct codegen_controller *, strtable_index_t);
