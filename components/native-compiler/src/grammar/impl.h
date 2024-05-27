//
// Created by why-iskra on 31.05.2024.
//

#pragma once

#include "support/matcher.h"
#include "support/elements.h"
#include "ast.h"

struct grammar_quantum {
    enum reduce_type type;
    bool is_wrapping;

    union {
        void (*normal)(struct matcher *);
        bool (*wrapping)(struct matcher *, bool);
    };

    struct ast_node *(*assemble)(morphine_coroutine_t, struct elements *);
};

extern struct grammar_quantum grammar[];
size_t grammar_size(void);

// quantum

void match_statement_block(struct matcher *);
void match_expression_block(struct matcher *);
void match_block_elem(struct matcher *);
void match_implicit_block_elem(struct matcher *);
struct ast_node *assemble_statement_block(morphine_coroutine_t, struct elements *);
struct ast_node *assemble_expression_block(morphine_coroutine_t, struct elements *);
struct ast_node *assemble_block_elem(morphine_coroutine_t, struct elements *);
struct ast_node *assemble_implicit_block_elem(morphine_coroutine_t, struct elements *);

void match_statement_if(struct matcher *);
void match_expression_if(struct matcher *);
struct ast_node *assemble_statement_if(morphine_coroutine_t, struct elements *);
struct ast_node *assemble_expression_if(morphine_coroutine_t, struct elements *);

void match_ast(struct matcher *);
struct ast_node *assemble_ast(morphine_coroutine_t, struct elements *);

void match_statement(struct matcher *);
struct ast_node *assemble_statement(morphine_coroutine_t, struct elements *);

void match_while(struct matcher *);
struct ast_node *assemble_while(morphine_coroutine_t, struct elements *);

void match_do_while(struct matcher *);
struct ast_node *assemble_do_while(morphine_coroutine_t, struct elements *);

void match_for(struct matcher *);
struct ast_node *assemble_for(morphine_coroutine_t, struct elements *);

void match_iterator(struct matcher *);
struct ast_node *assemble_iterator(morphine_coroutine_t, struct elements *);

void match_declaration(struct matcher *);
struct ast_node *assemble_declaration(morphine_coroutine_t, struct elements *);

void match_assigment(struct matcher *);
struct ast_node *assemble_assigment(morphine_coroutine_t, struct elements *);

void match_expression(struct matcher *);
struct ast_node *assemble_expression(morphine_coroutine_t, struct elements *);

bool match_binary_or(struct matcher *, bool);
bool match_binary_and(struct matcher *, bool);
bool match_binary_equal(struct matcher *, bool);
bool match_binary_condition(struct matcher *, bool);
bool match_binary_concat(struct matcher *, bool);
bool match_binary_additive(struct matcher *, bool);
bool match_binary_multiplicative(struct matcher *, bool);
struct ast_node *assemble_binary(morphine_coroutine_t, struct elements *);

void match_prefix(struct matcher *);
struct ast_node *assemble_prefix(morphine_coroutine_t, struct elements *);

bool match_postfix(struct matcher *, bool);
struct ast_node *assemble_postfix(morphine_coroutine_t, struct elements *);

void match_primary(struct matcher *);
struct ast_node *assemble_primary(morphine_coroutine_t, struct elements *);

bool match_variable(struct matcher *, bool);
struct ast_node *assemble_variable(morphine_coroutine_t, struct elements *);

void match_value(struct matcher *);
struct ast_node *assemble_value(morphine_coroutine_t, struct elements *);

void match_table(struct matcher *);
struct ast_node *assemble_table(morphine_coroutine_t, struct elements *);

void match_vector(struct matcher *);
struct ast_node *assemble_vector(morphine_coroutine_t, struct elements *);

void match_function(struct matcher *);
struct ast_node *assemble_function(morphine_coroutine_t, struct elements *);
