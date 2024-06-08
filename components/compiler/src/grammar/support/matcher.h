//
// Created by why-iskra on 27.05.2024.
//

#pragma once

#include "morphinec/lex.h"

#define symbol_eos            ((struct matcher_symbol) { .type = TT_EOS })
#define symbol_int            ((struct matcher_symbol) { .type = TT_INTEGER })
#define symbol_dec            ((struct matcher_symbol) { .type = TT_DECIMAL })
#define symbol_str            ((struct matcher_symbol) { .type = TT_STRING })
#define symbol_word           ((struct matcher_symbol) { .type = TT_WORD })
#define symbol_predef_word(w) ((struct matcher_symbol) { .type = TT_PREDEFINED_WORD, .predefined_word = (w) })
#define symbol_operator(o)    ((struct matcher_symbol) { .type = TT_OPERATOR, .operator = (o) })

enum reduce_type {
    REDUCE_TYPE_AST,

    REDUCE_TYPE_STATEMENT,
    REDUCE_TYPE_WHILE,
    REDUCE_TYPE_DO_WHILE,
    REDUCE_TYPE_FOR,
    REDUCE_TYPE_ITERATOR,
    REDUCE_TYPE_DECLARATION,
    REDUCE_TYPE_ASSIGMENT,

    REDUCE_TYPE_BLOCK_ELEM,
    REDUCE_TYPE_IMPLICIT_BLOCK_ELEM,
    REDUCE_TYPE_STATEMENT_BLOCK,
    REDUCE_TYPE_EXPRESSION_BLOCK,

    REDUCE_TYPE_STATEMENT_IF,
    REDUCE_TYPE_EXPRESSION_IF,

    REDUCE_TYPE_EXPRESSION,
    REDUCE_TYPE_OR,
    REDUCE_TYPE_AND,
    REDUCE_TYPE_EQUAL,
    REDUCE_TYPE_CONDITION,
    REDUCE_TYPE_CONCAT,
    REDUCE_TYPE_ADDITIVE,
    REDUCE_TYPE_MULTIPLICATIVE,
    REDUCE_TYPE_PREFIX,
    REDUCE_TYPE_POSTFIX,
    REDUCE_TYPE_PRIMARY,
    REDUCE_TYPE_VARIABLE,
    REDUCE_TYPE_VALUE,
    REDUCE_TYPE_TABLE,
    REDUCE_TYPE_VECTOR,
    REDUCE_TYPE_FUNCTION,
};

struct matcher;

struct matcher_symbol {
    enum token_type type;
    union {
        enum token_predefined_word predefined_word;
        enum token_operator operator;
    };
};

bool matcher_symbol(struct matcher_symbol, struct token);
morphine_noret void matcher_error(struct matcher *, const char *);
bool matcher_look(struct matcher *, struct matcher_symbol);
bool matcher_match(struct matcher *, struct matcher_symbol);
struct token matcher_consume(struct matcher *, struct matcher_symbol);
void matcher_reduce(struct matcher *, enum reduce_type);
bool matcher_is_reduced(struct matcher *, enum reduce_type);
