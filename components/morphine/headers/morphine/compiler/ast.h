//
// Created by why-iskra on 27.05.2024.
//

#pragma once

#include <morphine.h>
#include "strtable.h"

#define ast_as_node(n)         ((struct ast_node *) (n))
#define ast_node_type(n)       (ast_as_node(n)->type)
#define ast_node_line(n)       (ast_as_node(n)->line)
#define ast_as_expression(n)   ((struct expression *) (n))
#define ast_expression_type(n) (ast_as_expression(n)->type)
#define ast_as_statement(n)    ((struct statement *) (n))
#define ast_statement_type(n)  (ast_as_statement(n)->type)

// ast

enum ast_node_type {
    AST_NODE_TYPE_EXPRESSION,
    AST_NODE_TYPE_STATEMENT,
};

enum statement_type {
    STATEMENT_TYPE_block,
    STATEMENT_TYPE_simple,
    STATEMENT_TYPE_eval,
    STATEMENT_TYPE_return,
    STATEMENT_TYPE_while,
    STATEMENT_TYPE_for,
    STATEMENT_TYPE_iterator,
    STATEMENT_TYPE_declaration,
    STATEMENT_TYPE_assigment,
    STATEMENT_TYPE_if,
};

enum expression_type {
    EXPRESSION_TYPE_value,
    EXPRESSION_TYPE_binary,
    EXPRESSION_TYPE_unary,
    EXPRESSION_TYPE_increment,
    EXPRESSION_TYPE_variable,
    EXPRESSION_TYPE_global,
    EXPRESSION_TYPE_table,
    EXPRESSION_TYPE_vector,
    EXPRESSION_TYPE_access,
    EXPRESSION_TYPE_call,
    EXPRESSION_TYPE_call_self,
    EXPRESSION_TYPE_function,
    EXPRESSION_TYPE_block,
    EXPRESSION_TYPE_if,
};

struct ast_node {
    enum ast_node_type type;
    struct ast_node *prev;
    ml_line line;
};

struct statement {
    struct ast_node node;
    enum statement_type type;
};

struct expression {
    struct ast_node node;
    enum expression_type type;
};

void ast(morphine_coroutine_t);
void ast_ready(morphine_coroutine_t, struct ast_node *);
struct ast_node *ast_root(morphine_coroutine_t);

struct expression *ast_node_as_expression(morphine_coroutine_t, struct ast_node *);
struct statement *ast_node_as_statement(morphine_coroutine_t, struct ast_node *);

/*
 * statements
 */

// eval

struct statement_eval {
    struct statement header;

    bool implicit;
    struct expression *expression;
};

// block

struct statement_block {
    struct statement header;

    size_t size;
    struct statement **statements;
};

// simple

enum statement_simple_type {
    STATEMENT_SIMPLE_TYPE_PASS,
    STATEMENT_SIMPLE_TYPE_YIELD,
    STATEMENT_SIMPLE_TYPE_LEAVE,
    STATEMENT_SIMPLE_TYPE_BREAK,
    STATEMENT_SIMPLE_TYPE_CONTINUE,
};

struct statement_simple {
    struct statement header;

    enum statement_simple_type type;
};

// return

struct statement_return {
    struct statement header;

    struct expression *expression;
};

// while

struct statement_while {
    struct statement header;

    bool first_condition;
    struct expression *condition;
    struct statement *statement;
};

// for

struct statement_for {
    struct statement header;

    struct statement *initial;
    struct expression *condition;
    struct statement *increment;
    struct statement *statement;
};

// iterator

struct statement_iterator {
    struct statement header;

    struct expression *expression;
    struct statement *statement;

    size_t size;
    union {
        struct {
            strtable_index_t *names;
            struct expression **keys;
        } multi;

        strtable_index_t name;
    };
};

// if

struct statement_if {
    struct statement header;

    struct expression *if_condition;
    struct statement *if_statement;
    struct statement *else_statement;

    size_t size;
    struct expression **elif_conditions;
    struct statement **elif_statements;
};

// declaration

struct statement_declaration {
    struct statement header;

    bool mutable;
    struct expression *expression;

    size_t size;
    union {
        struct {
            strtable_index_t *names;
            struct expression **keys;
        } multi;

        strtable_index_t name;
    };
};

// assigment

struct statement_assigment {
    struct statement header;

    struct expression *expression;

    size_t size;
    union {
        struct {
            struct expression **containers;
            struct expression **keys;
        } multi;

        struct expression *container;
    };
};

/*
 * expressions
 */

// value

enum expression_value_type {
    EXPRESSION_VALUE_TYPE_NIL,
    EXPRESSION_VALUE_TYPE_INT,
    EXPRESSION_VALUE_TYPE_DEC,
    EXPRESSION_VALUE_TYPE_STR,
    EXPRESSION_VALUE_TYPE_BOOL,
};

struct expression_value {
    struct expression header;

    enum expression_value_type type;
    union {
        ml_integer integer;
        ml_decimal decimal;
        strtable_index_t string;
        bool boolean;
    } value;
};

// binary

enum expression_binary_type {
    EXPRESSION_BINARY_TYPE_ADD,
    EXPRESSION_BINARY_TYPE_SUB,
    EXPRESSION_BINARY_TYPE_MUL,
    EXPRESSION_BINARY_TYPE_DIV,
    EXPRESSION_BINARY_TYPE_MOD,
    EXPRESSION_BINARY_TYPE_EQUAL,
    EXPRESSION_BINARY_TYPE_NOT_EQUAL,
    EXPRESSION_BINARY_TYPE_LESS,
    EXPRESSION_BINARY_TYPE_MORE,
    EXPRESSION_BINARY_TYPE_MORE_EQUAL,
    EXPRESSION_BINARY_TYPE_LESS_EQUAL,
    EXPRESSION_BINARY_TYPE_CONCAT,
    EXPRESSION_BINARY_TYPE_AND,
    EXPRESSION_BINARY_TYPE_OR,
};

struct expression_binary {
    struct expression header;

    enum expression_binary_type type;
    struct expression *a;
    struct expression *b;
};

// unary

enum expression_unary_type {
    EXPRESSION_UNARY_TYPE_NEGATE,
    EXPRESSION_UNARY_TYPE_NOT,
    EXPRESSION_UNARY_TYPE_TYPE,
    EXPRESSION_UNARY_TYPE_LEN,
    EXPRESSION_UNARY_TYPE_REF,
    EXPRESSION_UNARY_TYPE_DEREF,
};

struct expression_unary {
    struct expression header;

    enum expression_unary_type type;
    struct expression *expression;
};

// increment

struct expression_increment {
    struct expression header;

    bool is_decrement;
    bool is_postfix;
    struct expression *container;
};

// global

enum expression_global_type {
    EXPRESSION_GLOBAL_TYPE_ENV,
    EXPRESSION_GLOBAL_TYPE_SELF,
};

struct expression_global {
    struct expression header;

    enum expression_global_type type;
};

// variable

struct expression_variable {
    struct expression header;

    strtable_index_t index;
};

// table

struct expression_table {
    struct expression header;

    size_t size;
    struct expression **keys;
    struct expression **values;
};

// vector

struct expression_vector {
    struct expression header;

    size_t size;
    struct expression **values;
};

// access

struct expression_access {
    struct expression header;

    struct expression *container;
    struct expression *key;
};

// call

struct expression_call {
    struct expression header;

    size_t args_size;
    struct expression *expression;
    struct expression **arguments;
};

// call self

struct expression_call_self {
    struct expression header;

    size_t args_size;
    bool extract_callable;
    struct expression *self;
    struct expression *callable;
    struct expression **arguments;
};

// function

struct expression_function {
    struct expression header;

    bool anonymous;
    strtable_index_t name;

    bool auto_closure;
    size_t closures_size;
    size_t args_size;
    size_t statics_size;

    strtable_index_t *closures;
    struct expression **arguments;
    strtable_index_t *statics;

    struct statement *body;
};

// block

struct expression_block {
    struct expression header;

    size_t size;
    struct statement **statements;
    struct statement_eval *result;
};

// if

struct expression_if {
    struct expression header;

    struct expression *if_condition;
    struct expression *if_expression;
    struct expression *else_expression;

    size_t size;
    struct expression **elif_conditions;
    struct expression **elif_expressions;
};

#define ast_node_empty_args
#define ast_node_args(args ...) , args
#define ast_node_functions(type, name, args) \
struct type##_##name *ast_create_##type##_##name(morphine_coroutine_t, ml_line line args); \
struct type##_##name *ast_as_##type##_##name(morphine_coroutine_t, struct ast_node *);

ast_node_functions(statement, block, ast_node_args(size_t size))
ast_node_functions(statement, eval, ast_node_empty_args)
ast_node_functions(statement, simple, ast_node_empty_args)
ast_node_functions(statement, return, ast_node_empty_args)
ast_node_functions(statement, for, ast_node_empty_args)
ast_node_functions(statement, while, ast_node_empty_args)
ast_node_functions(statement, iterator, ast_node_args(size_t size))
ast_node_functions(statement, if, ast_node_args(size_t size))
ast_node_functions(statement, declaration, ast_node_args(size_t size))
ast_node_functions(statement, assigment, ast_node_args(size_t size))

ast_node_functions(expression, value, ast_node_empty_args)
ast_node_functions(expression, binary, ast_node_empty_args)
ast_node_functions(expression, unary, ast_node_empty_args)
ast_node_functions(expression, increment, ast_node_empty_args)
ast_node_functions(expression, global, ast_node_empty_args)
ast_node_functions(expression, variable, ast_node_empty_args)
ast_node_functions(expression, table, ast_node_args(size_t size))
ast_node_functions(expression, vector, ast_node_args(size_t size))
ast_node_functions(expression, access, ast_node_empty_args)
ast_node_functions(expression, call, ast_node_args(size_t args_size))
ast_node_functions(expression, call_self, ast_node_args(size_t args_size))
ast_node_functions(expression, function, ast_node_args(size_t closures, size_t args, size_t statics))
ast_node_functions(expression, block, ast_node_args(size_t size))
ast_node_functions(expression, if, ast_node_args(size_t size))

#undef ast_node_empty_args
#undef ast_node_args
#undef ast_node_functions
