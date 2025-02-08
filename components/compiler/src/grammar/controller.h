//
// Created by why-iskra on 05.08.2024.
//

#pragma once

#include "morphinec/ast.h"
#include "morphinec/lex.h"

enum predefined_word {
#define predef_word(n) PW_##n,
#include "predefword.h"
#undef predef_word
};

enum expected_token_type {
    ETT_INTEGER,
    ETT_DECIMAL,
    ETT_STRING,
    ETT_WORD,
    ETT_EOS,
    ETT_OPERATOR,
    ETT_PREDEFINED_WORD,
    ETT_IMPLICIT_WORD,
};

struct expected_token {
    enum expected_token_type type;
    union {
        enum mc_lex_token_operator op;
        enum predefined_word predefined_word;
    };
};

#define et_integer()          ((struct expected_token) { .type = ETT_INTEGER })
#define et_decimal()          ((struct expected_token) { .type = ETT_DECIMAL })
#define et_string()           ((struct expected_token) { .type = ETT_STRING })
#define et_word()             ((struct expected_token) { .type = ETT_WORD })
#define et_eos()              ((struct expected_token) { .type = ETT_EOS })
#define et_operator(n)        ((struct expected_token) { .type = ETT_OPERATOR, .op = MCLTOP_##n })
#define et_predef_word(n)     ((struct expected_token) { .type = ETT_PREDEFINED_WORD, .predefined_word = PW_##n })
#define et_implicit_word()    ((struct expected_token) { .type = ETT_IMPLICIT_WORD })

struct parse_controller;
typedef struct mc_ast_node *(*parse_function_t)(struct parse_controller *);

ml_line parser_get_line(struct parse_controller *);
ml_size parser_index(struct parse_controller *);
mattr_noret void parser_errorf(struct parse_controller *, const char *, ...);

struct mc_strtable_entry parser_string(struct parse_controller *, mc_strtable_index_t);
morphine_coroutine_t parser_U(struct parse_controller *);
struct mc_ast *parser_A(struct parse_controller *);
bool parser_look(struct parse_controller *, struct expected_token);
bool parser_match(struct parse_controller *, struct expected_token);
struct mc_lex_token parser_consume(struct parse_controller *, struct expected_token);
struct mc_ast_node *parser_reduce(struct parse_controller *, parse_function_t);
void parser_reset(struct parse_controller *);

// rules

#define rule(name) struct mc_ast_node *rule_##name(struct parse_controller *)

rule(root);

rule(statement_block);
rule(expression_block);

rule(statement);
rule(while);
rule(dowhile);
rule(for);
rule(declaration);
rule(assigment);

rule(expression);
rule(or);
rule(and);
rule(equal);
rule(condition);
rule(concat);
rule(additive);
rule(multiplicative);
rule(prefix);
rule(postfix);
rule(primary);
rule(variable);
rule(value);
rule(constant);
rule(table);
rule(vector);
rule(function);
rule(if);

#undef rule
