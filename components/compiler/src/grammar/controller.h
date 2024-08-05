//
// Created by why-iskra on 05.08.2024.
//

#pragma once

#include "morphinec/lex.h"
#include "morphinec/ast.h"

#define et_integer()       ((struct mc_lex_token) { .type = MCLTT_INTEGER })
#define et_decimal()       ((struct mc_lex_token) { .type = MCLTT_DECIMAL })
#define et_string()        ((struct mc_lex_token) { .type = MCLTT_STRING })
#define et_word()          ((struct mc_lex_token) { .type = MCLTT_WORD })
#define et_eos()           ((struct mc_lex_token) { .type = MCLTT_EOS })
#define et_operator(op)    ((struct mc_lex_token) { .type = MCLTT_OPERATOR, .operator = MCLTOP_##op })
#define et_predef_word(pw) ((struct mc_lex_token) { .type = MCLTT_PREDEFINED_WORD, .predefined_word = MCLTPW_##pw })

struct parse_controller;
typedef struct mc_ast_node *(*parse_function_t)(struct parse_controller *);

ml_line parser_get_line(struct parse_controller *);
morphine_noret void parser_error(struct parse_controller *, const char *);

morphine_coroutine_t parser_U(struct parse_controller *);
struct mc_ast *parser_A(struct parse_controller *);
bool parser_look(struct parse_controller *, struct mc_lex_token expected);
bool parser_match(struct parse_controller *, struct mc_lex_token expected);
struct mc_lex_token parser_consume(struct parse_controller *, struct mc_lex_token expected);
struct mc_ast_node *parser_reduce(struct parse_controller *, parse_function_t);
void parser_reset(struct parse_controller *);

struct mc_ast_node *parse_root(struct parse_controller *);

// rules

#define rule(name) struct mc_ast_node *rule_##name(struct parse_controller *)

rule(statement_explicit);
rule(statement_explicit_without_semicolon);
rule(statement_implicit);

rule(statement_block);
rule(expression_block);

rule(statement_if);
rule(expression_if);

rule(while);
rule(dowhile);
rule(for);
rule(iterator);
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
rule(table);
rule(vector);
rule(function);

#undef rule
