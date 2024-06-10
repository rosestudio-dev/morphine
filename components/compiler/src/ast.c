//
// Created by why-iskra on 27.05.2024.
//

#include <string.h>
#include "morphinec/ast.h"

#define MORPHINE_TYPE "ast"

struct ast {
    struct ast_node *nodes;
    struct ast_function *functions;
};

static void ast_free(morphine_instance_t I, void *p) {
    struct ast *A = p;

    struct ast_function *function = A->functions;
    while (function != NULL) {
        struct ast_function *prev = function->prev;
        mapi_allocator_free(I, function);
        function = prev;
    }

    struct ast_node *node = A->nodes;
    while (node != NULL) {
        struct ast_node *prev = node->prev;
        mapi_allocator_free(I, node);
        node = prev;
    }
}

struct ast *ast(morphine_coroutine_t U) {
    struct ast *A = mapi_push_userdata(U, MORPHINE_TYPE, sizeof(struct ast));

    *A = (struct ast) {
        .nodes = NULL,
        .functions = NULL
    };

    mapi_userdata_set_free(U, ast_free);

    return A;
}

struct ast *get_ast(morphine_coroutine_t U) {
    if (strcmp(mapi_userdata_type(U), MORPHINE_TYPE) == 0) {
        return mapi_userdata_pointer(U);
    } else {
        mapi_error(U, "expected "MORPHINE_TYPE);
    }
}

struct ast_function *ast_functions(struct ast *A) {
    return A->functions;
}

struct expression *ast_node_as_expression(morphine_coroutine_t U, struct ast_node *node) {
    if (node->type != AST_NODE_TYPE_EXPRESSION) {
        mapi_error(U, "ast node isn't expression");
    }

    return (struct expression *) node;
}

struct statement *ast_node_as_statement(morphine_coroutine_t U, struct ast_node *node) {
    if (node->type != AST_NODE_TYPE_STATEMENT) {
        mapi_error(U, "ast node isn't statement");
    }

    return (struct statement *) node;
}

struct ast_function *ast_create_function(
    morphine_coroutine_t U,
    struct ast *A,
    size_t closures,
    size_t args,
    size_t statics
) {
    size_t size = sizeof(struct ast_function) +
                  sizeof(strtable_index_t) * closures +
                  sizeof(strtable_index_t) * args +
                  sizeof(strtable_index_t) * statics;

    struct ast_function *function = mapi_allocator_uni(
        mapi_instance(U),
        NULL,
        size
    );

    function->closures_size = closures;
    function->args_size = args;
    function->statics_size = statics;

    function->closures = ((void *) function) + sizeof(struct ast_function);
    function->arguments = ((void *) function->closures) + sizeof(strtable_index_t) * closures;
    function->statics = ((void *) function->arguments) + sizeof(strtable_index_t) * args;

    function->prev = A->functions;
    A->functions = function;

    return function;
}

static void setup_node(
    struct ast *A,
    struct ast_node *node,
    ml_line line,
    enum ast_node_type type
) {
    *node = (struct ast_node) {
        .prev = A->nodes,
        .type = type,
        .line = line
    };

    A->nodes = node;
}

static struct expression *ast_insert_expression(
    morphine_coroutine_t U,
    struct ast *A,
    ml_line line,
    enum expression_type type,
    size_t size
) {
    struct expression *expression = mapi_allocator_uni(
        mapi_instance(U),
        NULL,
        size
    );

    *expression = (struct expression) {
        .type = type
    };

    setup_node(A, ast_as_node(expression), line, AST_NODE_TYPE_EXPRESSION);
    return expression;
}

static struct statement *ast_insert_statement(
    morphine_coroutine_t U,
    struct ast *A,
    ml_line line,
    enum statement_type type,
    size_t size
) {
    struct statement *statement = mapi_allocator_uni(
        mapi_instance(U),
        NULL,
        size
    );

    *statement = (struct statement) {
        .type = type
    };

    setup_node(A, ast_as_node(statement), line, AST_NODE_TYPE_STATEMENT);
    return statement;
}

// create

#define body_create(U, A, line, nodetype, prefix, typ, size) \
    ((struct nodetype##_##typ *) ast_insert_##nodetype(U, A, line, prefix##_##typ, \
        sizeof(struct nodetype##_##typ) + (size)))

#define body_expression_create(type, U, A, line, size) body_create(U, A, line, expression, EXPRESSION_TYPE, type, size)
#define body_statement_create(type, U, A, line, size) body_create(U, A, line, statement, STATEMENT_TYPE, type, size)

#define function_expression_create(type) \
    struct expression_##type *ast_create_expression_##type(morphine_coroutine_t U, struct ast *A, ml_line line) { \
        return body_expression_create(type, U, A, line, 0); }

#define function_statement_create(type) \
    struct statement_##type *ast_create_statement_##type(morphine_coroutine_t U, struct ast *A, ml_line line) { \
        return body_statement_create(type, U, A, line, 0); }

// as

#define body_as(U, node, nodetype, prefix, typ) \
    struct nodetype *temp = ast_node_as_##nodetype(U, node); \
    if(temp->type != prefix##_##typ) { mapi_error(U, "expected " #nodetype " " #typ); } \
    return (struct nodetype##_##typ *) temp;

#define body_expression_as(U, node, type) body_as(U, node, expression, EXPRESSION_TYPE, type)
#define body_statement_as(U, node, type) body_as(U, node, statement, STATEMENT_TYPE, type)

#define function_expression_as(type) \
    struct expression_##type *ast_as_expression_##type(morphine_coroutine_t U, struct ast_node *node) { \
        body_expression_as(U, node, type); }

#define function_statement_as(type) \
    struct statement_##type *ast_as_statement_##type(morphine_coroutine_t U, struct ast_node *node) { \
        body_statement_as(U, node, type); }

/*
 * statement functions
 */

// block

struct statement_block *
ast_create_statement_block(morphine_coroutine_t U, struct ast *A, ml_line line, size_t size) {
    size_t extend_size = sizeof(struct statement *) * size;
    struct statement_block *result = body_statement_create(block, U, A, line, extend_size);

    result->size = size;
    result->statements = ((void *) result) + sizeof(struct statement_block);

    return result;
}

function_statement_as(block)

// eval

function_statement_create(eval)
function_statement_as(eval)

// for

function_statement_create(for)
function_statement_as(for)

// return

function_statement_create(return)
function_statement_as(return)

// simple

function_statement_create(simple)
function_statement_as(simple)

// while

function_statement_create(while)
function_statement_as(while)

// iterator

struct statement_iterator *ast_create_statement_iterator(
    morphine_coroutine_t U, struct ast *A, ml_line line, size_t size
) {
    size_t extend_size = (sizeof(struct expression *) + sizeof(strtable_index_t)) * size;
    struct statement_iterator *result = body_statement_create(iterator, U, A, line, extend_size);

    result->size = size;
    result->multi.names = ((void *) result) + sizeof(struct statement_iterator);
    result->multi.keys = ((void *) result->multi.names) + sizeof(strtable_index_t) * size;

    return result;
}

function_statement_as(iterator)

// if

struct statement_if *ast_create_statement_if(
    morphine_coroutine_t U, struct ast *A, ml_line line, size_t size
) {
    size_t extend_size = (sizeof(struct expression *) + sizeof(struct statement *)) * size;
    struct statement_if *result = body_statement_create(if, U, A, line, extend_size);

    result->size = size;
    result->elif_conditions = ((void *) result) + sizeof(struct statement_if);
    result->elif_statements = ((void *) result->elif_conditions) + sizeof(struct expression *) * size;

    return result;
}

function_statement_as(if)

// declaration

struct statement_declaration *ast_create_statement_declaration(
    morphine_coroutine_t U, struct ast *A, ml_line line, size_t size
) {
    size_t extend_size = (sizeof(struct expression *) + sizeof(strtable_index_t)) * size;
    struct statement_declaration *result = body_statement_create(declaration, U, A, line, extend_size);

    result->size = size;
    result->multi.names = ((void *) result) + sizeof(struct statement_declaration);
    result->multi.keys = ((void *) result->multi.names) + sizeof(strtable_index_t) * size;

    return result;
}

function_statement_as(declaration)

// assigment

struct statement_assigment *ast_create_statement_assigment(
    morphine_coroutine_t U, struct ast *A, ml_line line, size_t size
) {
    size_t extend_size = sizeof(struct expression *) * 2 * size;
    struct statement_assigment *result = body_statement_create(assigment, U, A, line, extend_size);

    result->size = size;
    result->multi.containers = ((void *) result) + sizeof(struct statement_assigment);
    result->multi.keys = ((void *) result->multi.containers) + sizeof(struct expression *) * size;

    return result;
}

function_statement_as(assigment)

/*
 * expression functions
 */

// value

function_expression_create(value)
function_expression_as(value)

// binary

function_expression_create(binary)
function_expression_as(binary)

// unary

function_expression_create(unary)
function_expression_as(unary)

// increment

function_expression_create(increment)
function_expression_as(increment)

// variable

function_expression_create(variable)
function_expression_as(variable)

// global

function_expression_create(global)
function_expression_as(global)

// table

struct expression_table *ast_create_expression_table(
    morphine_coroutine_t U, struct ast *A, ml_line line, size_t size
) {
    size_t extend_size = sizeof(struct expression *) * 2 * size;
    struct expression_table *result = body_expression_create(table, U, A, line, extend_size);

    result->size = size;
    result->keys = ((void *) result) + sizeof(struct expression_table);
    result->values = ((void *) result->keys) + sizeof(struct expression *) * size;

    return result;
}

function_expression_as(table)

// vector

struct expression_vector *ast_create_expression_vector(
    morphine_coroutine_t U, struct ast *A, ml_line line, size_t size
) {
    size_t extend_size = sizeof(struct expression *) * size;
    struct expression_vector *result = body_expression_create(vector, U, A, line, extend_size);

    result->size = size;
    result->values = ((void *) result) + sizeof(struct expression_vector);

    return result;
}

function_expression_as(vector)

// access

function_expression_create(access)
function_expression_as(access)

// call

struct expression_call *ast_create_expression_call(
    morphine_coroutine_t U, struct ast *A, ml_line line, size_t size
) {
    size_t extend_size = sizeof(struct expression *) * size;
    struct expression_call *result = body_expression_create(call, U, A, line, extend_size);

    result->args_size = size;
    result->arguments = ((void *) result) + sizeof(struct expression_call);

    return result;
}

function_expression_as(call)

// call_self

struct expression_call_self *ast_create_expression_call_self(
    morphine_coroutine_t U, struct ast *A, ml_line line, size_t size
) {
    size_t extend_size = sizeof(struct expression *) * size;
    struct expression_call_self *result = body_expression_create(call_self, U, A, line, extend_size);

    result->args_size = size;
    result->arguments = ((void *) result) + sizeof(struct expression_call_self);

    return result;
}

function_expression_as(call_self)

// function

function_expression_create(function)
function_expression_as(function)

// block

struct expression_block *ast_create_expression_block(
    morphine_coroutine_t U, struct ast *A, ml_line line, size_t size
) {
    size_t extend_size = sizeof(struct statement *) * size;
    struct expression_block *result = body_expression_create(block, U, A, line, extend_size);

    result->size = size;
    result->statements = ((void *) result) + sizeof(struct expression_block);

    return result;
}

function_expression_as(block)

// if

struct expression_if *ast_create_expression_if(
    morphine_coroutine_t U, struct ast *A, ml_line line, size_t size
) {
    size_t extend_size = sizeof(struct expression *) * 2 * size;
    struct expression_if *result = body_expression_create(if, U, A, line, extend_size);

    result->size = size;
    result->elif_conditions = ((void *) result) + sizeof(struct expression_if);
    result->elif_expressions = ((void *) result->elif_conditions) + sizeof(struct expression *) * size;

    return result;
}

function_expression_as(if)
