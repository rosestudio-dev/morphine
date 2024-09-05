//
// Created by why-iskra on 27.05.2024.
//

#include <string.h>
#include "morphinec/ast.h"
#include "ast/creator.h"

struct mc_ast {
    struct mc_ast_node *nodes;
    struct mc_ast_function *functions;
    struct mc_ast_function *root_function;
};

// api

static void ast_userdata_init(morphine_instance_t I, void *data) {
    (void) I;

    struct mc_ast *A = data;
    *A = (struct mc_ast) {
        .nodes = NULL,
        .functions = NULL,
        .root_function = NULL
    };
}

static void ast_userdata_free(morphine_instance_t I, void *data) {
    struct mc_ast *A = data;

    struct mc_ast_node *node = A->nodes;
    while (node != NULL) {
        struct mc_ast_node *prev = node->prev;
        mapi_allocator_free(I, node);
        node = prev;
    }

    struct mc_ast_function *function = A->functions;
    while (function != NULL) {
        struct mc_ast_function *prev = function->prev;
        mapi_allocator_free(I, function->closures);
        mapi_allocator_free(I, function->arguments);
        mapi_allocator_free(I, function->statics);
        mapi_allocator_free(I, function);
        function = prev;
    }
}

MORPHINE_API struct mc_ast *mcapi_push_ast(morphine_coroutine_t U) {
    mapi_type_declare(
        mapi_instance(U),
        MC_AST_USERDATA_TYPE,
        sizeof(struct mc_ast),
        ast_userdata_init,
        ast_userdata_free,
        false
    );

    return mapi_push_userdata(U, MC_AST_USERDATA_TYPE);
}

MORPHINE_API struct mc_ast *mcapi_get_ast(morphine_coroutine_t U) {
    return mapi_userdata_pointer(U, MC_AST_USERDATA_TYPE);
}

MORPHINE_API struct mc_ast_expression *mcapi_ast_node2expression(
    morphine_coroutine_t U,
    struct mc_ast_node *node
) {
    if (node->type != MCANT_EXPRESSION) {
        mapi_error(U, "expected expression");
    }

    return (struct mc_ast_expression *) node;
}

MORPHINE_API struct mc_ast_statement *mcapi_ast_node2statement(
    morphine_coroutine_t U,
    struct mc_ast_node *node
) {
    if (node->type != MCANT_STATEMENT) {
        mapi_error(U, "expected statement");
    }

    return (struct mc_ast_statement *) node;
}

MORPHINE_API const char *mcapi_ast_type_name(morphine_coroutine_t U, struct mc_ast_node *node) {
    switch (node->type) {
        case MCANT_EXPRESSION: {
            struct mc_ast_expression *expression = mcapi_ast_node2expression(U, node);
            switch (expression->type) {
                case MCEXPRT_value:
                    return "expression_value";
                case MCEXPRT_binary:
                    return "expression_binary";
                case MCEXPRT_unary:
                    return "expression_unary";
                case MCEXPRT_increment:
                    return "expression_increment";
                case MCEXPRT_variable:
                    return "expression_variable";
                case MCEXPRT_global:
                    return "expression_global";
                case MCEXPRT_leave:
                    return "expression_leave";
                case MCEXPRT_table:
                    return "expression_table";
                case MCEXPRT_vector:
                    return "expression_vector";
                case MCEXPRT_access:
                    return "expression_access";
                case MCEXPRT_call:
                    return "expression_call";
                case MCEXPRT_function:
                    return "expression_function";
                case MCEXPRT_block:
                    return "expression_block";
                case MCEXPRT_if:
                    return "expression_if";
            }
            break;
        }
        case MCANT_STATEMENT: {
            struct mc_ast_statement *statement = mcapi_ast_node2statement(U, node);
            switch (statement->type) {
                case MCSTMTT_block:
                    return "statement_block";
                case MCSTMTT_simple:
                    return "statement_simple";
                case MCSTMTT_eval:
                    return "statement_eval";
                case MCSTMTT_while:
                    return "statement_while";
                case MCSTMTT_for:
                    return "statement_for";
                case MCSTMTT_iterator:
                    return "statement_iterator";
                case MCSTMTT_declaration:
                    return "statement_declaration";
                case MCSTMTT_assigment:
                    return "statement_assigment";
                case MCSTMTT_if:
                    return "statement_if";
            }
            break;
        }
    }

    mapi_error(U, "unknown ast node type");
}

MORPHINE_API struct mc_ast_function *mcapi_ast_functions(struct mc_ast *A) {
    return A->functions;
}

MORPHINE_API struct mc_ast_function *mcapi_ast_create_function(
    morphine_coroutine_t U,
    struct mc_ast *A,
    size_t closures,
    size_t args,
    size_t statics
) {
    struct mc_ast_function *function = mapi_allocator_uni(
        mapi_instance(U), NULL, sizeof(struct mc_ast_function)
    );

    *function = (struct mc_ast_function) {
        .line = 0,
        .recursive = false,
        .anonymous = true,
        .auto_closure = false,
        .closures_size = 0,
        .args_size = 0,
        .statics_size = 0,
        .closures = NULL,
        .arguments = NULL,
        .statics = NULL,
        .body = NULL,
        .prev = A->functions
    };

    A->functions = function;

    function->closures = mapi_allocator_vec(
        mapi_instance(U), NULL, closures, sizeof(mc_strtable_index_t)
    );
    function->closures_size = closures;

    function->arguments = mapi_allocator_vec(
        mapi_instance(U), NULL, args, sizeof(mc_strtable_index_t)
    );
    function->args_size = args;

    function->statics = mapi_allocator_vec(
        mapi_instance(U), NULL, statics, sizeof(mc_strtable_index_t)
    );
    function->statics_size = statics;

    return function;
}

MORPHINE_API void mcapi_ast_set_root_function(struct mc_ast *A, struct mc_ast_function *function) {
    A->root_function = function;
}

MORPHINE_API struct mc_ast_function *mcapi_ast_get_root_function(struct mc_ast *A) {
    return A->root_function;
}

// creator

static void init_node(
    struct mc_ast *A,
    struct mc_ast_node *node,
    enum mc_ast_node_type type,
    ml_line line
) {
    (*node) = (struct mc_ast_node) {
        .type = type,
        .line = line,
        .prev = A->nodes
    };

    A->nodes = node;
}

struct mc_ast_expression *ast_create_expression(
    morphine_coroutine_t U,
    struct mc_ast *A,
    enum mc_expression_type type,
    ml_line line,
    size_t size
) {
    struct mc_ast_expression *expression =
        mapi_allocator_uni(mapi_instance(U), NULL, size);
    init_node(A, &expression->node, MCANT_EXPRESSION, line);

    expression->type = type;

    return expression;
}

struct mc_ast_statement *ast_create_statement(
    morphine_coroutine_t U,
    struct mc_ast *A,
    enum mc_statement_type type,
    ml_line line,
    size_t size
) {
    struct mc_ast_statement *statement =
        mapi_allocator_uni(mapi_instance(U), NULL, size);
    init_node(A, &statement->node, MCANT_STATEMENT, line);

    statement->type = type;

    return statement;
}
