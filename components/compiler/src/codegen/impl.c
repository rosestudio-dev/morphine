//
// Created by why-iskra on 04.06.2024.
//

#include "impl.h"
#include "support/instruction.h"

#define gen_func(t, n) static void gen_##t##_##n(struct codegen_controller *, struct t##_##n *, size_t state);
#define gen_func_dec_named(t, n, a) static void gen_##t##_##n(struct codegen_controller *N, struct t##_##n * a, size_t state)
#define gen_func_dec(t, n) gen_func_dec_named(t, n, n)

gen_func(statement, block)
gen_func(statement, eval)
gen_func(statement, simple)
gen_func(statement, return)
gen_func(statement, for)
gen_func(statement, while)
gen_func(statement, iterator)
gen_func(statement, if)
gen_func(statement, declaration)
gen_func(statement, assigment)

gen_func(expression, value)
gen_func(expression, binary)
gen_func(expression, unary)
gen_func(expression, increment)
gen_func(expression, global)
gen_func(expression, variable)
gen_func(expression, table)
gen_func(expression, vector)
gen_func(expression, access)
gen_func(expression, call)
gen_func(expression, call_self)
gen_func(expression, function)
gen_func(expression, block)
gen_func(expression, if)

#define call_expression(n) case EXPRESSION_TYPE_##n: gen_expression_##n(N, ast_as_expression_##n(codegen_U(N), ast_as_node(expression)), state); break
#define call_statement(n) case STATEMENT_TYPE_##n: gen_statement_##n(N, ast_as_statement_##n(codegen_U(N), ast_as_node(statement)), state); break

void gen_expression(struct codegen_controller *N, struct expression *expression, size_t state) {
    switch (ast_expression_type(expression)) {
        call_expression(value);
        call_expression(binary);
        call_expression(unary);
        call_expression(increment);
        call_expression(variable);
        call_expression(global);
        call_expression(table);
        call_expression(vector);
        call_expression(access);
        call_expression(call);
        call_expression(call_self);
        call_expression(function);
        call_expression(block);
        call_expression(if);
        default:
            break;
    }
}

void gen_statement(struct codegen_controller *N, struct statement *statement, size_t state) {
    switch (ast_statement_type(statement)) {
        call_statement(block);
        call_statement(simple);
        call_statement(eval);
        call_statement(return);
        call_statement(while);
        call_statement(for);
        call_statement(iterator);
        call_statement(declaration);
        call_statement(assigment);
        call_statement(if);
        default:
            break;
    }
}

static void get_variable(
    struct codegen_controller *N,
    strtable_index_t variable,
    struct codegen_argument_slot result
) {
    struct codegen_variable_info info = codegen_get_variable(N, variable);
    switch (info.type) {
        case CVT_VARIABLE:
            codegen_instruction_MOVE(N, info.variable, result);
            break;
        case CVT_ARGUMENT:
            codegen_instruction_ARG(N, info.argument, result);
            break;
        case CVT_RECURSION:
            codegen_instruction_RECURSION(N, result);
            break;
        case CVT_CLOSURE:
            codegen_instruction_RECURSION(N, result);
            codegen_instruction_GET_CLOSURE(N, result, info.closure, result);
            break;
        case CVT_STATIC:
            codegen_instruction_RECURSION(N, result);
            codegen_instruction_GET_STATIC(N, result, info.static_variable, result);
            break;
        default:
            codegen_errorf(N, "variable %s not found", codegen_string(N, variable).string);
    }
}

static void set_variable(
    struct codegen_controller *N,
    strtable_index_t variable,
    bool ignore_mutable,
    struct codegen_argument_slot value,
    struct codegen_argument_slot *container
) {
    struct codegen_variable_info info = codegen_get_variable(N, variable);

    if (!ignore_mutable && !info.mutable) {
        codegen_errorf(N, "immutable variable %s", codegen_string(N, variable).string);
    }

    switch (info.type) {
        case CVT_VARIABLE:
            codegen_instruction_MOVE(N, value, info.variable);
            break;
        case CVT_ARGUMENT:
            codegen_error(N, "cannot be set to argument");
        case CVT_RECURSION:
            codegen_error(N, "cannot be set to function");
        case CVT_CLOSURE: {
            if (container == NULL) {
                codegen_error(N, "cannot be set to closure");
            }

            *container = codegen_temporary(N);
            codegen_instruction_RECURSION(N, *container);
            codegen_instruction_SET_CLOSURE(N, *container, info.closure, value);
            break;
        }
        case CVT_STATIC: {
            if (container == NULL) {
                codegen_error(N, "cannot be set to static");
            }

            *container = codegen_temporary(N);
            codegen_instruction_RECURSION(N, *container);
            codegen_instruction_SET_STATIC(N, *container, info.static_variable, value);
            break;
        }
        default:
            codegen_errorf(N, "variable %s not found", codegen_string(N, variable).string);
    }
}

static void set(
    struct codegen_controller *N,
    size_t state,
    size_t state_offset,
    struct expression *expression,
    struct codegen_argument_slot value,
    struct codegen_argument_slot *container,
    struct codegen_argument_slot *key
) {
    switch (expression->type) {
        case EXPRESSION_TYPE_variable: {
            struct expression_variable *variable = ast_as_expression_variable(
                codegen_U(N), ast_as_node(expression)
            );

            set_variable(N, variable->index, false, value, container);
            break;
        }
        case EXPRESSION_TYPE_access: {
            struct expression_access *access = ast_as_expression_access(
                codegen_U(N), ast_as_node(expression)
            );

            if (state == state_offset) {
                *container = codegen_temporary(N);
                *key = codegen_temporary(N);

                codegen_visit_expression(N, access->container, *container, state_offset + 1);
            } else if (state == state_offset + 1) {
                codegen_visit_expression(N, access->key, *key, state_offset + 2);
            } else if (state == state_offset + 2) {
                codegen_instruction_SET(N, *container, *key, value);
            } else {
                codegen_error(N, "undefined set state");
            }
            break;
        }
        default:
            codegen_error(N, "cannot be set to expression");
    }
}

struct gen_sblock_data {
    size_t index;
};

gen_func_dec(statement, block) {
    struct gen_sblock_data *data;
    if (codegen_save(N, sizeof(struct gen_sblock_data), (void **) &data)) {
        data->index = 0;
    }

    switch (state) {
        case 0: {
            codegen_scope_enter(N);
            codegen_visit_next(N, 1);
        }
        case 1: {
            if (data->index < block->size) {
                size_t index = data->index;
                data->index++;
                codegen_visit_statement(N, block->statements[index], 1);
            } else {
                codegen_scope_exit(N);
                codegen_visit_return(N);
            }
        }
        default:
            break;
    }
}

gen_func_dec(statement, simple) {
    (void) state;

    switch (simple->type) {
        case STATEMENT_SIMPLE_TYPE_PASS:
            codegen_visit_return(N);
        case STATEMENT_SIMPLE_TYPE_YIELD:
            codegen_instruction_YIELD(N);
            codegen_visit_return(N);
        case STATEMENT_SIMPLE_TYPE_LEAVE: {
            struct codegen_argument_index constant = codegen_constant_nil(N);
            struct codegen_argument_slot temp = codegen_temporary(N);
            codegen_instruction_LOAD(N, constant, temp);
            codegen_instruction_LEAVE(N, temp);
            codegen_visit_return(N);
        }
        case STATEMENT_SIMPLE_TYPE_BREAK:
            codegen_instruction_JUMP(N, codegen_break_get(N));
            codegen_visit_return(N);
        case STATEMENT_SIMPLE_TYPE_CONTINUE:
            codegen_instruction_JUMP(N, codegen_continue_get(N));
            codegen_visit_return(N);
    }
}

gen_func_dec(statement, eval) {
    switch (state) {
        case 0: {
            struct codegen_argument_slot slot = codegen_temporary(N);
            codegen_visit_expression(N, eval->expression, slot, 1);
        }
        case 1:
            codegen_visit_return(N);
        default:
            break;
    }
}

struct gen_return_data {
    struct codegen_argument_slot slot;
};

gen_func_dec_named(statement, return, ret) {
    struct gen_return_data *data;
    codegen_save(N, sizeof(struct gen_return_data), (void **) &data);

    switch (state) {
        case 0: {
            data->slot = codegen_temporary(N);
            codegen_visit_expression(N, ret->expression, data->slot, 1);
        }
        case 1:
            codegen_instruction_LEAVE(N, data->slot);
            codegen_visit_return(N);
        default:
            break;
    }
}

struct gen_while_data {
    struct codegen_argument_anchor anchor;
    struct codegen_argument_slot slot;
};

gen_func_dec_named(statement, while, wh) {
    struct gen_while_data *data;
    codegen_save(N, sizeof(struct gen_while_data), (void **) &data);

    if (wh->first_condition) {
        switch (state) {
            case 0: {
                data->slot = codegen_temporary(N);

                codegen_scope_enter(N);
                codegen_init_break_continue(N);
                codegen_visit_expression(N, wh->condition, data->slot, 1);
            }
            case 1: {
                struct codegen_argument_anchor anchor = codegen_anchor(N);
                codegen_instruction_JUMP_IF(
                    N, data->slot, anchor, codegen_break_get(N)
                );

                codegen_anchor_change(N, anchor);
                codegen_visit_statement(N, wh->statement, 2);
            }
            case 2: {
                codegen_instruction_JUMP(N, codegen_continue_get(N));

                codegen_break_change(N);
                codegen_scope_exit(N);
                codegen_visit_return(N);
            }
            default:
                break;
        }
    } else {
        switch (state) {
            case 0: {
                data->anchor = codegen_anchor(N);
                data->slot = codegen_temporary(N);

                codegen_scope_enter(N);
                codegen_init_break_continue(N);
                codegen_anchor_change(N, data->anchor);
                codegen_visit_statement(N, wh->statement, 1);
            }
            case 1: {
                codegen_continue_change(N);
                codegen_visit_expression(N, wh->condition, data->slot, 2);
            }
            case 2: {
                codegen_instruction_JUMP_IF(
                    N, data->slot, data->anchor, codegen_break_get(N)
                );

                codegen_break_change(N);
                codegen_scope_exit(N);
                codegen_visit_return(N);
            }
            default:
                break;
        }
    }
}

struct gen_for_data {
    struct codegen_argument_slot slot;
};

gen_func_dec_named(statement, for, fr) {
    struct gen_for_data *data;
    codegen_save(N, sizeof(struct gen_for_data), (void **) &data);

    switch (state) {
        case 0: {
            data->slot = codegen_temporary(N);

            codegen_scope_enter(N);
            codegen_init_break_continue(N);
            codegen_visit_statement(N, fr->initial, 1);
        }
        case 1: {
            codegen_continue_change(N);
            codegen_visit_expression(N, fr->condition, data->slot, 2);
        }
        case 2: {
            struct codegen_argument_anchor anchor = codegen_anchor(N);
            codegen_instruction_JUMP_IF(
                N, data->slot, anchor, codegen_break_get(N)
            );

            codegen_anchor_change(N, anchor);
            codegen_visit_statement(N, fr->statement, 3);
        }
        case 3: {
            codegen_visit_statement(N, fr->increment, 4);
        }
        case 4: {
            codegen_instruction_JUMP(N, codegen_continue_get(N));

            codegen_break_change(N);
            codegen_scope_exit(N);
            codegen_visit_return(N);
        }
        default:
            break;
    }
}

struct gen_iterator_data {
    size_t index;
    struct codegen_argument_slot slot;
    struct codegen_argument_slot temp;
    struct codegen_argument_slot container;
};

gen_func_dec(statement, iterator) {
    struct gen_iterator_data *data;
    if (codegen_save(N, sizeof(struct gen_iterator_data), (void **) &data)) {
        data->index = 0;
    }

    switch (state) {
        case 0: {
            data->slot = codegen_temporary(N);

            codegen_scope_enter(N);
            codegen_init_break_continue(N);
            codegen_visit_expression(N, iterator->expression, data->slot, 1);
        }
        case 1: {
            struct codegen_argument_anchor anchor = codegen_anchor(N);
            data->container = codegen_temporary(N);
            codegen_instruction_ITERATOR(N, data->slot, data->slot);
            codegen_instruction_ITERATOR_INIT(N, data->slot);

            codegen_continue_change(N);
            codegen_instruction_ITERATOR_HAS(N, data->slot, data->container);

            codegen_instruction_JUMP_IF(
                N, data->container, anchor, codegen_break_get(N)
            );

            codegen_anchor_change(N, anchor);
            codegen_instruction_ITERATOR_NEXT(N, data->slot, data->container);

            if (iterator->size == 0) {
                codegen_declare_variable(N, iterator->name, false);
                set_variable(N, iterator->name, true, data->container, NULL);

                codegen_visit_next(N, 4);
            } else {
                for (size_t i = 0; i < iterator->size; i++) {
                    codegen_declare_variable(N, iterator->multi.names[i], false);
                }

                data->temp = codegen_temporary(N);
                codegen_visit_next(N, 2);
            }
        }
        case 2: {
            if (data->index < iterator->size) {
                codegen_visit_expression(N, iterator->multi.keys[data->index], data->temp, 3);
            } else {
                codegen_visit_next(N, 4);
            }
        }
        case 3: {
            codegen_instruction_GET(N, data->container, data->temp, data->temp);
            set_variable(N, iterator->multi.names[data->index], true, data->temp, NULL);

            data->index++;
            codegen_visit_next(N, 2);
        }
        case 4: {
            codegen_visit_statement(N, iterator->statement, 5);
        }
        case 5: {
            codegen_instruction_JUMP(N, codegen_continue_get(N));

            codegen_break_change(N);
            codegen_scope_exit(N);
            codegen_visit_return(N);
        }
        default:
            break;
    }
}

struct gen_sif_data {
    size_t index;
    struct codegen_argument_slot slot;
    struct codegen_argument_anchor anchor_if;
    struct codegen_argument_anchor anchor_else;
    struct codegen_argument_anchor anchor_exit;
};

gen_func_dec_named(statement, if, sif) {
    struct gen_sif_data *data;
    if (codegen_save(N, sizeof(struct gen_sif_data), (void **) &data)) {
        data->index = 0;
    }

    switch (state) {
        case 0: {
            data->slot = codegen_temporary(N);
            data->anchor_if = codegen_anchor(N);
            data->anchor_else = codegen_anchor(N);
            data->anchor_exit = codegen_anchor(N);

            codegen_visit_expression(N, sif->if_condition, data->slot, 1);
        }
        case 1: {
            codegen_instruction_JUMP_IF(N, data->slot, data->anchor_if, data->anchor_else);
            codegen_anchor_change(N, data->anchor_if);
            codegen_visit_statement(N, sif->if_statement, 2);
        }
        case 2: {
            codegen_instruction_JUMP(N, data->anchor_exit);
            codegen_anchor_change(N, data->anchor_else);

            if (data->index < sif->size) {
                data->anchor_if = codegen_anchor(N);
                data->anchor_else = codegen_anchor(N);

                codegen_visit_expression(N, sif->elif_conditions[data->index], data->slot, 3);
            } else if (sif->else_statement != NULL) {
                codegen_visit_statement(N, sif->else_statement, 4);
            } else {
                codegen_visit_next(N, 4);
            }
        }
        case 3: {
            codegen_instruction_JUMP_IF(N, data->slot, data->anchor_if, data->anchor_else);
            codegen_anchor_change(N, data->anchor_if);

            size_t index = data->index;
            data->index++;
            codegen_visit_statement(N, sif->elif_statements[index], 2);
        }
        case 4: {
            codegen_anchor_change(N, data->anchor_exit);
            codegen_visit_return(N);
        }
        default:
            break;
    }
}

struct gen_declaration_data {
    size_t index;
    struct codegen_argument_slot slot;
    struct codegen_argument_slot temp;
};

gen_func_dec(statement, declaration) {
    struct gen_declaration_data *data;
    if (codegen_save(N, sizeof(struct gen_declaration_data), (void **) &data)) {
        data->index = 0;
    }

    switch (state) {
        case 0: {
            data->slot = codegen_temporary(N);
            codegen_visit_expression(N, declaration->expression, data->slot, 1);
        }
        case 1: {
            if (declaration->size == 0) {
                codegen_declare_variable(N, declaration->name, declaration->mutable);
                set_variable(N, declaration->name, true, data->slot, NULL);

                codegen_visit_return(N);
            } else {
                for (size_t i = 0; i < declaration->size; i++) {
                    codegen_declare_variable(N, declaration->multi.names[i], declaration->mutable);
                }

                data->temp = codegen_temporary(N);
                codegen_visit_next(N, 2);
            }
        }
        case 2: {
            if (data->index < declaration->size) {
                codegen_visit_expression(N, declaration->multi.keys[data->index], data->temp, 3);
            } else {
                codegen_visit_return(N);
            }
        }
        case 3: {
            codegen_instruction_GET(N, data->slot, data->temp, data->temp);
            set_variable(N, declaration->multi.names[data->index], true, data->temp, NULL);

            data->index++;
            codegen_visit_next(N, 2);
        }
        default:
            break;
    }
}

struct gen_assigment_data {
    size_t index;
    struct codegen_argument_slot slot;
    struct codegen_argument_slot temp;
    struct codegen_argument_slot container;
    struct codegen_argument_slot key;
};

gen_func_dec(statement, assigment) {
    struct gen_assigment_data *data;
    if (codegen_save(N, sizeof(struct gen_assigment_data), (void **) &data)) {
        data->index = 0;
    }

    switch (state) {
        case 0: {
            data->slot = codegen_temporary(N);
            codegen_visit_expression(N, assigment->expression, data->slot, 1);
        }
        case 1: {
            data->key = codegen_temporary(N);
            data->container = codegen_temporary(N);
            if (assigment->size == 0) {
                codegen_visit_next(N, 2);
            } else {
                data->temp = codegen_temporary(N);
                codegen_visit_next(N, 5);
            }
        }
        case 2:
        case 3:
        case 4: {
            set(N, state, 2, assigment->container, data->slot, &data->container, &data->key);
            codegen_visit_return(N);
        }
        case 5: {
            if (data->index < assigment->size) {
                codegen_visit_expression(N, assigment->multi.keys[data->index], data->temp, 6);
            } else {
                codegen_visit_return(N);
            }
        }
        case 6: {
            codegen_instruction_GET(N, data->slot, data->temp, data->temp);
            codegen_visit_next(N, 7);
        }
        case 7:
        case 8:
        case 9: {
            set(
                N, state, 7,
                assigment->multi.containers[data->index],
                data->temp,
                &data->container,
                &data->key
            );

            data->index++;
            codegen_visit_next(N, 5);
        }
        default:
            break;
    }
}

gen_func_dec(expression, value) {
    (void) state;

    switch (value->type) {
        case EXPRESSION_VALUE_TYPE_NIL: {
            struct codegen_argument_index constant = codegen_constant_nil(N);
            codegen_instruction_LOAD(N, constant, codegen_result(N));
            codegen_visit_return(N);
        }
        case EXPRESSION_VALUE_TYPE_INT: {
            struct codegen_argument_index constant = codegen_constant_int(N, value->value.integer);
            codegen_instruction_LOAD(N, constant, codegen_result(N));
            codegen_visit_return(N);
        }
        case EXPRESSION_VALUE_TYPE_DEC: {
            struct codegen_argument_index constant = codegen_constant_dec(N, value->value.decimal);
            codegen_instruction_LOAD(N, constant, codegen_result(N));
            codegen_visit_return(N);
        }
        case EXPRESSION_VALUE_TYPE_STR: {
            struct codegen_argument_index constant = codegen_constant_str(N, value->value.string);
            codegen_instruction_LOAD(N, constant, codegen_result(N));
            codegen_visit_return(N);
        }
        case EXPRESSION_VALUE_TYPE_BOOL: {
            struct codegen_argument_index constant = codegen_constant_bool(N, value->value.boolean);
            codegen_instruction_LOAD(N, constant, codegen_result(N));
            codegen_visit_return(N);
        }
    }
}

struct gen_binary_data {
    struct codegen_argument_slot slot;
};

gen_func_dec(expression, binary) {
    struct gen_binary_data *data;
    codegen_save(N, sizeof(struct gen_binary_data), (void **) &data);

    switch (state) {
        case 0: {
            codegen_visit_expression(N, binary->a, codegen_result(N), 1);
        }
        case 1: {
            data->slot = codegen_temporary(N);
            codegen_visit_expression(N, binary->b, data->slot, 2);
        }
        case 2: {
            switch (binary->type) {
                case EXPRESSION_BINARY_TYPE_ADD:
                    codegen_instruction_ADD(N, codegen_result(N), data->slot, codegen_result(N));
                    codegen_visit_return(N);
                case EXPRESSION_BINARY_TYPE_SUB:
                    codegen_instruction_SUB(N, codegen_result(N), data->slot, codegen_result(N));
                    codegen_visit_return(N);
                case EXPRESSION_BINARY_TYPE_MUL:
                    codegen_instruction_MUL(N, codegen_result(N), data->slot, codegen_result(N));
                    codegen_visit_return(N);
                case EXPRESSION_BINARY_TYPE_DIV:
                    codegen_instruction_DIV(N, codegen_result(N), data->slot, codegen_result(N));
                    codegen_visit_return(N);
                case EXPRESSION_BINARY_TYPE_MOD:
                    codegen_instruction_MOD(N, codegen_result(N), data->slot, codegen_result(N));
                    codegen_visit_return(N);
                case EXPRESSION_BINARY_TYPE_EQUAL:
                    codegen_instruction_EQUAL(N, codegen_result(N), data->slot, codegen_result(N));
                    codegen_visit_return(N);
                case EXPRESSION_BINARY_TYPE_LESS:
                    codegen_instruction_LESS(N, codegen_result(N), data->slot, codegen_result(N));
                    codegen_visit_return(N);
                case EXPRESSION_BINARY_TYPE_CONCAT:
                    codegen_instruction_CONCAT(N, codegen_result(N), data->slot, codegen_result(N));
                    codegen_visit_return(N);
                case EXPRESSION_BINARY_TYPE_AND:
                    codegen_instruction_AND(N, codegen_result(N), data->slot, codegen_result(N));
                    codegen_visit_return(N);
                case EXPRESSION_BINARY_TYPE_OR:
                    codegen_instruction_OR(N, codegen_result(N), data->slot, codegen_result(N));
                    codegen_visit_return(N);
            }
        }
        default:
            break;
    }
}

gen_func_dec(expression, unary) {
    switch (state) {
        case 0: {
            codegen_visit_expression(N, unary->expression, codegen_result(N), 1);
        }
        case 1: {
            switch (unary->type) {
                case EXPRESSION_UNARY_TYPE_NEGATE:
                    codegen_instruction_NEGATIVE(N, codegen_result(N), codegen_result(N));
                    codegen_visit_return(N);
                case EXPRESSION_UNARY_TYPE_NOT:
                    codegen_instruction_NOT(N, codegen_result(N), codegen_result(N));
                    codegen_visit_return(N);
                case EXPRESSION_UNARY_TYPE_TYPE:
                    codegen_instruction_TYPE(N, codegen_result(N), codegen_result(N));
                    codegen_visit_return(N);
                case EXPRESSION_UNARY_TYPE_LEN:
                    codegen_instruction_LENGTH(N, codegen_result(N), codegen_result(N));
                    codegen_visit_return(N);
                case EXPRESSION_UNARY_TYPE_REF:
                    codegen_instruction_REF(N, codegen_result(N), codegen_result(N));
                    codegen_visit_return(N);
                case EXPRESSION_UNARY_TYPE_DEREF:
                    codegen_instruction_DEREF(N, codegen_result(N), codegen_result(N));
                    codegen_visit_return(N);
            }
        }
        default:
            break;
    }
}

struct gen_increment_data {
    struct codegen_argument_slot slot;
    struct codegen_argument_slot container;
    struct codegen_argument_slot key;
};

gen_func_dec(expression, increment) {
    struct gen_increment_data *data;
    codegen_save(N, sizeof(struct gen_increment_data), (void **) &data);

    if (increment->is_postfix) {
        switch (state) {
            case 0: {
                codegen_visit_expression(N, increment->container, codegen_result(N), 1);
            }
            case 1: {
                data->slot = codegen_temporary(N);
                data->container = codegen_temporary(N);
                data->key = codegen_temporary(N);
                struct codegen_argument_index constant = codegen_constant_int(N, 1);
                codegen_instruction_LOAD(N, constant, data->slot);

                if (increment->is_decrement) {
                    codegen_instruction_SUB(N, codegen_result(N), data->slot, data->slot);
                } else {
                    codegen_instruction_ADD(N, codegen_result(N), data->slot, data->slot);
                }

                codegen_visit_next(N, 2);
            }
            case 2:
            case 3:
            case 4: {
                set(N, state, 2, increment->container, data->slot, &data->container, &data->key);
                codegen_visit_return(N);
            }
            default:
                break;
        }
    } else {
        switch (state) {
            case 0: {
                codegen_visit_expression(N, increment->container, codegen_result(N), 1);
            }
            case 1: {
                data->slot = codegen_temporary(N);
                data->container = codegen_temporary(N);
                data->key = codegen_temporary(N);
                struct codegen_argument_index constant = codegen_constant_int(N, 1);
                codegen_instruction_LOAD(N, constant, data->slot);

                if (increment->is_decrement) {
                    codegen_instruction_SUB(N, codegen_result(N), data->slot, codegen_result(N));
                } else {
                    codegen_instruction_ADD(N, codegen_result(N), data->slot, codegen_result(N));
                }

                codegen_visit_next(N, 2);
            }
            case 2:
            case 3:
            case 4: {
                set(N, state, 2, increment->container, codegen_result(N), &data->container, &data->key);
                codegen_visit_return(N);
            }
            default:
                break;
        }
    }
}

gen_func_dec(expression, variable) {
    (void) state;
    get_variable(N, variable->index, codegen_result(N));
    codegen_visit_return(N);
}

gen_func_dec(expression, global) {
    (void) state;

    switch (global->type) {
        case EXPRESSION_GLOBAL_TYPE_ENV:
            codegen_instruction_ENV(N, codegen_result(N));
            codegen_visit_return(N);
        case EXPRESSION_GLOBAL_TYPE_SELF:
            codegen_instruction_SELF(N, codegen_result(N));
            codegen_visit_return(N);
    }
}

struct gen_binary_table {
    size_t index;
    struct codegen_argument_slot key;
    struct codegen_argument_slot value;
};

gen_func_dec(expression, table) {
    struct gen_binary_table *data;
    if (codegen_save(N, sizeof(struct gen_binary_table), (void **) &data)) {
        data->index = 0;
    }

    switch (state) {
        case 0: {
            codegen_instruction_TABLE(N, codegen_result(N));
            data->key = codegen_temporary(N);
            data->value = codegen_temporary(N);
            codegen_visit_next(N, 1);
        }
        case 1: {
            if (data->index < table->size) {
                codegen_visit_expression(N, table->keys[data->index], data->key, 2);
            } else {
                codegen_visit_return(N);
            }
        }
        case 2: {
            codegen_visit_expression(N, table->values[data->index], data->value, 3);
        }
        case 3: {
            codegen_instruction_SET(N, codegen_result(N), data->key, data->value);

            data->index++;
            codegen_visit_next(N, 1);
        }
        default:
            break;
    }
}

struct gen_binary_vector {
    size_t index;
    struct codegen_argument_slot key;
    struct codegen_argument_slot value;
};

gen_func_dec(expression, vector) {
    struct gen_binary_vector *data;
    if (codegen_save(N, sizeof(struct gen_binary_vector), (void **) &data)) {
        data->index = 0;
    }

    switch (state) {
        case 0: {
            struct codegen_argument_count count = {
                .count = vector->size
            };

            codegen_instruction_VECTOR(N, codegen_result(N), count);
            data->key = codegen_temporary(N);
            data->value = codegen_temporary(N);
            codegen_visit_next(N, 1);
        }
        case 1: {
            if (data->index < vector->size) {
                codegen_visit_expression(N, vector->values[data->index], data->value, 2);
            } else {
                codegen_visit_return(N);
            }
        }
        case 2: {
            if (data->index > MLIMIT_INTEGER_MAX) {
                codegen_error(N, "vector too big");
            }

            struct codegen_argument_index constant = codegen_constant_int(N, (ml_integer) data->index);
            codegen_instruction_LOAD(N, constant, data->key);
            codegen_instruction_SET(N, codegen_result(N), data->key, data->value);

            data->index++;
            codegen_visit_next(N, 1);
        }
        default:
            break;
    }
}

struct gen_access_vector {
    struct codegen_argument_slot slot;
};

gen_func_dec(expression, access) {
    struct gen_access_vector *data;
    codegen_save(N, sizeof(struct gen_access_vector), (void **) &data);

    switch (state) {
        case 0:
            codegen_visit_expression(N, access->container, codegen_result(N), 1);
        case 1: {
            data->slot = codegen_temporary(N);
            codegen_visit_expression(N, access->key, data->slot, 2);
        }
        case 2:
            codegen_instruction_GET(N, codegen_result(N), data->slot, codegen_result(N));
            codegen_visit_return(N);
        default:
            break;
    }
}

struct gen_call_data {
    size_t index;
    struct codegen_argument_slot *args;
};

gen_func_dec(expression, call) {
    struct gen_call_data *data;
    size_t size = sizeof(struct gen_call_data) + sizeof(struct codegen_argument_slot) * call->args_size;

    if (codegen_save(N, size, (void **) &data)) {
        data->index = 0;
        data->args = ((void *) data) + sizeof(struct gen_call_data);
    }

    switch (state) {
        case 0: {
            for (size_t i = 0; i < call->args_size; i++) {
                data->args[i] = codegen_temporary(N);
            }

            codegen_visit_expression(N, call->expression, codegen_result(N), 1);
        }
        case 1: {
            if (data->index < call->args_size) {
                size_t index = data->index;
                data->index++;
                codegen_visit_expression(N, call->arguments[index], data->args[index], 1);
            } else {
                codegen_visit_next(N, 2);
            }
        }
        case 2: {
            for (size_t i = 0; i < call->args_size; i++) {
                struct codegen_argument_index index = {
                    .index = i
                };

                codegen_instruction_PARAM(N, data->args[i], index);
            }

            struct codegen_argument_count count = {
                .count = call->args_size
            };

            codegen_instruction_CALL(N, codegen_result(N), count);
            codegen_instruction_RESULT(N, codegen_result(N));
            codegen_visit_return(N);
        }
        default:
            break;
    }
}

struct gen_call_self_data {
    size_t index;
    struct codegen_argument_slot slot;
    struct codegen_argument_slot *args;
};

gen_func_dec(expression, call_self) {
    struct gen_call_self_data *data;
    size_t size = sizeof(struct gen_call_self_data) +
                  sizeof(struct codegen_argument_slot) * call_self->args_size;

    if (codegen_save(N, size, (void **) &data)) {
        data->index = 0;
        data->args = ((void *) data) + sizeof(struct gen_call_self_data);
    }

    switch (state) {
        case 0: {
            data->slot = codegen_temporary(N);
            codegen_visit_expression(N, call_self->self, codegen_result(N), 1);
        }
        case 1:
            codegen_visit_expression(N, call_self->callable, data->slot, 2);
        case 2: {
            if (call_self->extract_callable) {
                codegen_instruction_GET(N, codegen_result(N), data->slot, data->slot);
            }

            for (size_t i = 0; i < call_self->args_size; i++) {
                data->args[i] = codegen_temporary(N);
            }

            codegen_visit_next(N, 3);
        }
        case 3: {
            if (data->index < call_self->args_size) {
                size_t index = data->index;
                data->index++;
                codegen_visit_expression(N, call_self->arguments[index], data->args[index], 3);
            } else {
                codegen_visit_next(N, 4);
            }
        }
        case 4: {
            for (size_t i = 0; i < call_self->args_size; i++) {
                struct codegen_argument_index index = {
                    .index = i
                };

                codegen_instruction_PARAM(N, data->args[i], index);
            }

            struct codegen_argument_count count = {
                .count = call_self->args_size
            };

            codegen_instruction_SCALL(N, data->slot, count, codegen_result(N));
            codegen_instruction_RESULT(N, codegen_result(N));
            codegen_visit_return(N);
        }
        default:
            break;
    }
}

gen_func_dec(expression, function) {
    switch (state) {
        case 0:
            codegen_visit_function(N, function->ref, 1);
        case 1: {
            struct codegen_argument_index constant = codegen_constant_fun(N, function->ref);
            codegen_instruction_LOAD(N, constant, codegen_result(N));

            size_t size;
            struct codegen_closure *closures;
            codegen_closure(N, function->ref, &size, &closures);

            if (size > 0) {
                struct codegen_argument_slot temp = codegen_temporary(N);
                for (size_t i = 0; i < size; i++) {
                    get_variable(N, closures[i].index, temp);

                    struct codegen_argument_index index = {
                        .index = i
                    };

                    codegen_instruction_PARAM(N, temp, index);
                }

                struct codegen_argument_count count = {
                    .count = size
                };

                codegen_instruction_CLOSURE(N, codegen_result(N), count, codegen_result(N));
            }

            codegen_visit_return(N);
        }
        default:
            break;
    }
}

struct gen_eblock_data {
    size_t index;
};

gen_func_dec(expression, block) {
    struct gen_eblock_data *data;
    if (codegen_save(N, sizeof(struct gen_eblock_data), (void **) &data)) {
        data->index = 0;
    }

    switch (state) {
        case 0: {
            codegen_scope_enter(N);
            codegen_visit_next(N, 1);
        }
        case 1: {
            if (data->index < block->size) {
                size_t index = data->index;
                data->index++;
                codegen_visit_statement(N, block->statements[index], 1);
            } else {
                codegen_visit_next(N, 2);
            }
        }
        case 2:
            codegen_visit_expression(N, block->result->expression, codegen_result(N), 3);
        case 3:
            codegen_scope_exit(N);
            codegen_visit_return(N);
        default:
            break;
    }
}

struct gen_eif_data {
    size_t index;
    struct codegen_argument_anchor anchor_if;
    struct codegen_argument_anchor anchor_else;
    struct codegen_argument_anchor anchor_exit;
};

gen_func_dec_named(expression, if, eif) {
    struct gen_eif_data *data;
    if (codegen_save(N, sizeof(struct gen_eif_data), (void **) &data)) {
        data->index = 0;
    }

    switch (state) {
        case 0: {
            data->anchor_if = codegen_anchor(N);
            data->anchor_else = codegen_anchor(N);
            data->anchor_exit = codegen_anchor(N);

            codegen_visit_expression(N, eif->if_condition, codegen_result(N), 1);
        }
        case 1: {
            codegen_instruction_JUMP_IF(N, codegen_result(N), data->anchor_if, data->anchor_else);
            codegen_anchor_change(N, data->anchor_if);
            codegen_visit_expression(N, eif->if_expression, codegen_result(N), 2);
        }
        case 2: {
            codegen_instruction_JUMP(N, data->anchor_exit);
            codegen_anchor_change(N, data->anchor_else);

            if (data->index < eif->size) {
                data->anchor_if = codegen_anchor(N);
                data->anchor_else = codegen_anchor(N);
                codegen_visit_expression(N, eif->elif_conditions[data->index], codegen_result(N), 3);
            } else {
                codegen_visit_expression(N, eif->else_expression, codegen_result(N), 4);
            }
        }
        case 3: {
            codegen_instruction_JUMP_IF(N, codegen_result(N), data->anchor_if, data->anchor_else);
            codegen_anchor_change(N, data->anchor_if);

            size_t index = data->index;
            data->index++;
            codegen_visit_expression(N, eif->elif_expressions[index], codegen_result(N), 2);
        }
        case 4: {
            codegen_anchor_change(N, data->anchor_exit);
            codegen_visit_return(N);
        }
        default:
            break;
    }
}