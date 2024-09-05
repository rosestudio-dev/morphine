//
// Created by why-iskra on 05.09.2024.
//

#include "morphinec/serializer.h"
#include "serializer/controller.h"
#include "serializer/implementation.h"

struct serializer_controller {
    morphine_coroutine_t U;
    struct mc_strtable *T;
    struct mc_ast *A;
    struct mc_visitor_controller *VC;

    struct {
        struct mc_ast_node *node;
        struct mc_ast_function *function;
    } callback;
};

static morphine_noret void serializer_enter_node(
    struct serializer_controller *C,
    struct mc_ast_node *node,
    size_t next_state
) {
    C->callback.node = node;
    mcapi_visitor_node(C->VC, node, next_state);
}

morphine_noret void serializer_enter_expression(
    struct serializer_controller *C,
    struct mc_ast_expression *expression,
    size_t next_state
) {
    serializer_enter_node(C, mcapi_ast_expression2node(expression), next_state);
}

morphine_noret void serializer_enter_statement(
    struct serializer_controller *C,
    struct mc_ast_statement *statement,
    size_t next_state
) {
    serializer_enter_node(C, mcapi_ast_statement2node(statement), next_state);
}

morphine_noret void serializer_enter_function(
    struct serializer_controller *C,
    struct mc_ast_function *function,
    size_t next_state
) {
    C->callback.function = function;
    mcapi_visitor_function(C->VC, function, next_state);
}

morphine_noret void serializer_jump(
    struct serializer_controller *C,
    size_t next_state
) {
    mcapi_visitor_jump(C->VC, next_state);
}

morphine_noret void serializer_complete(struct serializer_controller *C) {
    mcapi_visitor_complete(C->VC);
}

void *serializer_saved(struct serializer_controller *C) {
    return mcapi_visitor_saved(C->VC);
}

void *serializer_alloc_saved_uni(struct serializer_controller *C, size_t size) {
    return mcapi_visitor_alloc_saved_uni(C->VC, size);
}

void serializer_push_string(struct serializer_controller *C, mc_strtable_index_t index) {
    struct mc_strtable_entry entry = mcapi_strtable_access(C->U, C->T, index);
    mapi_push_stringn(C->U, entry.string, entry.size);
}

void serializer_push_cstr(struct serializer_controller *C, const char *value) {
    mapi_push_string(C->U, value);
}

void serializer_push_integer(struct serializer_controller *C, ml_integer value) {
    mapi_push_integer(C->U, value);
}

void serializer_push_decimal(struct serializer_controller *C, ml_decimal value) {
    mapi_push_decimal(C->U, value);
}

void serializer_push_boolean(struct serializer_controller *C, bool value) {
    mapi_push_boolean(C->U, value);
}

void serializer_push_size(struct serializer_controller *C, size_t value, const char *name) {
    mapi_push_size(C->U, value, name);
}

void serializer_push_vector(struct serializer_controller *C, size_t size) {
    mapi_push_vector(C->U, mapi_csize2size(C->U, size, NULL));
}

void serializer_vector_set(struct serializer_controller *C, size_t index) {
    mapi_vector_set(C->U, mapi_csize2size(C->U, index, "index"));
}

void serializer_push_table(struct serializer_controller *C) {
    mapi_push_table(C->U);
}

void serializer_set(struct serializer_controller *C, const char *key) {
    maux_table_set(C->U, key);
}

#define expr_case(n) case MCEXPRT_##n: serialize_expression_##n(C, mcapi_ast_expression2##n(C->U, expression), state); break;
#define stmt_case(n) case MCSTMTT_##n: serialize_statement_##n(C, mcapi_ast_statement2##n(C->U, statement), state); break;

static void visitor_function(
    struct mc_visitor_controller *VC,
    struct mc_ast_node *node,
    size_t state
) {
    struct serializer_controller *C = mcapi_visitor_data(VC);
    C->VC = VC;

    switch (node->type) {
        case MCANT_STATEMENT: {
            struct mc_ast_statement *statement =
                mcapi_ast_node2statement(C->U, node);

            switch (statement->type) {
                stmt_case(block)
                stmt_case(simple)
                stmt_case(eval)
                stmt_case(leave)
                stmt_case(while)
                stmt_case(for)
                stmt_case(iterator)
                stmt_case(declaration)
                stmt_case(assigment)
                stmt_case(if)
            }
            break;
        }
        case MCANT_EXPRESSION: {
            struct mc_ast_expression *expression =
                mcapi_ast_node2expression(C->U, node);

            switch (expression->type) {
                expr_case(value)
                expr_case(binary)
                expr_case(unary)
                expr_case(increment)
                expr_case(variable)
                expr_case(global)
                expr_case(table)
                expr_case(vector)
                expr_case(access)
                expr_case(call)
                expr_case(function)
                expr_case(block)
                expr_case(if)
            }
            break;
        }
    }
}

MORPHINE_API bool mcapi_serializer_step(
    morphine_coroutine_t U,
    struct mc_visitor *V,
    struct mc_strtable *T,
    struct mc_ast *A
) {
    struct serializer_controller controller = {
        .U = U,
        .T = T,
        .A = A,
        .VC = NULL,
        .callback.node = NULL,
        .callback.function = NULL,
    };

    enum mc_visitor_event event = MCVE_INIT;
    bool result = mcapi_visitor_step(
        U, V, A, visitor_function, &event, &controller
    );

    switch (event) {
        case MCVE_INIT: {
            controller.callback.function = mcapi_ast_get_root_function(A);
        }
        case MCVE_ENTER_FUNCTION: {
            struct mc_ast_function *function = controller.callback.function;

            mapi_push_table(U);

            mapi_push_size(U, function->line, "line");
            maux_table_set(U, "line");

            mapi_push_boolean(U, function->recursive);
            maux_table_set(U, "recursive");

            mapi_push_boolean(U, function->anonymous);
            maux_table_set(U, "anonymous");

            if (!function->anonymous) {
                serializer_push_string(&controller, function->name);
                maux_table_set(U, "name");
            }

            mapi_push_boolean(U, function->auto_closure);
            maux_table_set(U, "autoclosure");

            if (!function->auto_closure) {
                ml_size closures_size = mapi_csize2size(U, function->closures_size, NULL);
                mapi_push_vector(U, closures_size);
                for (ml_size i = 0; i < closures_size; i++) {
                    serializer_push_string(&controller, function->closures[i]);
                    mapi_vector_set(U, i);
                }
                maux_table_set(U, "closures");
            }

            ml_size args_size = mapi_csize2size(U, function->args_size, NULL);
            mapi_push_vector(U, args_size);
            for (ml_size i = 0; i < args_size; i++) {
                serializer_push_string(&controller, function->arguments[i]);
                mapi_vector_set(U, i);
            }
            maux_table_set(U, "arguments");

            ml_size statics_size = mapi_csize2size(U, function->statics_size, NULL);
            mapi_push_vector(U, statics_size);
            for (ml_size i = 0; i < statics_size; i++) {
                serializer_push_string(&controller, function->statics[i]);
                mapi_vector_set(U, i);
            }
            maux_table_set(U, "statics");

            controller.callback.node = mcapi_ast_statement2node(function->body);
        }
        case MCVE_ENTER_NODE: {
            mapi_push_table(U);
            mapi_push_string(U, mcapi_ast_type_name(U, controller.callback.node));
            maux_table_set(U, "type");

            mapi_push_table(U);
            break;
        }
        case MCVE_DROP_FUNCTION: {
            maux_table_set(U, "data");
            maux_table_set(U, "body");
            break;
        }
        case MCVE_DROP_NODE: {
            maux_table_set(U, "data");
            break;
        }
        case MCVE_NEXT_STEP:
        case MCVE_COMPLETE:
            break;
    }

    return result;
}
