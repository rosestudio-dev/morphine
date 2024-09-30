//
// Created by why-iskra on 15.08.2024.
//

#pragma once

#include "morphinec/codegen.h"

typedef size_t anchor_t;
typedef size_t variable_slot_t;
typedef size_t temporary_slot_t;

enum instruction_argument_type {
    IAT_sslot,
    IAT_dslot,
    IAT_size,
    IAT_position,
    IAT_constant_index,
    IAT_param_index,
    IAT_argument_index,
    IAT_static_index,
    IAT_closure_index,
    IAT_params_count,
    IAT_stub,
};

enum variable_info_type {
    VIT_VARIABLE,
    VIT_STATIC,
    VIT_ARGUMENT,
    VIT_RECURSIVE,
    VIT_CLOSURE,
    VIT_NOT_FOUND
};

struct instruction_slot {
    bool is_variable;
    union {
        variable_slot_t variable_slot;
        temporary_slot_t temporary_slot;
    };
};

struct instruction_argument {
    enum instruction_argument_type type;

    union {
        size_t value_stub;

        struct instruction_slot value_sslot;
        struct instruction_slot value_dslot;
        struct instruction_slot value_slot;

        anchor_t value_position;
        size_t value_size;

        size_t value_constant_index;
        size_t value_param_index;
        size_t value_argument_index;
        size_t value_static_index;
        size_t value_closure_index;
        size_t value_params_count;
    };
};

struct variable_info {
    enum variable_info_type type;
    bool mutable;
    union {
        struct instruction_slot variable;
        size_t static_variable;
        size_t closure_variable;
        size_t argument;
    };
};

struct variable;
struct codegen_controller;

struct mc_strtable_entry codegen_string(struct codegen_controller *, mc_strtable_index_t);
morphine_noret void codegen_errorf(struct codegen_controller *, const char *, ...);
morphine_noret void codegen_lined_errorf(struct codegen_controller *, ml_line, const char *, ...);

morphine_noret void codegen_statement(
    struct codegen_controller *,
    struct mc_ast_statement *,
    size_t next_state
);

morphine_noret void codegen_expression(
    struct codegen_controller *,
    struct mc_ast_expression *,
    struct instruction_slot result_slot,
    size_t next_state
);

morphine_noret void codegen_set(
    struct codegen_controller *,
    struct mc_ast_expression *,
    struct instruction_slot slot,
    size_t next_state
);

morphine_noret void codegen_function(
    struct codegen_controller *,
    struct mc_ast_function *,
    size_t next_state
);

morphine_noret void codegen_jump(
    struct codegen_controller *,
    size_t next_state
);

morphine_noret void codegen_complete(struct codegen_controller *);

void *codegen_saved(struct codegen_controller *);
void *codegen_alloc_saved_uni(struct codegen_controller *, size_t);
void *codegen_alloc_saved_vec(struct codegen_controller *, size_t, size_t);

void codegen_add_instruction(
    struct codegen_controller *,
    morphine_opcode_t,
    struct instruction_argument argument1,
    struct instruction_argument argument2,
    struct instruction_argument argument3
);

size_t codegen_add_constant_nil(struct codegen_controller *);
size_t codegen_add_constant_int(struct codegen_controller *, ml_integer);
size_t codegen_add_constant_index(struct codegen_controller *, size_t);
size_t codegen_add_constant_dec(struct codegen_controller *, ml_decimal);
size_t codegen_add_constant_bool(struct codegen_controller *, bool);
size_t codegen_add_constant_str(struct codegen_controller *, mc_strtable_index_t);
size_t codegen_add_constant_cstr(struct codegen_controller *, const char *);
size_t codegen_add_constant_fun(struct codegen_controller *, struct mc_ast_function *);

anchor_t codegen_add_anchor(struct codegen_controller *);
void codegen_anchor_change(struct codegen_controller *, anchor_t);

void codegen_enter_scope(struct codegen_controller *, bool create_control_anchors);
void codegen_exit_scope(struct codegen_controller *);
anchor_t codegen_scope_break_anchor(struct codegen_controller *);
anchor_t codegen_scope_continue_anchor(struct codegen_controller *);

struct instruction_slot codegen_result(struct codegen_controller *);

bool codegen_is_recursive(struct codegen_controller *);

struct instruction_slot codegen_declare_temporary(struct codegen_controller *);

struct instruction_slot codegen_declare_variable(
    struct codegen_controller *,
    mc_strtable_index_t,
    bool mutable
);

struct variable_info codegen_get_variable(struct codegen_controller *, mc_strtable_index_t);

mc_strtable_index_t codegen_variable_name(struct variable *, size_t index);

size_t codegen_closures(
    struct codegen_controller *,
    struct mc_ast_function *,
    struct variable **
);
